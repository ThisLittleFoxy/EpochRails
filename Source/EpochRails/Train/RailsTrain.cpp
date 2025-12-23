// RailsTrain.cpp

#include "RailsTrain.h"
#include "Character/RailsPlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EpochRails.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "Kismet/KismetMathLibrary.h"
#include "RailsSplinePath.h"
#include "TrainSpeedometerWidget.h"

ARailsTrain::ARailsTrain() {
  PrimaryActorTick.bCanEverTick = true;

  // Create root component
  TrainRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TrainRoot"));
  RootComponent = TrainRoot;

  // Create train body mesh
  TrainBodyMesh =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrainBodyMesh"));
  TrainBodyMesh->SetupAttachment(TrainRoot);
  TrainBodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  TrainBodyMesh->SetCollisionProfileName(TEXT("BlockAll"));

  // Create platform mesh
  PlatformMesh =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
  PlatformMesh->SetupAttachment(TrainRoot);
  PlatformMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  PlatformMesh->SetCollisionProfileName(TEXT("OverlapAll"));
  PlatformMesh->SetSimulatePhysics(false);
  PlatformMesh->SetEnableGravity(false);

  // Create physics component
  PhysicsComponent =
      CreateDefaultSubobject<UTrainPhysicsComponent>(TEXT("PhysicsComponent"));

  // Create trigger box for train interior
  TrainInteriorTrigger =
      CreateDefaultSubobject<UBoxComponent>(TEXT("TrainInteriorTrigger"));
  TrainInteriorTrigger->SetupAttachment(RootComponent);
  TrainInteriorTrigger->SetBoxExtent(FVector(500.f, 250.f, 200.f));
  TrainInteriorTrigger->SetCollisionProfileName(TEXT("Trigger"));
  TrainInteriorTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  TrainInteriorTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
  TrainInteriorTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

  // Initialize IMC priority
  IMCPriority = 0;

  // Create control panel mesh
  ControlPanelMesh =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ControlPanelMesh"));
  ControlPanelMesh->SetupAttachment(TrainRoot);
  ControlPanelMesh->SetRelativeLocation(FVector(250.0f, 0.0f, 120.0f));
  ControlPanelMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
  ControlPanelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  // Create speedometer widget component
  SpeedometerWidgetComponent =
      CreateDefaultSubobject<UWidgetComponent>(TEXT("SpeedometerWidget"));
  SpeedometerWidgetComponent->SetupAttachment(ControlPanelMesh);
  SpeedometerWidgetComponent->SetRelativeLocation(SpeedometerRelativeLocation);
  SpeedometerWidgetComponent->SetRelativeRotation(SpeedometerRelativeRotation);
  SpeedometerWidgetComponent->SetDrawSize(SpeedometerDrawSize);
  SpeedometerWidgetComponent->SetWidgetSpace(SpeedometerWidgetSpace);
  SpeedometerWidgetComponent->SetCollisionEnabled(
      ECollisionEnabled::NoCollision);
  SpeedometerWidgetComponent->SetVisibility(bShowSpeedometer);

  // Create control panel widget component
  ControlPanelWidgetComponent =
      CreateDefaultSubobject<UWidgetComponent>(TEXT("ControlPanelWidget"));
  ControlPanelWidgetComponent->SetupAttachment(ControlPanelMesh);
  ControlPanelWidgetComponent->SetRelativeLocation(
      ControlPanelRelativeLocation);
  ControlPanelWidgetComponent->SetRelativeRotation(
      ControlPanelRelativeRotation);
  ControlPanelWidgetComponent->SetDrawSize(ControlPanelDrawSize);
  ControlPanelWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
  ControlPanelWidgetComponent->SetCollisionEnabled(
      ECollisionEnabled::QueryOnly);
  // === LOAD WIDGET CLASSES IN CONSTRUCTOR ===
  static ConstructorHelpers::FClassFinder<UUserWidget> SpeedometerWidgetFinder(
      TEXT("/Game/Train/BP_Train/UI_Train/"
           "WBP_TrainSpeedometer.WBP_TrainSpeedometer_C"));
  if (SpeedometerWidgetFinder.Succeeded()) {
    SpeedometerWidgetClass = SpeedometerWidgetFinder.Class;
  }

  static ConstructorHelpers::FClassFinder<UUserWidget> ControlPanelWidgetFinder(
      TEXT("/Game/Train/BP_Train/UI_Train/"
           "WBP_TrainControlPanel.WBP_TrainControlPanel_C"));
  if (ControlPanelWidgetFinder.Succeeded()) {
    ControlPanelWidgetClass = ControlPanelWidgetFinder.Class;
  }
}

