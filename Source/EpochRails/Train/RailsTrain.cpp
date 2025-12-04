// RailsTrain.cpp

#include "RailsTrain.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RailsSplinePath.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

ARailsTrain::ARailsTrain() {
  PrimaryActorTick.bCanEverTick = true;

  // Create root component
  TrainRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TrainRoot"));
  RootComponent = TrainRoot;

  // Create train body mesh
  TrainBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrainBodyMesh"));
  TrainBodyMesh->SetupAttachment(TrainRoot);
  TrainBodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  TrainBodyMesh->SetCollisionProfileName(TEXT("BlockAll"));

  // Create platform mesh
  PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
  PlatformMesh->SetupAttachment(TrainRoot);
  PlatformMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  PlatformMesh->SetCollisionProfileName(TEXT("BlockAll"));
  
  // CRITICAL: Setup for moving platform behavior
  PlatformMesh->SetSimulatePhysics(false);
  PlatformMesh->SetEnableGravity(false);
  PlatformMesh->CanCharacterStepUpOn = ECB_Yes;

  // Create boarding zone
  BoardingZone = CreateDefaultSubobject<UBoxComponent>(TEXT("BoardingZone"));
  BoardingZone->SetupAttachment(PlatformMesh);
  BoardingZone->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
  BoardingZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  BoardingZone->SetCollisionProfileName(TEXT("OverlapAll"));

  // Create physics component
  PhysicsComponent = CreateDefaultSubobject<UTrainPhysicsComponent>(TEXT("PhysicsComponent"));
}

void ARailsTrain::BeginPlay() {
  Super::BeginPlay();

  // Bind boarding zone events
  if (BoardingZone) {
    BoardingZone->OnComponentBeginOverlap.AddDynamic(
        this, &ARailsTrain::OnBoardingZoneBeginOverlap);
    BoardingZone->OnComponentEndOverlap.AddDynamic(
        this, &ARailsTrain::OnBoardingZoneEndOverlap);
  }

  // Cache spline component
  if (SplinePathRef) {
    CachedSplineComponent = SplinePathRef->GetSpline();
  }

  // Auto-start if enabled
  if (bAutoStart) {
    StartTrain();
  }
}

void ARailsTrain::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  // Update movement
  UpdateTrainMovement(DeltaTime);
  
  // Update passengers to maintain relative physics
  UpdatePassengersPhysics(DeltaTime);

  // Draw debug info if enabled
  if (bShowPhysicsDebug && bUsePhysicsSimulation) {
    DrawPhysicsDebug();
  }
}

// ========== Movement Functions ==========

void ARailsTrain::UpdateTrainMovement(float DeltaTime) {
  if (!CachedSplineComponent) {
    return;
  }

  if (bUsePhysicsSimulation) {
    UpdatePhysicsMovement(DeltaTime);
  } else {
    UpdateLegacyMovement(DeltaTime);
  }
}

void ARailsTrain::UpdatePhysicsMovement(float DeltaTime) {
  if (!PhysicsComponent || !CachedSplineComponent) {
    return;
  }

  // Update physics parameters based on track
  UpdatePhysicsParameters(DeltaTime);

  // Apply throttle and brake to physics
  PhysicsComponent->SetThrottle(FMath::Max(0.0f, CurrentThrottle));
  PhysicsComponent->SetBrake(CurrentBrake);

  // Get velocity from physics (in m/s)
  float VelocityMs = PhysicsComponent->GetVelocityMs();
  
  // Convert to cm/s for Unreal units
  CurrentSpeed = VelocityMs * 100.0f;

  // Update distance along spline
  CurrentDistance += VelocityMs * 100.0f * DeltaTime;

  // Handle looping
  float SplineLength = CachedSplineComponent->GetSplineLength();
  if (bLoopPath) {
    if (CurrentDistance >= SplineLength) {
      CurrentDistance = FMath::Fmod(CurrentDistance, SplineLength);
    }
  } else {
    CurrentDistance = FMath::Clamp(CurrentDistance, 0.0f, SplineLength);
    if (CurrentDistance >= SplineLength && TrainState == ETrainState::Moving) {
      StopTrain();
    }
  }

  // Move to new position
  MoveToDistance(CurrentDistance);

  // Update state based on acceleration
  float Acceleration = PhysicsComponent->PhysicsState.Acceleration;
  if (FMath::Abs(Acceleration) < 0.01f) {
    TrainState = (CurrentSpeed > 1.0f) ? ETrainState::Moving : ETrainState::Stopped;
  } else if (Acceleration > 0.01f) {
    TrainState = ETrainState::Accelerating;
  } else {
    TrainState = ETrainState::Decelerating;
  }
}

