// Copyright Epic Games, Inc. All Rights Reserved.

#include "Train/LocomotionComponent.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "Train/RailSplineActor.h"

ULocomotionComponent::ULocomotionComponent() {
  PrimaryComponentTick.bCanEverTick = true;

  // Default physics parameters
  MaxSpeed = 2000.0f; // 72 km/h in cm/s
  Acceleration = 200.0f;
  Deceleration = 300.0f;
  BrakingForce = 500.0f;
  Mass = 10000.0f; // 10 tons
  InertiaDamping = 0.95f;
  FrictionCoefficient = 0.02f;

  // Initial state
  CurrentSpeed = 0.0f;
  TargetSpeed = 0.0f;
  DistanceAlongSpline = 0.0f;
  CurrentThrottle = 0.0f;
  CurrentBrake = 0.0f;

  // Cache
  CachedSplineLength = 0.0f;
  bSplineCacheValid = false;

  // Debug
  bShowDebugInfo = false;
}

void ULocomotionComponent::BeginPlay() {
  Super::BeginPlay();

  // Validate initial setup
  if (!ValidateSpline()) {
    UE_LOG(LogTemp, Warning,
           TEXT("LocomotionComponent: No rail spline assigned at begin play"));
  }
}

void ULocomotionComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  if (!ValidateSpline()) {
    return;
  }

  UpdateMovement(DeltaTime);
  UpdatePhysics(DeltaTime);
  UpdatePositionOnSpline(DeltaTime);

  if (bShowDebugInfo) {
    DrawDebugInfo();
  }
}

void ULocomotionComponent::SetRailSpline(ARailSplineActor *NewRailSpline) {
  if (!NewRailSpline) {
    UE_LOG(LogTemp, Warning,
           TEXT("LocomotionComponent: Attempted to set null rail spline"));
    CurrentRailSpline = nullptr;
    CachedSplineComponent = nullptr;
    InvalidateSplineCache();
    return;
  }

  CurrentRailSpline = NewRailSpline;
  CachedSplineComponent = NewRailSpline->GetSplineComponent();

  if (!CachedSplineComponent.IsValid()) {
    UE_LOG(LogTemp, Error,
           TEXT("LocomotionComponent: Rail spline has no spline component!"));
    return;
  }

  InvalidateSplineCache();
  UpdateSplineCache();

  UE_LOG(LogTemp, Log,
         TEXT("LocomotionComponent: Rail spline set successfully. Spline "
              "length: %.2f"),
         CachedSplineLength);
}

void ULocomotionComponent::SetThrottle(float ThrottleValue) {
  CurrentThrottle = FMath::Clamp(ThrottleValue, -1.0f, 1.0f);
  TargetSpeed = CurrentThrottle * MaxSpeed;

  if (FMath::Abs(CurrentThrottle) > 0.01f) {
    CurrentBrake = 0.0f; // Release brake when throttle is applied
  }
}

void ULocomotionComponent::ApplyBrake(float BrakeValue) {
  CurrentBrake = FMath::Clamp(BrakeValue, 0.0f, 1.0f);

  if (CurrentBrake > 0.01f) {
    CurrentThrottle = 0.0f; // Release throttle when braking
    TargetSpeed = 0.0f;
  }
}

void ULocomotionComponent::ResetMovement() {
  CurrentSpeed = 0.0f;
  TargetSpeed = 0.0f;
  CurrentThrottle = 0.0f;
  CurrentBrake = 0.0f;
  DistanceAlongSpline = 0.0f;

  UE_LOG(LogTemp, Log, TEXT("LocomotionComponent: Movement reset"));
}

void ULocomotionComponent::UpdateMovement(float DeltaTime) {
  // Apply braking force
  if (CurrentBrake > 0.01f) {
    const float BrakeDeceleration = BrakingForce * CurrentBrake;

    if (CurrentSpeed > 0.0f) {
      CurrentSpeed =
          FMath::Max(0.0f, CurrentSpeed - BrakeDeceleration * DeltaTime);
    } else if (CurrentSpeed < 0.0f) {
      CurrentSpeed =
          FMath::Min(0.0f, CurrentSpeed + BrakeDeceleration * DeltaTime);
    }

    return;
  }

  // Apply acceleration/deceleration
  if (FMath::Abs(TargetSpeed - CurrentSpeed) > 1.0f) {
    const float AccelRate = (FMath::Abs(TargetSpeed) > FMath::Abs(CurrentSpeed))
                                ? Acceleration
                                : Deceleration;
    const float Direction = FMath::Sign(TargetSpeed - CurrentSpeed);

    CurrentSpeed += Direction * AccelRate * DeltaTime;
    CurrentSpeed = FMath::Clamp(CurrentSpeed, -MaxSpeed, MaxSpeed);
  } else {
    CurrentSpeed = TargetSpeed;
  }
}

