#include "Train/BaseVehicle.h"
#include "Gameplay/ResourceInventory.h"
#include "Train/LocomotionComponent.h"
#include "Train/RailSplineActor.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ABaseVehicle::ABaseVehicle() {
  PrimaryActorTick.bCanEverTick = true;

  // ИСПРАВЛЕНИЕ: Убрали дублирование инициализации здоровья из конструктора

  // Create components with correct types
  ResourceInventory =
      CreateDefaultSubobject<UResourceInventory>(TEXT("ResourceInventory"));
  LocomotionComponent =
      CreateDefaultSubobject<ULocomotionComponent>(TEXT("LocomotionComponent"));
}

void ABaseVehicle::BeginPlay() {
  Super::BeginPlay();

  // Инициализация здоровья только здесь
  CurrentHealth = MaxHealth;
  UE_LOG(LogTemp, Log, TEXT("BaseVehicle '%s' spawned with health: %f"),
         *GetName(), CurrentHealth);

  // Проверка компонентов
  if (!ResourceInventory) {
    UE_LOG(LogTemp, Error, TEXT("BaseVehicle '%s': ResourceInventory is null!"),
           *GetName());
  } else {
    ResourceInventory->InitializeInventory();
  }

  if (!LocomotionComponent) {
    UE_LOG(LogTemp, Error,
           TEXT("BaseVehicle '%s': LocomotionComponent is null!"), *GetName());
  }

  InitializeRailSpline();
}

void ABaseVehicle::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  // Обновление текущей скорости для отображения
  if (LocomotionComponent) {
    CurrentSpeed = LocomotionComponent->GetCurrentSpeed();
  }
}

void ABaseVehicle::SetupPlayerInputComponent(
    UInputComponent *PlayerInputComponent) {
  Super::SetupPlayerInputComponent(PlayerInputComponent);
}

float ABaseVehicle::TakeDamage(float DamageAmount,
                               FDamageEvent const &DamageEvent,
                               AController *EventInstigator,
                               AActor *DamageCauser) {
  // Не принимаем урон, если уже уничтожены
  if (bIsDestroyed || CurrentHealth <= 0.0f) {
    return 0.0f;
  }

  const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent,
                                               EventInstigator, DamageCauser);

  if (ActualDamage <= 0.0f) {
    return 0.0f;
  }

  CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);
  UE_LOG(LogTemp, Log,
         TEXT("BaseVehicle '%s' took damage: %f, current health: %f"),
         *GetName(), ActualDamage, CurrentHealth);

  // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Реализована обработка смерти
  if (CurrentHealth <= 0.0f && !bIsDestroyed) {
    bIsDestroyed = true;
    UE_LOG(LogTemp, Warning, TEXT("BaseVehicle '%s' destroyed"), *GetName());
    HandleDestruction();
  }

  return ActualDamage;
}

void ABaseVehicle::HandleDestruction_Implementation() {
  // Останавливаем транспорт
  if (LocomotionComponent) {
    LocomotionComponent->SetThrottle(0.0f);
    LocomotionComponent->ApplyBrakes(1000.0f);
  }

  // Вызываем событие для Blueprint
  OnVehicleDestroyed.Broadcast(this);

  // Опциональное автоматическое уничтожение через 5 секунд
  FTimerHandle DestroyTimer;
  GetWorldTimerManager().SetTimer(
      DestroyTimer,
      [this]() {
        if (IsValid(this)) {
          Destroy();
        }
      },
      5.0f, false);
}

void ABaseVehicle::SetThrottle(float Value) {
  if (LocomotionComponent && !bIsDestroyed) {
    LocomotionComponent->SetThrottle(Value);
  }
}