void ARailsTrain::BeginPlay() {
  Super::BeginPlay();

  // Cache spline component
  if (SplinePathRef) {
    CachedSplineComponent = SplinePathRef->GetSpline();
  }

  // Auto-start if enabled
  if (bAutoStart) {
    StartTrain();
  }

  // Bind overlap events for train interior trigger
  if (TrainInteriorTrigger) {
    TrainInteriorTrigger->OnComponentBeginOverlap.AddDynamic(
        this, &ARailsTrain::OnTrainInteriorBeginOverlap);
    TrainInteriorTrigger->OnComponentEndOverlap.AddDynamic(
        this, &ARailsTrain::OnTrainInteriorEndOverlap);
    UE_LOG(LogEpochRails, Log,
           TEXT("Train interior trigger configured for train: %s"), *GetName());
  }

  // Initialize UI widgets
  InitializeSpeedometer();
  InitializeControlPanel();
}

void ARailsTrain::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  // Update gear shift timer
  TimeSinceLastGearShift += DeltaTime;

  // Apply continuous braking if button held
  if (bBrakeButtonHeld) {
    CurrentThrottle = FMath::Max(0.0f, CurrentThrottle - (0.5f * DeltaTime));
    CurrentBrake = 1.0f;
  }
  // Update movement
  UpdateTrainMovement(DeltaTime);

  // Draw debug info if enabled
  if (bShowPhysicsDebug && bUsePhysicsSimulation) {
    DrawPhysicsDebug();
  }

  // Update speedometer display
  UpdateSpeedometerDisplay();
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
    if (bShowPhysicsDebug) {
      UE_LOG(
          LogTemp, Error,
          TEXT("UpdatePhysicsMovement: PhysicsComponent or Spline is NULL!"));
    }
    return;
  }

  // Update physics parameters based on track
  UpdatePhysicsParameters(DeltaTime);

  // Apply throttle and brake to physics
  PhysicsComponent->SetThrottle(CurrentThrottle);
  PhysicsComponent->SetBrake(CurrentBrake);

  // Get velocity from physics (in m/s, with direction sign)
  float VelocityMs = PhysicsComponent->PhysicsState.Velocity; // Signed velocity

  // LOG THIS:
  if (bShowPhysicsDebug) {
    UE_LOG(LogTemp, Warning, TEXT("=== RAILS TRAIN UPDATE ==="));
    UE_LOG(LogTemp, Warning, TEXT("  VelocityMs from Physics: %.2f m/s"),
           VelocityMs);
    UE_LOG(LogTemp, Warning, TEXT("  CurrentDistance BEFORE: %.1f cm"),
           CurrentDistance);
  }

  // Convert to cm/s for Unreal units (absolute for display)
  CurrentSpeed = FMath::Abs(VelocityMs) * 100.0f;

  // Update distance along spline (velocity already includes direction)
  float DistanceDelta = VelocityMs * 100.0f * DeltaTime;
  CurrentDistance += DistanceDelta;

  // LOG THIS:
  if (bShowPhysicsDebug) {
    UE_LOG(LogTemp, Warning,
           TEXT("  DistanceDelta: %.2f cm (VelocityMs=%.2f * 100 * "
                "DeltaTime=%.4f)"),
           DistanceDelta, VelocityMs, DeltaTime);
    UE_LOG(LogTemp, Warning, TEXT("  CurrentDistance AFTER: %.1f cm"),
           CurrentDistance);
  }

  // Handle looping and bounds
  float SplineLength = CachedSplineComponent->GetSplineLength();

  if (bLoopPath) {
    // Wrap around for looping paths
    while (CurrentDistance >= SplineLength) {
      CurrentDistance -= SplineLength;
    }
    while (CurrentDistance < 0.0f) {
      CurrentDistance += SplineLength;
    }
  } else {
    // Clamp for non-looping paths
    float OldDistance = CurrentDistance;
    CurrentDistance = FMath::Clamp(CurrentDistance, 0.0f, SplineLength);

    // Stop if reached the end
    if ((OldDistance >= SplineLength || OldDistance <= 0.0f) &&
        TrainState == ETrainState::Moving) {
      StopTrain();
    }
  }

  // Move to new position
  MoveToDistance(CurrentDistance);

  // Update state based on acceleration
  float Acceleration = PhysicsComponent->PhysicsState.Acceleration;
  if (FMath::Abs(Acceleration) < 0.01f) {
    TrainState =
        (CurrentSpeed > 1.0f) ? ETrainState::Moving : ETrainState::Stopped;
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
    TrainState =
        (CurrentSpeed > 1.0f) ? ETrainState::Moving : ETrainState::Stopped;
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
  FRotator NewRotation =
      CachedSplineComponent->GetRotationAtDistanceAlongSpline(
          Distance, ESplineCoordinateSpace::World);

  // Use sweep to prevent characters from being ejected
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

float ARailsTrain::CalculateTrackGrade() {
  if (!CachedSplineComponent) {
    return 0.0f;
  }

  // Get tangent at current position
  FVector CurrentTangent =
      CachedSplineComponent->GetTangentAtDistanceAlongSpline(
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
  float GradeDegrees = FMath::RadiansToDegrees(FMath::Asin(AverageTangent.Z));
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
  SmoothedGrade = FMath::FInterpTo(SmoothedGrade, TargetGrade, DeltaTime,
                                   GradeSmoothingSpeed);
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
      TEXT("=== TRAIN PHYSICS DEBUG ===\n") TEXT(
          "Speed: %.1f km/h (%.1f m/s)\n") TEXT("Acceleration: %.2f m/s²\n")
          TEXT("Mass: %.0f kg\n") TEXT("\n") TEXT("Forces:\n") TEXT(
              " Tractive: %.0f N\n") TEXT(" Braking: %.0f N\n")
              TEXT(" Total Resistance: %.0f N\n") TEXT("\n") TEXT(
                  "Resistance Breakdown:\n") TEXT(" Rolling: %.0f N\n")
                  TEXT(" Air Drag: %.0f N\n") TEXT(" Grade: %.0f N (%.1f°)\n")
                      TEXT(" Curve: %.0f N (%.2f)\n") TEXT("\n")
                          TEXT("Track:\n") TEXT(" Grade: %.2f°\n") TEXT(
                              " Curvature: %.2f\n") TEXT("\n") TEXT("Status:\n")
                              TEXT(" Engine: %s\n") TEXT(" Direction: %s\n")
                                  TEXT(" Wheel Slip: %s\n")
                                      TEXT(" Stopping Distance: %.0f m\n")
                                          TEXT(" Distance Traveled: %.0f m\n")
                                              TEXT(" Passengers: %d"),
      PhysicsComponent->GetVelocityKmh(), PhysicsComponent->GetVelocityMs(),
      PhysicsComponent->PhysicsState.Acceleration,
      PhysicsComponent->PhysicsState.TotalMass,
      PhysicsComponent->PhysicsState.AppliedTractiveForce,
      PhysicsComponent->PhysicsState.AppliedBrakingForce,
      PhysicsComponent->PhysicsState.TotalResistance,
      PhysicsComponent->PhysicsState.RollingResistance,
      PhysicsComponent->PhysicsState.AirDragResistance,
      PhysicsComponent->PhysicsState.GradeResistance, SmoothedGrade,
      PhysicsComponent->PhysicsState.CurveResistance, SmoothedCurvature,
      SmoothedGrade, SmoothedCurvature,
      bEngineRunning ? TEXT("ON") : TEXT("OFF"),
      ReverseMultiplier > 0 ? TEXT("Forward") : TEXT("Reverse"),
      PhysicsComponent->PhysicsState.bIsWheelSlipping ? TEXT("YES")
                                                      : TEXT("NO"),
      PhysicsComponent->CalculateStoppingDistance(),
      PhysicsComponent->PhysicsState.DistanceTraveled, PassengersInside.Num());

  // Display on screen
  GEngine->AddOnScreenDebugMessage(
      -1, 0.0f,
      PhysicsComponent->PhysicsState.bIsWheelSlipping ? FColor::Red
                                                      : FColor::Green,
      DebugText);

  // Draw grade visualization
  FVector TrainLocation = GetActorLocation();
  FVector UpVector = GetActorUpVector();
  FVector ForwardVector = GetActorForwardVector();

  // Draw grade line
  float GradeVisualizationLength = 500.0f;
  FVector GradeEndPoint =
      TrainLocation +
      ForwardVector * GradeVisualizationLength *
          FMath::Cos(FMath::DegreesToRadians(SmoothedGrade)) +
      UpVector * GradeVisualizationLength *
          FMath::Sin(FMath::DegreesToRadians(SmoothedGrade));
  FColor GradeColor =
      SmoothedGrade > 0.0f
          ? FColor::Red
          : (SmoothedGrade < 0.0f ? FColor::Green : FColor::White);
  DrawDebugLine(GetWorld(), TrainLocation, GradeEndPoint, GradeColor, false,
                -1.0f, 0, 5.0f);
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
  ARailsPlayerCharacter *PlayerChar = Cast<ARailsPlayerCharacter>(Character);
  return PlayerChar ? PassengersInside.Contains(PlayerChar) : false;
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

// ========== NEW: Control Panel Functions ==========

void ARailsTrain::ToggleEngine() {
  bEngineRunning = !bEngineRunning;

  if (!bEngineRunning) {
    // Stop train when engine is turned off
    CurrentThrottle = 0.0f;
    TrainState = ETrainState::Stopped;
  }

  UE_LOG(LogTemp, Log, TEXT("Engine %s"),
         bEngineRunning ? TEXT("Started") : TEXT("Stopped"));
}

void ARailsTrain::ToggleReverse() {
  float CurrentSpeedKmh = GetCurrentSpeedKmh();

  // Check if train is moving
  if (CurrentSpeedKmh > 5.0f) // 5 km/h threshold
  {
    UE_LOG(
        LogTemp, Warning,
        TEXT("Cannot reverse while moving (%.1f km/h) - Stop the train first!"),
        CurrentSpeedKmh);

    // Auto-brake
    CurrentBrake = 1.0f;
    CurrentThrottle = 0.0f;

    return;
  }

  // Train is stopped - toggle direction
  ReverseMultiplier *= -1.0f;

  // Update physics component direction
  if (PhysicsComponent) {
    PhysicsComponent->SetDirection(ReverseMultiplier);
  }

  UE_LOG(LogTemp, Log, TEXT("Direction: %s"),
         ReverseMultiplier > 0 ? TEXT("Forward") : TEXT("Reverse"));
}



void ARailsTrain::IncreaseThrottle(float Amount) {
  if (!bEngineRunning) {
    UE_LOG(LogTemp, Warning, TEXT("Cannot apply throttle - engine is off"));
    return;
  }

  if (CurrentGear == 0) {
    UE_LOG(LogTemp, Warning, TEXT("Cannot apply throttle - in NEUTRAL gear"));
    return;
  }

  // Use gear-specific acceleration rate
  float AccelRate = GetCurrentGearAccelerationRate();
  CurrentThrottle =
      FMath::Clamp(CurrentThrottle + (Amount * AccelRate), 0.0f, 1.0f);

  TrainState = ETrainState::Accelerating;

  UE_LOG(LogTemp, Log, TEXT("Throttle: %.2f (Gear: %d, Accel Rate: %.2f)"),
         CurrentThrottle, CurrentGear, AccelRate);
}


void ARailsTrain::StartBraking() {
  bBrakeButtonHeld = true;
  CurrentBrake = 1.0f;

  // Reduce throttle gradually while braking
  CurrentThrottle = FMath::Max(0.0f, CurrentThrottle - 0.1f);

  TrainState = ETrainState::Decelerating;
  UE_LOG(LogTemp, Log, TEXT("Braking started"));
}

void ARailsTrain::StopBraking() {
  bBrakeButtonHeld = false;
  CurrentBrake = 0.0f;

  UE_LOG(LogTemp, Log, TEXT("Braking stopped"));
}



// ===== Passenger Management Implementation =====

bool ARailsTrain::IsPassengerInside(ARailsPlayerCharacter *Character) const {
  return PassengersInside.Contains(Character);
}

void ARailsTrain::OnPlayerEnterTrain(ARailsPlayerCharacter *Character) {
  if (!Character) {
    return;
  }

  if (PassengersInside.Contains(Character)) {
    UE_LOG(LogEpochRails, Warning,
           TEXT("Player %s already registered as passenger"),
           *Character->GetName());
    return;
  }

  // Add to passengers list
  PassengersInside.Add(Character);

  // Switch to passenger IMC (no jump)
  SwitchInputMappingContext(Character, true);

  UE_LOG(LogEpochRails, Log, TEXT("Player %s entered train %s - Jump disabled"),
         *Character->GetName(), *GetName());
}

void ARailsTrain::OnPlayerExitTrain(ARailsPlayerCharacter *Character) {
  if (!Character) {
    return;
  }

  if (!PassengersInside.Contains(Character)) {
    UE_LOG(LogEpochRails, Warning,
           TEXT("Player %s was not registered as passenger"),
           *Character->GetName());
    return;
  }

  // Remove from passengers list
  PassengersInside.Remove(Character);

  // Restore default IMC (with jump)
  SwitchInputMappingContext(Character, false);

  UE_LOG(LogEpochRails, Log, TEXT("Player %s exited train %s - Jump enabled"),
         *Character->GetName(), *GetName());
}

void ARailsTrain::SwitchInputMappingContext(ARailsPlayerCharacter *Character,
                                            bool bInsideTrain) {
  if (!Character) {
    return;
  }

  UEnhancedInputLocalPlayerSubsystem *Subsystem = GetInputSubsystem(Character);
  if (!Subsystem) {
    UE_LOG(LogEpochRails, Warning,
           TEXT("Could not get Enhanced Input subsystem for character %s"),
           *Character->GetName());
    return;
  }

  if (bInsideTrain) {
    // Remove default IMC
    if (DefaultInputMappingContext) {
      Subsystem->RemoveMappingContext(DefaultInputMappingContext);
      UE_LOG(LogEpochRails, Log, TEXT("Removed default IMC: %s"),
             *DefaultInputMappingContext->GetFName().ToString());
    }

    // Add passenger IMC (without jump)
    if (TrainPassengerInputMappingContext) {
      Subsystem->AddMappingContext(TrainPassengerInputMappingContext,
                                   IMCPriority);
      UE_LOG(LogEpochRails, Log, TEXT("Added passenger IMC (no jump): %s"),
             *TrainPassengerInputMappingContext->GetFName().ToString());
    } else {
      UE_LOG(LogEpochRails, Error,
             TEXT("TrainPassengerInputMappingContext is not set! Jump will not "
                  "be disabled."));
    }
  } else {
    // Remove passenger IMC
    if (TrainPassengerInputMappingContext) {
      Subsystem->RemoveMappingContext(TrainPassengerInputMappingContext);
      UE_LOG(LogEpochRails, Log, TEXT("Removed passenger IMC: %s"),
             *TrainPassengerInputMappingContext->GetFName().ToString());
    }

    // Restore default IMC (with jump)
    if (DefaultInputMappingContext) {
      Subsystem->AddMappingContext(DefaultInputMappingContext, IMCPriority);
      UE_LOG(LogEpochRails, Log, TEXT("Restored default IMC (with jump): %s"),
             *DefaultInputMappingContext->GetFName().ToString());
    } else {
      UE_LOG(LogEpochRails, Error,
             TEXT("DefaultInputMappingContext is not set! Player may have no "
                  "input."));
    }
  }
}

UEnhancedInputLocalPlayerSubsystem *
ARailsTrain::GetInputSubsystem(ARailsPlayerCharacter *Character) const {
  if (!Character) {
    return nullptr;
  }

  APlayerController *PC = Cast<APlayerController>(Character->GetController());
  if (!PC) {
    return nullptr;
  }

  ULocalPlayer *LocalPlayer = PC->GetLocalPlayer();
  if (!LocalPlayer) {
    return nullptr;
  }

  return LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
}

void ARailsTrain::OnTrainInteriorBeginOverlap(
    UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult &SweepResult) {
  ARailsPlayerCharacter *Player = Cast<ARailsPlayerCharacter>(OtherActor);
  if (Player) {
    OnPlayerEnterTrain(Player);
  }
}

void ARailsTrain::OnTrainInteriorEndOverlap(
    UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex) {
  ARailsPlayerCharacter *Player = Cast<ARailsPlayerCharacter>(OtherActor);
  if (Player) {
    OnPlayerExitTrain(Player);
  }
}

// ========== UI Functions ==========

void ARailsTrain::InitializeSpeedometer() {
  if (!SpeedometerWidgetComponent) {
    UE_LOG(LogTemp, Warning,
           TEXT("ARailsTrain::InitializeSpeedometer - "
                "SpeedometerWidgetComponent is null!"));
    return;
  }

  // Set widget class if provided
  if (SpeedometerWidgetClass) {
    SpeedometerWidgetComponent->SetWidgetClass(SpeedometerWidgetClass);
  } else {
    UE_LOG(LogTemp, Warning,
           TEXT("ARailsTrain::InitializeSpeedometer - SpeedometerWidgetClass "
                "not set!"));
    return;
  }

  // Get widget instance
  if (UUserWidget *UserWidget =
          SpeedometerWidgetComponent->GetUserWidgetObject()) {
    CachedSpeedometerWidget = Cast<UTrainSpeedometerWidget>(UserWidget);
    if (CachedSpeedometerWidget) {
      // Initialize with max speed
      CachedSpeedometerWidget->SetMaxSpeed(SpeedometerMaxSpeed);
      CachedSpeedometerWidget->SetSpeedImmediate(0.0f);
      UE_LOG(LogTemp, Log,
             TEXT("ARailsTrain::InitializeSpeedometer - Speedometer "
                  "initialized successfully"));
    } else {
      UE_LOG(LogTemp, Warning,
             TEXT("ARailsTrain::InitializeSpeedometer - Failed to cast to "
                  "UTrainSpeedometerWidget!"));
    }
  }
}

void ARailsTrain::InitializeControlPanel() {
  if (!ControlPanelWidgetComponent || !ControlPanelWidgetClass) {
    UE_LOG(LogTemp, Warning,
           TEXT("Control Panel Widget Component or Class not set!"));
    return;
  }

  // Create widget instance
  ControlPanelWidgetComponent->SetWidgetClass(ControlPanelWidgetClass);

  // Configure for interaction
  ControlPanelWidgetComponent->SetCollisionEnabled(
      ECollisionEnabled::QueryOnly);
  ControlPanelWidgetComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
  ControlPanelWidgetComponent->SetCollisionResponseToChannel(ECC_Visibility,
                                                             ECR_Block);

  // REMOVED: SetReceiveHardwareInput - not available in UE 5.7
  // Widget interaction is handled automatically by PlayerController settings

  UE_LOG(LogTemp, Log, TEXT("Control panel initialized successfully"));
}


void ARailsTrain::UpdateSpeedometerDisplay() {
  if (CachedSpeedometerWidget) {
    // Get current speed in km/h
    float SpeedKmh = GetCurrentSpeedKmh();

    // Update speedometer
    CachedSpeedometerWidget->UpdateSpeed(SpeedKmh, SpeedometerMaxSpeed);
  }
}

void ARailsTrain::SetSpeedometerVisible(bool bVisible) {
  bShowSpeedometer = bVisible;
  if (SpeedometerWidgetComponent) {
    SpeedometerWidgetComponent->SetVisibility(bVisible);
  }
}

void ARailsTrain::SetSpeedometerMaxSpeed(float NewMaxSpeed) {
  SpeedometerMaxSpeed = FMath::Max(NewMaxSpeed, 10.0f);
  if (CachedSpeedometerWidget) {
    CachedSpeedometerWidget->SetMaxSpeed(SpeedometerMaxSpeed);
  }
}

// ========== Gear System Implementation ==========

bool ARailsTrain::CanShiftGear() const {
  return TimeSinceLastGearShift >= GearShiftDelay;
}

void ARailsTrain::ShiftGearUp() {
  if (!bEngineRunning) {
    UE_LOG(LogTemp, Warning, TEXT("Cannot shift gear - engine is off"));
    return;
  }

  if (!CanShiftGear()) {
    UE_LOG(LogTemp, Warning, TEXT("Cannot shift gear - wait %.1f seconds"),
           GearShiftDelay - TimeSinceLastGearShift);
    return;
  }

  if (CurrentGear < MaxGears) {
    CurrentGear++;
    TimeSinceLastGearShift = 0.0f;

    // Update physics component gear
    if (PhysicsComponent) {
      PhysicsComponent->SetGear(CurrentGear);
    }

    UE_LOG(LogTemp, Log, TEXT("Gear shifted UP to: %d"), CurrentGear);
  } else {
    UE_LOG(LogTemp, Warning, TEXT("Already in highest gear: %d"), CurrentGear);
  }
}

void ARailsTrain::ShiftGearDown() {
  if (!CanShiftGear()) {
    UE_LOG(LogTemp, Warning, TEXT("Cannot shift gear - wait %.1f seconds"),
           GearShiftDelay - TimeSinceLastGearShift);
    return;
  }

  if (CurrentGear > 0) {
    CurrentGear--;
    TimeSinceLastGearShift = 0.0f;

    // Update physics component gear
    if (PhysicsComponent) {
      PhysicsComponent->SetGear(CurrentGear);
    }

    if (CurrentGear == 0) {
      // Neutral gear - reset throttle
      CurrentThrottle = 0.0f;
      UE_LOG(LogTemp, Log, TEXT("Gear shifted to NEUTRAL"));
    } else {
      UE_LOG(LogTemp, Log, TEXT("Gear shifted DOWN to: %d"), CurrentGear);
    }
  } else {
    UE_LOG(LogTemp, Warning, TEXT("Already in neutral gear"));
  }
}


float ARailsTrain::GetCurrentGearSpeedMultiplier() const {
  if (CurrentGear == 0 || CurrentGear > GearSpeedMultipliers.Num()) {
    return 0.0f;
  }
  return GearSpeedMultipliers[CurrentGear - 1];
}

float ARailsTrain::GetCurrentGearAccelerationRate() const {
  if (CurrentGear == 0 || CurrentGear > GearAccelerationRates.Num()) {
    return 0.1f; // Default
  }
  return GearAccelerationRates[CurrentGear - 1];
}