void ULocomotionComponent::UpdatePhysics(float DeltaTime) {
  ApplyInertia(DeltaTime);
  ApplyFriction(DeltaTime);
}

void ULocomotionComponent::ApplyInertia(float DeltaTime) {
  // Realistic inertia simulation for heavy trains
  if (FMath::Abs(CurrentSpeed) > 0.01f && FMath::Abs(CurrentThrottle) < 0.01f &&
      CurrentBrake < 0.01f) {
    CurrentSpeed *= FMath::Pow(InertiaDamping, DeltaTime);

    // Stop completely when speed is very low
    if (FMath::Abs(CurrentSpeed) < 0.5f) {
      CurrentSpeed = 0.0f;
    }
  }
}

void ULocomotionComponent::ApplyFriction(float DeltaTime) {
  // Rolling resistance based on mass and friction coefficient
  if (FMath::Abs(CurrentSpeed) > 0.01f) {
    const float FrictionForce =
        FrictionCoefficient * Mass * 980.0f; // 980 cm/s^2 gravity
    const float FrictionDeceleration = FrictionForce / Mass;
    const float Direction = -FMath::Sign(CurrentSpeed);

    CurrentSpeed += Direction * FrictionDeceleration * DeltaTime;
  }
}

void ULocomotionComponent::UpdatePositionOnSpline(float DeltaTime) {
  if (!bSplineCacheValid) {
    UpdateSplineCache();
  }

  // Update distance along spline
  DistanceAlongSpline += CurrentSpeed * DeltaTime;

  // Clamp to spline length
  if (DistanceAlongSpline < 0.0f) {
    DistanceAlongSpline = 0.0f;
    CurrentSpeed = 0.0f;
  } else if (DistanceAlongSpline > CachedSplineLength) {
    DistanceAlongSpline = CachedSplineLength;
    CurrentSpeed = 0.0f;
  }

  // Get position and rotation from spline
  AActor *Owner = GetOwner();
  if (!Owner || !CachedSplineComponent.IsValid()) {
    return;
  }

  const FVector NewLocation =
      CachedSplineComponent->GetLocationAtDistanceAlongSpline(
          DistanceAlongSpline, ESplineCoordinateSpace::World);
  const FRotator NewRotation =
      CachedSplineComponent->GetRotationAtDistanceAlongSpline(
          DistanceAlongSpline, ESplineCoordinateSpace::World);

  Owner->SetActorLocation(NewLocation);
  Owner->SetActorRotation(NewRotation);
}

bool ULocomotionComponent::ValidateSpline() const {
  if (!CurrentRailSpline.IsValid()) {
    return false;
  }

  if (!CachedSplineComponent.IsValid()) {
    return false;
  }

  return true;
}

void ULocomotionComponent::InvalidateSplineCache() {
  bSplineCacheValid = false;
  CachedSplineLength = 0.0f;
}

void ULocomotionComponent::UpdateSplineCache() {
  if (!CachedSplineComponent.IsValid()) {
    bSplineCacheValid = false;
    return;
  }

  CachedSplineLength = CachedSplineComponent->GetSplineLength();
  bSplineCacheValid = true;
}

void ULocomotionComponent::DrawDebugInfo() const {
  const AActor *Owner = GetOwner();
  if (!Owner) {
    return;
  }

  const FVector DebugLocation =
      Owner->GetActorLocation() + FVector(0.0f, 0.0f, 200.0f);
  const FString DebugText =
      FString::Printf(TEXT("Speed: %.1f km/h\nThrottle: %.2f\nBrake: "
                           "%.2f\nDistance: %.1f / %.1f"),
                      GetCurrentSpeedKmH(), CurrentThrottle, CurrentBrake,
                      DistanceAlongSpline, CachedSplineLength);

  DrawDebugString(Owner->GetWorld(), DebugLocation, DebugText, nullptr,
                  FColor::Green, 0.0f, true);
}