float ABaseVehicle::GetHealthPercent() const {
  return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

void ABaseVehicle::SetRailSpline(ARailSplineActor *NewRailSpline) {
  if (!NewRailSpline) {
    UE_LOG(LogTemp, Warning,
           TEXT("BaseVehicle '%s': Attempted to assign null rail spline"),
           *GetName());
    return;
  }

  AssignedRailSpline = NewRailSpline;
  InitializeRailSpline();
}

void ABaseVehicle::InitializeRailSpline() {
  if (!LocomotionComponent) {
    UE_LOG(LogTemp, Error,
           TEXT("BaseVehicle '%s': LocomotionComponent is null, cannot "
                "initialize rail spline"),
           *GetName());
    return;
  }

  ARailSplineActor *RailToUse = nullptr;

  // Приоритет 1: Используем назначенный вручную рельс
  if (AssignedRailSpline && AssignedRailSpline->RailSpline) {
    RailToUse = AssignedRailSpline;
    UE_LOG(LogTemp, Log,
           TEXT("BaseVehicle '%s': Using manually assigned rail '%s'"),
           *GetName(), *RailToUse->GetName());
  }
  // Приоритет 2: Автопоиск ближайшего рельса (если включено)
  else if (bAutoFindNearestRail) {
    RailToUse = FindNearestRail();
    if (RailToUse) {
      UE_LOG(LogTemp, Log,
             TEXT("BaseVehicle '%s': Auto-found nearest rail '%s'"), *GetName(),
             *RailToUse->GetName());
    }
  }

  // Валидация и назначение
  if (RailToUse && RailToUse->RailSpline) {
    LocomotionComponent->SetRailSpline(RailToUse->RailSpline);

    // НОВОЕ: Автоматически позиционируем транспорт на начало пути
    // Компонент сам установит позицию благодаря bAutoPositionOnStart = true

    UE_LOG(LogTemp, Log,
           TEXT("BaseVehicle '%s': Rail spline successfully assigned and "
                "positioned on '%s'"),
           *GetName(), *RailToUse->GetName());
  } else {
    UE_LOG(LogTemp, Error,
           TEXT("BaseVehicle '%s': Failed to initialize rail spline! "
                "AssignedRailSpline=%s, AutoFind=%s"),
           *GetName(), AssignedRailSpline ? TEXT("Set") : TEXT("None"),
           bAutoFindNearestRail ? TEXT("Enabled") : TEXT("Disabled"));
  }
}

ARailSplineActor *ABaseVehicle::FindNearestRail() {
  UWorld *World = GetWorld();
  if (!World) {
    return nullptr;
  }

  const FVector VehicleLocation = GetActorLocation();
  ARailSplineActor *NearestRail = nullptr;
  float MinDistance = RailSearchRadius;

  // ИСПРАВЛЕНИЕ: Ограниченный поиск с проверкой расстояния
  TArray<AActor *> FoundRails;
  UGameplayStatics::GetAllActorsOfClass(World, ARailSplineActor::StaticClass(),
                                        FoundRails);

  for (AActor *Actor : FoundRails) {
    ARailSplineActor *Rail = Cast<ARailSplineActor>(Actor);
    if (!Rail || !Rail->RailSpline) {
      continue;
    }

    // Находим ближайшую точку на сплайне
    const float InputKey =
        Rail->RailSpline->FindInputKeyClosestToWorldLocation(VehicleLocation);
    const FVector ClosestPoint = Rail->RailSpline->GetLocationAtSplineInputKey(
        InputKey, ESplineCoordinateSpace::World);
    const float Distance = FVector::Dist(VehicleLocation, ClosestPoint);

    if (Distance < MinDistance) {
      MinDistance = Distance;
      NearestRail = Rail;
    }

#if !UE_BUILD_SHIPPING
    // Визуализация в режиме отладки
    DrawDebugLine(World, VehicleLocation, ClosestPoint,
                  Distance < RailSearchRadius ? FColor::Green : FColor::Red,
                  false, 2.0f, 0, 2.0f);
#endif
  }

  if (NearestRail) {
    UE_LOG(LogTemp, Log,
           TEXT("BaseVehicle '%s': Found nearest rail '%s' at distance %f"),
           *GetName(), *NearestRail->GetName(), MinDistance);
  } else {
    UE_LOG(LogTemp, Warning,
           TEXT("BaseVehicle '%s': No rail found within search radius %f"),
           *GetName(), RailSearchRadius);
  }

  return NearestRail;
}
