// Copyright Epic Games, Inc. All Rights Reserved.

#include "TrainPhysicsComponent.h"
#include "Kismet/KismetMathLibrary.h"

UTrainPhysicsComponent::UTrainPhysicsComponent() {
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UTrainPhysicsComponent::BeginPlay() {
  Super::BeginPlay();

  // Initialize total mass
  UpdateTotalMass();
}

void UTrainPhysicsComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  if (DeltaTime > 0.0f) {
    // Update physics simulation
    UpdatePhysics(DeltaTime);
  }
}

void UTrainPhysicsComponent::SetThrottle(float NewThrottle) {
  CurrentThrottle = FMath::Clamp(NewThrottle, 0.0f, 1.0f);
}

void UTrainPhysicsComponent::SetBrake(float NewBrake) {
  CurrentBrake = FMath::Clamp(NewBrake, 0.0f, 1.0f);
}

void UTrainPhysicsComponent::SetTrackGrade(float GradeDegrees) {
  CurrentGradeDegrees = FMath::Clamp(GradeDegrees, -45.0f, 45.0f);
}

void UTrainPhysicsComponent::SetTrackCurvature(float Curvature) {
  CurrentCurvature = FMath::Clamp(Curvature, 0.0f, 1.0f);
}

float UTrainPhysicsComponent::GetVelocityKmh() const {
  // Convert m/s to km/h
  return PhysicsState.Velocity * 3.6f;
}

void UTrainPhysicsComponent::AddWagons(int32 Count) {
  if (Count > 0) {
    PhysicsParameters.WagonCount += Count;
    UpdateTotalMass();
  }
}

void UTrainPhysicsComponent::RemoveWagons(int32 Count) {
  if (Count > 0) {
    PhysicsParameters.WagonCount =
        FMath::Max(0, PhysicsParameters.WagonCount - Count);
    UpdateTotalMass();
  }
}

void UTrainPhysicsComponent::SetWagonMass(float NewWagonMass) {
  if (NewWagonMass > 0.0f) {
    PhysicsParameters.WagonMass = NewWagonMass;
    UpdateTotalMass();
  }
}

void UTrainPhysicsComponent::EmergencyBrake() {
  CurrentThrottle = 0.0f;
  CurrentBrake = 1.0f;
}

void UTrainPhysicsComponent::ResetPhysics() {
  PhysicsState.Velocity = 0.0f;
  PhysicsState.Acceleration = 0.0f;
  PhysicsState.DistanceTraveled = 0.0f;
  CurrentThrottle = 0.0f;
  CurrentBrake = 0.0f;
}

float UTrainPhysicsComponent::CalculateStoppingDistance() const {
  if (PhysicsState.Velocity <= 0.0f) {
    return 0.0f;
  }

  // Simple formula: d = v² / (2 * a)
  // where a = braking deceleration
  float BrakeDeceleration = 2.0f * PhysicsParameters.BrakingMultiplier;

  return (PhysicsState.Velocity * PhysicsState.Velocity) /
         (2.0f * BrakeDeceleration);
}

void UTrainPhysicsComponent::UpdateTotalMass() {
  PhysicsState.TotalMass =
      PhysicsParameters.LocomotiveMass +
      (PhysicsParameters.WagonMass * PhysicsParameters.WagonCount);
}

void UTrainPhysicsComponent::CalculateResistanceForces() {
  PhysicsState.RollingResistance = CalculateRollingResistance();
  PhysicsState.AirDragResistance = CalculateAirDrag();
  PhysicsState.GradeResistance = CalculateGradeResistance();
  PhysicsState.CurveResistance = CalculateCurveResistance();

  PhysicsState.TotalResistance =
      PhysicsState.RollingResistance + PhysicsState.AirDragResistance +
      PhysicsState.GradeResistance + PhysicsState.CurveResistance;
}

float UTrainPhysicsComponent::CalculateRollingResistance() const {
  // Rolling resistance: Fr = Crr * m * g
  return PhysicsParameters.RollingResistance * PhysicsState.TotalMass *
         PhysicsParameters.Gravity;
}

float UTrainPhysicsComponent::CalculateAirDrag() const {
  // Air drag: Fd = k * v²
  float VelocitySquared = PhysicsState.Velocity * PhysicsState.Velocity;
  return PhysicsParameters.AirDragCoefficient * VelocitySquared;
}

float UTrainPhysicsComponent::CalculateGradeResistance() const {
  // Grade resistance: Fg = m * g * sin(grade)
  float GradeRadians = FMath::DegreesToRadians(CurrentGradeDegrees);
  float SinGrade = FMath::Sin(GradeRadians);
  return PhysicsState.TotalMass * PhysicsParameters.Gravity * SinGrade;
}