void ARailsTrain::UpdateLegacyMovement(float DeltaTime) {
  if (!CachedSplineComponent) {
    return;
  }

  // Calculate target speed
  float TargetSpeed = GetTargetSpeed();

  // Interpolate current speed towards target
  if (CurrentSpeed < TargetSpeed) {
    CurrentSpeed = FMath::FInterpConstantTo(CurrentSpeed, TargetSpeed,
                                            DeltaTime, AccelerationRate);
    TrainState = ETrainState::Accelerating;
  } else if (CurrentSpeed > TargetSpeed) {
    CurrentSpeed = FMath::FInterpConstantTo(CurrentSpeed, TargetSpeed,
                                            DeltaTime, DecelerationRate);
    TrainState = ETrainState::Decelerating;
  } else {
    TrainState = (CurrentSpeed > 1.0f) ? ETrainState::Moving : ETrainState::Stopped;
  }

  // Update distance along spline
  CurrentDistance += CurrentSpeed * DeltaTime;

  // Handle looping
  float SplineLength = CachedSplineComponent->GetSplineLength();
  if (bLoopPath) {
    if (CurrentDistance >= SplineLength) {
      CurrentDistance = FMath::Fmod(CurrentDistance, SplineLength);
    }
  } else {
    CurrentDistance = FMath::Clamp(CurrentDistance, 0.0f, SplineLength);
    if (CurrentDistance >= SplineLength && TrainState == ETrainState::Moving) {
      StopTrain();
    }
  }

  // Move to new position
  MoveToDistance(CurrentDistance);
}

void ARailsTrain::MoveToDistance(float Distance) {
  if (!CachedSplineComponent) {
    return;
  }

  // Get location and rotation at distance
  FVector NewLocation = CachedSplineComponent->GetLocationAtDistanceAlongSpline(
      Distance, ESplineCoordinateSpace::World);
  FRotator NewRotation = CachedSplineComponent->GetRotationAtDistanceAlongSpline(
      Distance, ESplineCoordinateSpace::World);

  // Use sweep to prevent characters from being ejected
  // This allows physics to properly update attached actors
  SetActorLocationAndRotation(NewLocation, NewRotation, true);
}

float ARailsTrain::GetTargetSpeed() const {
  if (TrainState == ETrainState::Stopped) {
    return 0.0f;
  }
  
  // Apply throttle as percentage of max speed
  return MaxSpeed * FMath::Abs(CurrentThrottle);
}

// ========== Physics Helper Functions ==========

void ARailsTrain::UpdatePassengersPhysics(float DeltaTime) {
  // Keep passengers synced with train movement
  for (ACharacter* Passenger : PassengersOnBoard) {
    if (Passenger && Passenger->IsValidLowLevel()) {
      UCharacterMovementComponent* MovementComp = Passenger->GetCharacterMovement();
      if (MovementComp) {
        // When character is falling (jumped), maintain train velocity
        if (MovementComp->IsFalling()) {
          // Calculate train's world velocity
          FVector TrainVelocity = GetVelocity();
          
          // Blend train velocity into character's velocity
          // This makes character stay relative to train even when jumping
          FVector CurrentVelocity = MovementComp->Velocity;
          FVector RelativeVelocity = CurrentVelocity - TrainVelocity;
          
          // Apply train velocity as base, keep only relative movement
          MovementComp->Velocity = TrainVelocity + RelativeVelocity;
          
          // Check if character should land back on platform
          FVector CharacterLocation = Passenger->GetActorLocation();
          FVector PlatformLocation = PlatformMesh->GetComponentLocation();
          float DistanceToPlatform = FVector::Dist(CharacterLocation, PlatformLocation);
          
          // If close enough and moving downward, snap back to platform
          if (DistanceToPlatform < 200.0f && MovementComp->Velocity.Z < 0.0f) {
            // Force walking mode to re-attach
            MovementComp->SetMovementMode(MOVE_Walking);
          }
        }
      }
    }
  }
}

