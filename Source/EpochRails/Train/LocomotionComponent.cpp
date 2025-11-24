#include "LocomotionComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

ULocomotionComponent::ULocomotionComponent() {
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.TickInterval = 0.016f; // ~60 FPS
}

void ULocomotionComponent::BeginPlay() {
  Super::BeginPlay();

  if (!RailSpline) {
    UE_LOG(LogTemp, Warning,
           TEXT("LocomotionComponent: No rail spline assigned!"));
  } else {
    UE_LOG(LogTemp, Log, TEXT("LocomotionComponent: Rail spline assigned"));
  }
}

void ULocomotionComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  if (!RailSpline) {
    return;
  }

  UpdatePosition(DeltaTime);
  UpdateRotation();
}

void ULocomotionComponent::SetThrottle(float Value) {
  ThrottleInput = FMath::Clamp(Value, -1.0f, 1.0f);
}

void ULocomotionComponent::ApplyBrakes(float BrakeForce) {
  const float Clamped = FMath::Clamp(BrakeForce, 0.0f, 1.0f);
  CurrentVelocity *= (1.0f - Clamped);
}

FVector ULocomotionComponent::GetForwardDirection() const {
  if (!RailSpline) {
    return GetOwner() ? GetOwner()->GetActorForwardVector()
                      : FVector::ForwardVector;
  }

  return RailSpline->GetDirectionAtDistanceAlongSpline(
      CurrentDistance, ESplineCoordinateSpace::World);
}

void ULocomotionComponent::SetRailSpline(USplineComponent *NewSpline) {
  RailSpline = NewSpline;
  CurrentDistance = 0.0f;
  CurrentVelocity = 0.0f;
}

void ULocomotionComponent::UpdatePosition(float DeltaTime) {
  // velocity update (as у теб€ было Ч трогать не об€зательно)
  if (ThrottleInput > 0.0f) {
    CurrentVelocity += ThrottleInput * Acceleration * DeltaTime;
    CurrentVelocity = FMath::Min(CurrentVelocity, MaxSpeed);
  } else if (ThrottleInput < 0.0f) {
    CurrentVelocity += ThrottleInput * BrakingDeceleration * DeltaTime;
    CurrentVelocity = FMath::Max(CurrentVelocity, -MaxSpeed * 0.5f);
  } else {
    if (CurrentVelocity > 0.0f) {
      CurrentVelocity -= DragDeceleration * DeltaTime;
      CurrentVelocity = FMath::Max(CurrentVelocity, 0.0f);
    } else if (CurrentVelocity < 0.0f) {
      CurrentVelocity += DragDeceleration * DeltaTime;
      CurrentVelocity = FMath::Min(CurrentVelocity, 0.0f);
    }
  }

  const float PreviousDistance = CurrentDistance;
  CurrentDistance += CurrentVelocity * DeltaTime;

  if (RailSpline) {
    const float SplineLength = RailSpline->GetSplineLength();
    CurrentDistance = FMath::Clamp(CurrentDistance, 0.0f, SplineLength);
  }

  OnSpeedChanged.Broadcast(PreviousDistance, CurrentDistance);
}

void ULocomotionComponent::UpdateRotation() {
  if (!RailSpline || !GetOwner()) {
    return;
  }

  const FVector SplinePosition = RailSpline->GetLocationAtDistanceAlongSpline(
      CurrentDistance, ESplineCoordinateSpace::World);

  const FVector SplineDirection = RailSpline->GetDirectionAtDistanceAlongSpline(
      CurrentDistance, ESplineCoordinateSpace::World);

  GetOwner()->SetActorLocation(SplinePosition);
  GetOwner()->SetActorRotation(SplineDirection.Rotation());

  OnLocationChanged.Broadcast(SplinePosition);
}