float UTrainPhysicsComponent::CalculateCurveResistance() const {
  // Curve resistance increases with curvature
  if (CurrentCurvature > 0.0f) {
    return CurrentCurvature * PhysicsState.TotalMass * 0.5f;
  }
  return 0.0f;
}

float UTrainPhysicsComponent::CalculateTractiveForce() const {
  if (CurrentThrottle <= 0.0f) {
    return 0.0f;
  }

  // Calculate force from engine power
  // Power (W) = Force * Velocity
  // Force = Power / Velocity
  float PowerWatts = PhysicsParameters.EnginePowerKW * 1000.0f;
  float CurrentVelocity = FMath::Max(1.0f, PhysicsState.Velocity);

  float TractiveForce = (PowerWatts / CurrentVelocity) * CurrentThrottle;
  TractiveForce *= PhysicsParameters.AccelerationMultiplier;

  return TractiveForce;
}

float UTrainPhysicsComponent::CalculateBrakingForce() const {
  // Braking force based on multiplier
  float BrakeDeceleration =
      2.0f * CurrentBrake * PhysicsParameters.BrakingMultiplier;
  return BrakeDeceleration * PhysicsState.TotalMass;
}

bool UTrainPhysicsComponent::CheckWheelSlip(float NetForce) const {
  // Simplified wheel slip check
  // Can be expanded later if needed
  return false;
}

void UTrainPhysicsComponent::SetGear(int32 NewGear) {
  PhysicsParameters.CurrentGear =
      FMath::Clamp(NewGear, 0, PhysicsParameters.MaxGears);

  if (bEnableDebugLogs) {
    if (PhysicsParameters.CurrentGear == 0) {
      UE_LOG(LogTemp, Log, TEXT("Gear: NEUTRAL"));
    } else {
      UE_LOG(LogTemp, Log, TEXT("Gear: %d (Max Speed: %.1f km/h)"),
             PhysicsParameters.CurrentGear, GetCurrentGearMaxSpeed());
    }
  }
}

float UTrainPhysicsComponent::GetCurrentGearMaxSpeed() const {
  if (PhysicsParameters.CurrentGear == 0 ||
      PhysicsParameters.CurrentGear >
          PhysicsParameters.GearMaxSpeedsKmh.Num()) {
    return 0.0f;
  }

  return PhysicsParameters.GearMaxSpeedsKmh[PhysicsParameters.CurrentGear - 1];
}

float UTrainPhysicsComponent::GetCurrentGearAccelMultiplier() const {
  if (PhysicsParameters.CurrentGear == 0 ||
      PhysicsParameters.CurrentGear >
          PhysicsParameters.GearAccelerationMultipliers.Num()) {
    return 1.0f;
  }

  return PhysicsParameters
      .GearAccelerationMultipliers[PhysicsParameters.CurrentGear - 1];
}

void UTrainPhysicsComponent::SetDirection(float Direction) {
  float NewDirection = (Direction >= 0.0f) ? 1.0f : -1.0f;

  // Check if direction is changing
  if (NewDirection != PhysicsParameters.DirectionMultiplier) {
    // Check if train is moving
    if (AbsoluteVelocity > 0.5f) // 0.5 m/s threshold
    {
      // Force braking when changing direction while moving
      CurrentBrake = 1.0f;
      CurrentThrottle = 0.0f;

      if (bEnableDebugLogs) {
        UE_LOG(LogTemp, Warning,
               TEXT("Cannot change direction while moving (%.1f km/h) - Apply "
                    "brake first!"),
               AbsoluteVelocity * 3.6f);
      }

      // Don't change direction yet
      return;
    }

    // Train is stopped - safe to change direction
    PhysicsParameters.DirectionMultiplier = NewDirection;

    if (bEnableDebugLogs) {
      UE_LOG(LogTemp, Log, TEXT("Direction changed to: %s"),
             PhysicsParameters.DirectionMultiplier > 0.0f ? TEXT("Forward")
                                                          : TEXT("Reverse"));
    }
  }
}