float ARailsTrain::CalculateTrackGrade() {
  if (!CachedSplineComponent) {
    return 0.0f;
  }

  // Get tangent at current position
  FVector CurrentTangent = CachedSplineComponent->GetTangentAtDistanceAlongSpline(
      CurrentDistance, ESplineCoordinateSpace::World);
  
  // Get tangent ahead
  float AheadDistance = CurrentDistance + PhysicsSampleDistance;
  float SplineLength = CachedSplineComponent->GetSplineLength();
  if (AheadDistance > SplineLength && bLoopPath) {
    AheadDistance = FMath::Fmod(AheadDistance, SplineLength);
  }
  
  FVector AheadTangent = CachedSplineComponent->GetTangentAtDistanceAlongSpline(
      AheadDistance, ESplineCoordinateSpace::World);

  // Calculate average grade
  CurrentTangent.Normalize();
  AheadTangent.Normalize();
  FVector AverageTangent = (CurrentTangent + AheadTangent) * 0.5f;
  AverageTangent.Normalize();

  // Calculate grade angle in degrees
  // Grade = arcsin(Z / |Tangent|)
  float GradeDegrees = FMath::RadiansToDegrees(
      FMath::Asin(AverageTangent.Z));

  return GradeDegrees;
}

float ARailsTrain::CalculateTrackCurvature() {
  if (!CachedSplineComponent) {
    return 0.0f;
  }

  // Get direction at current position
  FVector CurrentDir = CachedSplineComponent->GetDirectionAtDistanceAlongSpline(
      CurrentDistance, ESplineCoordinateSpace::World);
  
  // Get direction ahead
  float AheadDistance = CurrentDistance + PhysicsSampleDistance;
  float SplineLength = CachedSplineComponent->GetSplineLength();
  if (AheadDistance > SplineLength && bLoopPath) {
    AheadDistance = FMath::Fmod(AheadDistance, SplineLength);
  }
  
  FVector AheadDir = CachedSplineComponent->GetDirectionAtDistanceAlongSpline(
      AheadDistance, ESplineCoordinateSpace::World);

  // Calculate angle between directions
  float DotProduct = FVector::DotProduct(CurrentDir, AheadDir);
  DotProduct = FMath::Clamp(DotProduct, -1.0f, 1.0f);
  float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

  // Normalize to 0-1 range (0 = straight, 1 = very tight curve)
  // Assuming 90 degrees is maximum practical curve
  float Curvature = FMath::Clamp(AngleDegrees / 90.0f, 0.0f, 1.0f);

  return Curvature;
}

void ARailsTrain::UpdatePhysicsParameters(float DeltaTime) {
  if (!PhysicsComponent) {
    return;
  }

  // Calculate current track parameters
  float TargetGrade = CalculateTrackGrade();
  float TargetCurvature = CalculateTrackCurvature();

  // Smooth the transitions
  SmoothedGrade = FMath::FInterpTo(SmoothedGrade, TargetGrade, 
                                   DeltaTime, GradeSmoothingSpeed);
  SmoothedCurvature = FMath::FInterpTo(SmoothedCurvature, TargetCurvature, 
                                       DeltaTime, GradeSmoothingSpeed);

  // Update physics component
  PhysicsComponent->SetTrackGrade(SmoothedGrade);
  PhysicsComponent->SetTrackCurvature(SmoothedCurvature);
}

