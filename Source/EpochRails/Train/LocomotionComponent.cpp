#include "Train/LocomotionComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"

ULocomotionComponent::ULocomotionComponent() {
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void ULocomotionComponent::BeginPlay() {
  Super::BeginPlay();

  // Инициализация без warning
  CurrentDistance = StartDistance;
  CurrentVelocity = 0.0f;
  ThrottleInput = 0.0f;

  // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Если сплайн уже назначен, инициализируем позицию
  if (RailSpline && bAutoPositionOnStart) {
    InitializePosition();
  }
}

void ULocomotionComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  if (!RailSpline) {
    return;
  }

  // Обновление скорости на основе throttle
  const float TargetSpeed = ThrottleInput * MaxSpeed;
  const float SpeedDelta = TargetSpeed - CurrentVelocity;

  if (FMath::Abs(SpeedDelta) > KINDA_SMALL_NUMBER) {
    // Ускорение или замедление
    const float AccelRate =
        (SpeedDelta > 0) ? Acceleration : BrakingDeceleration;
    const float VelocityChange =
        FMath::Sign(SpeedDelta) * AccelRate * DeltaTime;

    CurrentVelocity += FMath::Clamp(VelocityChange, -FMath::Abs(SpeedDelta),
                                    FMath::Abs(SpeedDelta));
  }

  // Применение drag (естественное замедление)
  if (FMath::Abs(ThrottleInput) < 0.01f) {
    const float DragForce = DragDeceleration * DeltaTime;
    if (CurrentVelocity > 0.0f) {
      CurrentVelocity = FMath::Max(0.0f, CurrentVelocity - DragForce);
    } else if (CurrentVelocity < 0.0f) {
      CurrentVelocity = FMath::Min(0.0f, CurrentVelocity + DragForce);
    }
  }

  // Обновление позиции и поворота
  UpdatePosition(DeltaTime);
  UpdateRotation();

  // Вызов делегатов
  const float OldSpeed = CurrentVelocity;
  if (FMath::Abs(OldSpeed - CurrentVelocity) > 0.1f) {
    OnSpeedChanged.Broadcast(CurrentVelocity, MaxSpeed);
  }

#if !UE_BUILD_SHIPPING
  // Отладочная визуализация
  if (GEngine && GetOwner()) {
    const FVector Forward = GetForwardDirection();
    DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(),
                  GetOwner()->GetActorLocation() + Forward * 200.0f,
                  FColor::Green, false, 0.0f, 0, 3.0f);

    // Визуализация прогресса
    GEngine->AddOnScreenDebugMessage(
        10, 0.0f, FColor::Cyan,
        FString::Printf(TEXT("Distance: %.1f / %.1f"), CurrentDistance,
                        RailSpline->GetSplineLength()));
  }
#endif
}

void ULocomotionComponent::UpdatePosition(float DeltaTime) {
  if (!RailSpline || !GetOwner()) {
    return;
  }

  const float SplineLength = RailSpline->GetSplineLength();
  if (SplineLength <= 0.0f) {
    UE_LOG(
        LogTemp, Warning,
        TEXT("LocomotionComponent: Rail spline has zero or negative length"));
    return;
  }

  // ИСПРАВЛЕНИЕ: Корректное обновление позиции
  const float NewDistance = CurrentDistance + (CurrentVelocity * DeltaTime);

  // Проверка границ
  if (NewDistance >= SplineLength) {
    // Достигнут конец пути
    CurrentDistance = SplineLength;
    CurrentVelocity = 0.0f;
    ThrottleInput = 0.0f;

    UE_LOG(LogTemp, Log,
           TEXT("LocomotionComponent: Reached end of rail at distance %f"),
           CurrentDistance);
  } else if (NewDistance < 0.0f) {
    // Достигнуто начало пути при движении назад
    CurrentDistance = 0.0f;
    CurrentVelocity = 0.0f;
    ThrottleInput = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("LocomotionComponent: Reached start of rail"));
  } else {
    CurrentDistance = NewDistance;
  }

  // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Обновление позиции актора
  const FVector NewLocation = RailSpline->GetLocationAtDistanceAlongSpline(
      CurrentDistance, ESplineCoordinateSpace::World);

  GetOwner()->SetActorLocation(NewLocation, false, nullptr,
                               ETeleportType::None);

  // Вызов делегата изменения позиции
  OnLocationChanged.Broadcast(NewLocation);
}

void ULocomotionComponent::UpdateRotation() {
  if (!RailSpline || !GetOwner()) {
    return;
  }

  // ИСПРАВЛЕНИЕ: Плавный поворот по направлению движения
  const FVector Forward = GetForwardDirection();

  if (!Forward.IsNearlyZero()) {
    const FRotator TargetRotation = Forward.Rotation();
    const FRotator CurrentRotation = GetOwner()->GetActorRotation();

    // Интерполяция для плавности
    const FRotator NewRotation = FMath::RInterpTo(
        CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 5.0f);

    GetOwner()->SetActorRotation(NewRotation);
  }
}

