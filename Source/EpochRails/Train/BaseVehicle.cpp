#include "Train/BaseVehicle.h"

#include "Gameplay/ResourceInventory.h"
#include "Train/LocomotionComponent.h"
#include "Train/RailSplineActor.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ABaseVehicle::ABaseVehicle() {
  PrimaryActorTick.bCanEverTick = true;

  CurrentHealth = MaxHealth;

  // Create components with correct types
  ResourceInventory =
      CreateDefaultSubobject<UResourceInventory>(TEXT("ResourceInventory"));
  LocomotionComponent =
      CreateDefaultSubobject<ULocomotionComponent>(TEXT("LocomotionComponent"));
}

void ABaseVehicle::BeginPlay() {
  Super::BeginPlay();

  CurrentHealth = MaxHealth;
  UE_LOG(LogTemp, Log, TEXT("BaseVehicle spawned with health: %f"),
         CurrentHealth);

  if (ResourceInventory) {
    ResourceInventory->InitializeInventory();
  }

  InitializeRailSpline();
}

void ABaseVehicle::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void ABaseVehicle::SetupPlayerInputComponent(
    UInputComponent *PlayerInputComponent) {
  Super::SetupPlayerInputComponent(PlayerInputComponent);
}

float ABaseVehicle::TakeDamage(float DamageAmount,
                               FDamageEvent const &DamageEvent,
                               AController *EventInstigator,
                               AActor *DamageCauser) {
  const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent,
                                               EventInstigator, DamageCauser);

  if (ActualDamage <= 0.0f || CurrentHealth <= 0.0f) {
    return 0.0f;
  }

  CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);
  UE_LOG(LogTemp, Log, TEXT("BaseVehicle took damage: %f, current health: %f"),
         ActualDamage, CurrentHealth);

  if (CurrentHealth <= 0.0f) {
    UE_LOG(LogTemp, Warning, TEXT("BaseVehicle destroyed"));
    // TODO: handle death
  }

  return ActualDamage;
}

void ABaseVehicle::SetThrottle(float Value) {
  if (LocomotionComponent) {
    LocomotionComponent->SetThrottle(Value);
  }
}

float ABaseVehicle::GetHealthPercent() const {
  return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

void ABaseVehicle::InitializeRailSpline() {
  if (!LocomotionComponent) {
    UE_LOG(LogTemp, Warning,
           TEXT("BaseVehicle: LocomotionComponent is null, cannot initialize "
                "rail spline"));
    return;
  }

  UWorld *World = GetWorld();
  if (!World) {
    UE_LOG(LogTemp, Warning,
           TEXT("BaseVehicle: World is null, cannot initialize rail spline"));
    return;
  }

  TArray<AActor *> FoundRails;
  UGameplayStatics::GetAllActorsOfClass(World, ARailSplineActor::StaticClass(),
                                        FoundRails);

  if (FoundRails.Num() == 0) {
    UE_LOG(LogTemp, Warning,
           TEXT("BaseVehicle: No ARailSplineActor found in world"));
    return;
  }

  ARailSplineActor *RailActor = Cast<ARailSplineActor>(FoundRails[0]);
  if (!RailActor) {
    UE_LOG(LogTemp, Warning,
           TEXT("BaseVehicle: Found actor is not ARailSplineActor"));
    return;
  }

  if (!RailActor->RailSpline) {
    UE_LOG(LogTemp, Warning,
           TEXT("BaseVehicle: ARailSplineActor has null RailSpline component"));
    return;
  }

  LocomotionComponent->SetRailSpline(RailActor->RailSpline);
  UE_LOG(LogTemp, Log, TEXT("BaseVehicle: Rail spline assigned from %s"),
         *RailActor->GetName());
}