void ARailsTrain::DrawPhysicsDebug() {
  if (!PhysicsComponent) {
    return;
  }

  // Build debug string
  FString DebugText = FString::Printf(
      TEXT("=== TRAIN PHYSICS DEBUG ===\n")
      TEXT("Speed: %.1f km/h (%.1f m/s)\n")
      TEXT("Acceleration: %.2f m/s²\n")
      TEXT("Mass: %.0f kg\n")
      TEXT("\n")
      TEXT("Forces:\n")
      TEXT("  Tractive: %.0f N\n")
      TEXT("  Braking: %.0f N\n")
      TEXT("  Total Resistance: %.0f N\n")
      TEXT("\n")
      TEXT("Resistance Breakdown:\n")
      TEXT("  Rolling: %.0f N\n")
      TEXT("  Air Drag: %.0f N\n")
      TEXT("  Grade: %.0f N (%.1f°)\n")
      TEXT("  Curve: %.0f N (%.2f)\n")
      TEXT("\n")
      TEXT("Track:\n")
      TEXT("  Grade: %.2f°\n")
      TEXT("  Curvature: %.2f\n")
      TEXT("\n")
      TEXT("Status:\n")
      TEXT("  Wheel Slip: %s\n")
      TEXT("  Stopping Distance: %.0f m\n")
      TEXT("  Distance Traveled: %.0f m\n")
      TEXT("  Passengers: %d"),
      PhysicsComponent->GetVelocityKmh(),
      PhysicsComponent->GetVelocityMs(),
      PhysicsComponent->PhysicsState.Acceleration,
      PhysicsComponent->PhysicsState.TotalMass,
      PhysicsComponent->PhysicsState.AppliedTractiveForce,
      PhysicsComponent->PhysicsState.AppliedBrakingForce,
      PhysicsComponent->PhysicsState.TotalResistance,
      PhysicsComponent->PhysicsState.RollingResistance,
      PhysicsComponent->PhysicsState.AirDragResistance,
      PhysicsComponent->PhysicsState.GradeResistance,
      SmoothedGrade,
      PhysicsComponent->PhysicsState.CurveResistance,
      SmoothedCurvature,
      SmoothedGrade,
      SmoothedCurvature,
      PhysicsComponent->PhysicsState.bIsWheelSlipping ? TEXT("YES") : TEXT("NO"),
      PhysicsComponent->CalculateStoppingDistance(),
      PhysicsComponent->PhysicsState.DistanceTraveled,
      PassengersOnBoard.Num()
  );

  // Display on screen
  GEngine->AddOnScreenDebugMessage(
      -1, 0.0f, 
      PhysicsComponent->PhysicsState.bIsWheelSlipping ? FColor::Red : FColor::Green, 
      DebugText
  );

  // Draw grade visualization
  FVector TrainLocation = GetActorLocation();
  FVector UpVector = GetActorUpVector();
  FVector ForwardVector = GetActorForwardVector();
  
  // Draw grade line
  float GradeVisualizationLength = 500.0f;
  FVector GradeEndPoint = TrainLocation + 
      ForwardVector * GradeVisualizationLength * FMath::Cos(FMath::DegreesToRadians(SmoothedGrade)) +
      UpVector * GradeVisualizationLength * FMath::Sin(FMath::DegreesToRadians(SmoothedGrade));
  
  FColor GradeColor = SmoothedGrade > 0.0f ? FColor::Red : (SmoothedGrade < 0.0f ? FColor::Green : FColor::White);
  DrawDebugLine(GetWorld(), TrainLocation, GradeEndPoint, GradeColor, false, -1.0f, 0, 5.0f);
}

// ========== Collision Events ==========

void ARailsTrain::OnBoardingZoneBeginOverlap(
    UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult &SweepResult) {
  
  ACharacter *Character = Cast<ACharacter>(OtherActor);
  if (Character && !PassengersOnBoard.Contains(Character)) {
    PassengersOnBoard.Add(Character);
    
    // Use KeepWorld for smooth transition
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::KeepWorld,
        EAttachmentRule::KeepWorld,
        EAttachmentRule::KeepWorld,
        false  // Don't weld - allows relative physics
    );
    
    Character->AttachToComponent(PlatformMesh, AttachRules);
    
    // CRITICAL: Setup character movement for relative space physics
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (MovementComp) {
      // These settings make character work in "train's local space"
      MovementComp->SetMovementMode(MOVE_Walking);
      
      // KEY: Inherit base velocity - character now in train's reference frame
      MovementComp->bImpartBaseVelocityX = true;
      MovementComp->bImpartBaseVelocityY = true;
      MovementComp->bImpartBaseVelocityZ = true;
      MovementComp->bImpartBaseAngularVelocity = true;
      
      // Don't ignore base rotation - follow train's rotation
      MovementComp->bIgnoreBaseRotation = false;
      
      // Floor checking
      MovementComp->bAlwaysCheckFloor = true;
      MovementComp->bUseFlatBaseForFloorChecks = true;
      
      // Perch settings for stable standing
      MovementComp->PerchRadiusThreshold = 0.0f;
      MovementComp->PerchAdditionalHeight = 0.0f;
      
      // Disable physics interaction with external objects
      MovementComp->bEnablePhysicsInteraction = false;
    }
  }
}