void ULocomotionComponent::SetThrottle(float Value) {
  ThrottleInput = FMath::Clamp(Value, -1.0f, 1.0f);
}

void ULocomotionComponent::ApplyBrakes(float BrakeForce) {
  if (BrakeForce <= 0.0f) {
    return;
  }

  const float BrakingAmount = BrakeForce * GetWorld()->GetDeltaSeconds();

  if (CurrentVelocity > 0.0f) {
    CurrentVelocity = FMath::Max(0.0f, CurrentVelocity - BrakingAmount);
  } else if (CurrentVelocity < 0.0f) {
    CurrentVelocity = FMath::Min(0.0f, CurrentVelocity + BrakingAmount);
  }
}

FVector ULocomotionComponent::GetForwardDirection() const {
  if (!RailSpline) {
    return FVector::ForwardVector;
  }

  const FVector Direction = RailSpline->GetDirectionAtDistanceAlongSpline(
      CurrentDistance, ESplineCoordinateSpace::World);

  // Направление зависит от знака скорости
  return CurrentVelocity >= 0.0f ? Direction : -Direction;
}

void ULocomotionComponent::SetRailSpline(USplineComponent *NewSpline) {
  if (!NewSpline) {
    UE_LOG(LogTemp, Warning,
           TEXT("LocomotionComponent: Attempted to set null spline"));
    return;
  }

  RailSpline = NewSpline;

  // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Инициализация позиции при назначении сплайна
  if (bAutoPositionOnStart) {
    InitializePosition();
  }

  UE_LOG(LogTemp, Log,
         TEXT("LocomotionComponent: Rail spline set successfully. Length: %f"),
         RailSpline->GetSplineLength());
}

// НОВЫЙ МЕТОД: Инициализация позиции
void ULocomotionComponent::InitializePosition() {
  if (!RailSpline || !GetOwner()) {
    return;
  }

  const float SplineLength = RailSpline->GetSplineLength();

  // Вычисление начальной дистанции
  if (bUsePercentageForStart) {
    // Используем процент от длины (0.0 = начало, 1.0 = конец)
    CurrentDistance = FMath::Clamp(StartDistance, 0.0f, 1.0f) * SplineLength;
  } else {
    // Используем абсолютное значение в единицах
    CurrentDistance = FMath::Clamp(StartDistance, 0.0f, SplineLength);
  }

  // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Сразу устанавливаем актор на позицию
  const FVector StartLocation = RailSpline->GetLocationAtDistanceAlongSpline(
      CurrentDistance, ESplineCoordinateSpace::World);

  const FRotator StartRotation = RailSpline->GetRotationAtDistanceAlongSpline(
      CurrentDistance, ESplineCoordinateSpace::World);

  GetOwner()->SetActorLocationAndRotation(StartLocation, StartRotation, false,
                                          nullptr, ETeleportType::ResetPhysics);

  // Сброс движения
  CurrentVelocity = 0.0f;
  ThrottleInput = 0.0f;

  UE_LOG(
      LogTemp, Log,
      TEXT(
          "LocomotionComponent: Initialized at distance %.1f of %.1f (%.1f%%)"),
      CurrentDistance, SplineLength, (CurrentDistance / SplineLength) * 100.0f);
}

// НОВЫЙ МЕТОД: Установка начальной дистанции
void ULocomotionComponent::SetStartDistance(float Distance) {
  StartDistance = Distance;

  if (RailSpline) {
    InitializePosition();
  }
}

// НОВЫЙ МЕТОД: Привязка к ближайшей точке
void ULocomotionComponent::SnapToNearestPointOnSpline() {
  if (!RailSpline || !GetOwner()) {
    UE_LOG(LogTemp, Warning,
           TEXT("LocomotionComponent: Cannot snap - missing spline or owner"));
    return;
  }

  const FVector CurrentLocation = GetOwner()->GetActorLocation();

  // Находим ближайшую точку на сплайне
  const float InputKey =
      RailSpline->FindInputKeyClosestToWorldLocation(CurrentLocation);
  CurrentDistance =
      RailSpline->GetDistanceAlongSplineAtSplineInputKey(InputKey);

  // Перемещаем актор на сплайн
  const FVector SnapLocation = RailSpline->GetLocationAtDistanceAlongSpline(
      CurrentDistance, ESplineCoordinateSpace::World);

  const FRotator SnapRotation = RailSpline->GetRotationAtDistanceAlongSpline(
      CurrentDistance, ESplineCoordinateSpace::World);

  GetOwner()->SetActorLocationAndRotation(SnapLocation, SnapRotation, false,
                                          nullptr, ETeleportType::ResetPhysics);

  // Сброс скорости
  CurrentVelocity = 0.0f;
  ThrottleInput = 0.0f;

  UE_LOG(LogTemp, Log,
         TEXT("LocomotionComponent: Snapped to nearest point at distance %.1f"),
         CurrentDistance);
}