void UTrainPhysicsComponent::UpdatePhysics(float DeltaTime) {
  // Calculate total mass
  UpdateTotalMass();

  // Get max speed for current gear (in m/s)
  float GearMaxSpeedKmh = GetCurrentGearMaxSpeed();
  float MaxSpeedMs = (GearMaxSpeedKmh > 0.0f) ? GearMaxSpeedKmh / 3.6f : 0.0f;

  // If in neutral gear (0), don't allow throttle
  if (PhysicsParameters.CurrentGear == 0) {
    CurrentThrottle = 0.0f;
  }

  // ========== Calculate Acceleration ==========

  float TargetAcceleration = 0.0f;

  if (CurrentThrottle > 0.0f && MaxSpeedMs > 0.0f) {
    // Calculate acceleration from engine power
    float PowerWatts = PhysicsParameters.EnginePowerKW * 1000.0f;
    float CurrentVelocity = FMath::Max(1.0f, AbsoluteVelocity);

    float TractiveForce = (PowerWatts / CurrentVelocity) * CurrentThrottle;

    // Apply base acceleration multiplier
    TractiveForce *= PhysicsParameters.AccelerationMultiplier;

    // Apply gear-specific acceleration multiplier
    float GearAccelMultiplier = GetCurrentGearAccelMultiplier();
    TractiveForce *= GearAccelMultiplier;

    // Calculate acceleration
    TargetAcceleration = TractiveForce / PhysicsState.TotalMass;

    PhysicsState.AppliedTractiveForce = TractiveForce;
  } else {
    PhysicsState.AppliedTractiveForce = 0.0f;
  }

  // ========== Calculate Braking ==========

  if (CurrentBrake > 0.0f) {
    float BrakeDeceleration =
        2.0f * CurrentBrake * PhysicsParameters.BrakingMultiplier;
    TargetAcceleration -= BrakeDeceleration;

    PhysicsState.AppliedBrakingForce =
        BrakeDeceleration * PhysicsState.TotalMass;
  } else {
    PhysicsState.AppliedBrakingForce = 0.0f;
  }

  // ========== Calculate Resistance Forces ==========

  CalculateResistanceForces();

  // Subtract resistance from acceleration
  float ResistanceDeceleration =
      PhysicsState.TotalResistance / PhysicsState.TotalMass;
  TargetAcceleration -= ResistanceDeceleration;

  // ========== Update Velocity ==========

  PhysicsState.Acceleration = TargetAcceleration;

  // Update absolute velocity (always positive)
  AbsoluteVelocity += PhysicsState.Acceleration * DeltaTime;

  // Clamp to prevent negative
  AbsoluteVelocity = FMath::Max(0.0f, AbsoluteVelocity);

  // Calculate speed limits based on direction
  float MaxSpeedForward = MaxSpeedMs;
  float MaxSpeedReverse =
      MaxSpeedMs * PhysicsParameters.ReverseSpeedLimitPercent;

  // Clamp to gear limits
  if (PhysicsParameters.DirectionMultiplier > 0.0f) {
    // Forward direction
    AbsoluteVelocity = FMath::Clamp(AbsoluteVelocity, 0.0f, MaxSpeedForward);
  } else {
    // Reverse direction
    AbsoluteVelocity = FMath::Clamp(AbsoluteVelocity, 0.0f, MaxSpeedReverse);
  }

  // Apply direction to get signed velocity
  PhysicsState.Velocity =
      AbsoluteVelocity * PhysicsParameters.DirectionMultiplier;

  // Update distance traveled (use absolute value)
  if (AbsoluteVelocity > 0.0f) {
    PhysicsState.DistanceTraveled += AbsoluteVelocity * DeltaTime;
  }

  // ========== Debug Logging ==========

  if (bEnableDebugLogs) {
    float CurrentSpeedKmh = AbsoluteVelocity * 3.6f;
    float MaxSpeedKmh =
        (PhysicsParameters.DirectionMultiplier > 0.0f)
            ? GearMaxSpeedKmh
            : GearMaxSpeedKmh * PhysicsParameters.ReverseSpeedLimitPercent;

    UE_LOG(LogTemp, Log, TEXT("=== PHYSICS UPDATE ==="));
    UE_LOG(LogTemp, Log, TEXT("  Gear: %d, Direction: %.1f (%s)"),
           PhysicsParameters.CurrentGear, PhysicsParameters.DirectionMultiplier,
           PhysicsParameters.DirectionMultiplier > 0.0f ? TEXT("FWD")
                                                        : TEXT("REV"));
    UE_LOG(LogTemp, Log, TEXT("  Throttle: %.2f, Brake: %.2f"), CurrentThrottle,
           CurrentBrake);
    UE_LOG(LogTemp, Log, TEXT("  AbsoluteVelocity: %.2f m/s (%.1f km/h)"),
           AbsoluteVelocity, CurrentSpeedKmh);
    UE_LOG(LogTemp, Log, TEXT("  PhysicsState.Velocity: %.2f m/s (SIGNED)"),
           PhysicsState.Velocity);
    UE_LOG(LogTemp, Log, TEXT("  Acceleration: %.2f m/s²"),
           PhysicsState.Acceleration);
    UE_LOG(LogTemp, Log, TEXT("  Max Speed: %.1f km/h"), MaxSpeedKmh);
  }
}



float UTrainPhysicsComponent::SmoothInput(float Current, float Target,
                                          float DeltaTime,
                                          float SmoothingSpeed) {
  return FMath::FInterpTo(Current, Target, DeltaTime, SmoothingSpeed);
}