void ARailsTrain::OnBoardingZoneEndOverlap(
    UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex) {
  
  ACharacter *Character = Cast<ACharacter>(OtherActor);
  if (Character && PassengersOnBoard.Contains(Character)) {
    PassengersOnBoard.Remove(Character);
    
    // Proper detachment
    FDetachmentTransformRules DetachRules(
        EDetachmentRule::KeepWorld,
        EDetachmentRule::KeepWorld,
        EDetachmentRule::KeepWorld,
        true
    );
    
    Character->DetachFromActor(DetachRules);
    
    // Restore character movement settings to world space
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (MovementComp) {
      MovementComp->bImpartBaseVelocityX = false;
      MovementComp->bImpartBaseVelocityY = false;
      MovementComp->bImpartBaseVelocityZ = false;
      MovementComp->bImpartBaseAngularVelocity = false;
      MovementComp->bIgnoreBaseRotation = true;
      MovementComp->bAlwaysCheckFloor = true;
      MovementComp->bUseFlatBaseForFloorChecks = false;
      MovementComp->bEnablePhysicsInteraction = true;
    }
  }
}

// ========== Public API ==========

void ARailsTrain::StartTrain() {
  if (bUsePhysicsSimulation && PhysicsComponent) {
    CurrentThrottle = 0.5f; // Start with 50% throttle
    CurrentBrake = 0.0f;
  } else {
    CurrentThrottle = 1.0f;
  }
  TrainState = ETrainState::Accelerating;
}

void ARailsTrain::StopTrain() {
  if (bUsePhysicsSimulation && PhysicsComponent) {
    CurrentThrottle = 0.0f;
    CurrentBrake = 1.0f; // Apply full brake
  } else {
    CurrentThrottle = 0.0f;
    CurrentSpeed = 0.0f;
  }
  TrainState = ETrainState::Stopped;
}

void ARailsTrain::SetSpeed(float NewSpeed) {
  if (!bUsePhysicsSimulation) {
    CurrentSpeed = FMath::Clamp(NewSpeed, 0.0f, MaxSpeed);
  }
}

float ARailsTrain::GetCurrentSpeedKmh() const {
  if (bUsePhysicsSimulation && PhysicsComponent) {
    return PhysicsComponent->GetVelocityKmh();
  }
  // Convert cm/s to km/h
  return (CurrentSpeed / 100.0f) * 3.6f;
}

bool ARailsTrain::IsCharacterOnTrain(ACharacter *Character) const {
  return PassengersOnBoard.Contains(Character);
}

void ARailsTrain::ApplyThrottle(float ThrottleValue) {
  CurrentThrottle = FMath::Clamp(ThrottleValue, -1.0f, 1.0f);
  
  if (bUsePhysicsSimulation && PhysicsComponent) {
    PhysicsComponent->SetThrottle(FMath::Max(0.0f, CurrentThrottle));
  }
}

void ARailsTrain::ApplyBrake(float BrakeValue) {
  CurrentBrake = FMath::Clamp(BrakeValue, 0.0f, 1.0f);
  
  if (bUsePhysicsSimulation && PhysicsComponent) {
    PhysicsComponent->SetBrake(CurrentBrake);
  }
}

void ARailsTrain::EmergencyBrake() {
  CurrentThrottle = 0.0f;
  CurrentBrake = 1.0f;
  
  if (bUsePhysicsSimulation && PhysicsComponent) {
    PhysicsComponent->EmergencyBrake();
  }
  
  TrainState = ETrainState::Decelerating;
}

float ARailsTrain::GetStoppingDistance() const {
  if (bUsePhysicsSimulation && PhysicsComponent) {
    return PhysicsComponent->CalculateStoppingDistance();
  }
  
  // Simple kinematic calculation for legacy mode
  if (CurrentSpeed > 0.0f && DecelerationRate > 0.0f) {
    return (CurrentSpeed * CurrentSpeed) / (2.0f * DecelerationRate);
  }
  
  return 0.0f;
}

void ARailsTrain::AddWagons(int32 Count) {
  if (PhysicsComponent) {
    PhysicsComponent->AddWagons(Count);
  }
}

void ARailsTrain::RemoveWagons(int32 Count) {
  if (PhysicsComponent) {
    PhysicsComponent->RemoveWagons(Count);
  }
}
