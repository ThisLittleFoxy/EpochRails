// RailsTrain.cpp

#include "RailsTrain.h"

#include "UObject/ConstructorHelpers.h"
#include "Character/RailsPlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "EpochRails.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "Kismet/KismetMathLibrary.h"
#include "RailsSplinePath.h"
#include "TrainSpeedometerWidget.h"
#include "Wagon.h"

ARailsTrain::ARailsTrain() {
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PrePhysics;

  // Load default widget classes
  static ConstructorHelpers::FClassFinder<UTrainSpeedometerWidget> SpeedometerWidgetBP(
      TEXT("/Game/Train/BP_Train/UI_Train/WBP_TrainSpeedometer"));
  if (SpeedometerWidgetBP.Succeeded()) {
    SpeedometerWidgetClass = SpeedometerWidgetBP.Class;
  }

  static ConstructorHelpers::FClassFinder<UUserWidget> ControlPanelWidgetBP(
      TEXT("/Game/Train/BP_Train/UI_Train/WBP_TrainControlPanel"));
  if (ControlPanelWidgetBP.Succeeded()) {
    ControlPanelWidgetClass = ControlPanelWidgetBP.Class;
  }

  // Create root component
  TrainRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TrainRoot"));
  RootComponent = TrainRoot;

  // Create FloatingPawnMovement for smooth interpolated movement
  MovementComponent =
      CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
  MovementComponent->UpdatedComponent = TrainRoot;
  MovementComponent->MaxSpeed = 5000.0f;          // Max speed in cm/s
  MovementComponent->Acceleration = 2000.0f;      // Acceleration rate
  MovementComponent->Deceleration = 4000.0f;      // Deceleration rate
  MovementComponent->TurningBoost = 0.0f;         // No turning boost for rails

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

  // Rear attachment point for the first wagon
  RearAttachmentPoint =
      CreateDefaultSubobject<USceneComponent>(TEXT("RearAttachmentPoint"));
  RearAttachmentPoint->SetupAttachment(TrainRoot);
  RearAttachmentPoint->SetRelativeLocation(FVector(-600.0f, 0.0f, 0.0f));
}

void ARailsTrain::BeginPlay() {
  Super::BeginPlay();

  // Cache spline component
  if (SplinePathRef) {
    CachedSplineComponent = SplinePathRef->GetSpline();
    if (!CachedSplineComponent) {
      UE_LOG(LogEpochRails, Error,
             TEXT("Train %s: SplinePathRef is set but GetSpline() returned "
                  "nullptr!"),
             *GetName());
    }
  } else {
    UE_LOG(LogEpochRails, Warning,
           TEXT("Train %s: No SplinePathRef assigned - train will not move!"),
           *GetName());
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

  if (PhysicsComponent) {
    AddTickPrerequisiteComponent(PhysicsComponent);
  }
  // Initialize UI widgets
  InitializeSpeedometer();
  InitializeControlPanel();
}

void ARailsTrain::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  // Clamp delta time to prevent large jumps
  const float SafeDeltaTime = FMath::Min(DeltaTime, 0.033f); // Max ~30 FPS step

  // Update gear shift timer
  TimeSinceLastGearShift += SafeDeltaTime;

  // Apply continuous braking if button held
  if (bBrakeButtonHeld) {
    CurrentThrottle = FMath::Max(0.0f, CurrentThrottle - (0.5f * SafeDeltaTime));
    CurrentBrake = 1.0f;
  }

  // Update movement (only once!)
  UpdateTrainMovement(SafeDeltaTime);

  // Draw debug info if enabled
  if (bShowPhysicsDebug) {
    DrawPhysicsDebug();
  }

  // Update wagons: only update first wagon, chain propagates automatically
  if (AttachedWagons.Num() > 0 && CachedSplineComponent) {
    AttachedWagons[0]->SetTargetDistance(CurrentDistance);
  }

  // Update speedometer display
  UpdateSpeedometerDisplay();
}

void ARailsTrain::UpdateTrainMovement(float DeltaTime) {
  UpdatePhysicsMovement(DeltaTime);
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

  // Get velocity from physics (in m/s, signed)
  float VelocityMs = PhysicsComponent->PhysicsState.Velocity;

  // Convert to cm/s for Unreal units (absolute for display)
  CurrentSpeed = FMath::Abs(VelocityMs) * 100.0f;

  // Update distance along spline (velocity already includes direction)
  const float DistanceDelta = VelocityMs * 100.0f * DeltaTime;
  CurrentDistance += DistanceDelta;

  // Handle looping and bounds
  const float SplineLength = CachedSplineComponent->GetSplineLength();

  if (bLoopPath) {
    while (CurrentDistance >= SplineLength) {
      CurrentDistance -= SplineLength;
    }
    while (CurrentDistance < 0.0f) {
      CurrentDistance += SplineLength;
    }
  } else {
    const float OldDistance = CurrentDistance;
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
  const float Acceleration = PhysicsComponent->PhysicsState.Acceleration;

  if (FMath::Abs(Acceleration) < 0.01f) {
    TrainState =
        (CurrentSpeed > 1.0f) ? ETrainState::Moving : ETrainState::Stopped;
  } else if (Acceleration > 0.01f) {
    TrainState = ETrainState::Accelerating;
  } else {
    TrainState = ETrainState::Decelerating;
  }
}

void ARailsTrain::MoveToDistance(float Distance) {
  if (!CachedSplineComponent) {
    return;
  }

  FVector TargetLocation =
      CachedSplineComponent->GetLocationAtDistanceAlongSpline(
          Distance, ESplineCoordinateSpace::World);
  FRotator TargetRotation =
      CachedSplineComponent->GetRotationAtDistanceAlongSpline(
          Distance, ESplineCoordinateSpace::World);

  // Move directly to target - spline provides smooth path, no collision needed
  SetActorLocationAndRotation(TargetLocation, TargetRotation);
}

float ARailsTrain::CalculateTrackGrade() {
  if (!CachedSplineComponent) {
    return 0.0f;
  }

  FVector CurrentTangent =
      CachedSplineComponent->GetTangentAtDistanceAlongSpline(
          CurrentDistance, ESplineCoordinateSpace::World);

  float AheadDistance = CurrentDistance + PhysicsSampleDistance;
  const float SplineLength = CachedSplineComponent->GetSplineLength();

  if (AheadDistance > SplineLength && bLoopPath) {
    AheadDistance = FMath::Fmod(AheadDistance, SplineLength);
  }

  FVector AheadTangent = CachedSplineComponent->GetTangentAtDistanceAlongSpline(
      AheadDistance, ESplineCoordinateSpace::World);

  CurrentTangent.Normalize();
  AheadTangent.Normalize();

  FVector AverageTangent = (CurrentTangent + AheadTangent) * 0.5f;
  AverageTangent.Normalize();

  return FMath::RadiansToDegrees(FMath::Asin(AverageTangent.Z));
}

float ARailsTrain::CalculateTrackCurvature() {
  if (!CachedSplineComponent) {
    return 0.0f;
  }

  FVector CurrentDir = CachedSplineComponent->GetDirectionAtDistanceAlongSpline(
      CurrentDistance, ESplineCoordinateSpace::World);

  float AheadDistance = CurrentDistance + PhysicsSampleDistance;
  const float SplineLength = CachedSplineComponent->GetSplineLength();

  if (AheadDistance > SplineLength && bLoopPath) {
    AheadDistance = FMath::Fmod(AheadDistance, SplineLength);
  }

  FVector AheadDir = CachedSplineComponent->GetDirectionAtDistanceAlongSpline(
      AheadDistance, ESplineCoordinateSpace::World);

  float DotProduct = FVector::DotProduct(CurrentDir, AheadDir);
  DotProduct = FMath::Clamp(DotProduct, -1.0f, 1.0f);

  float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

  // Normalize to 0-1 range (0 = straight, 1 = very tight curve)
  return FMath::Clamp(AngleDegrees / 90.0f, 0.0f, 1.0f);
}

void ARailsTrain::UpdatePhysicsParameters(float DeltaTime) {
  if (!PhysicsComponent) {
    return;
  }

  const float TargetGrade = CalculateTrackGrade();
  const float TargetCurvature = CalculateTrackCurvature();

  SmoothedGrade = FMath::FInterpTo(SmoothedGrade, TargetGrade, DeltaTime,
                                   GradeSmoothingSpeed);
  SmoothedCurvature = FMath::FInterpTo(SmoothedCurvature, TargetCurvature,
                                       DeltaTime, GradeSmoothingSpeed);

  PhysicsComponent->SetTrackGrade(SmoothedGrade);
  PhysicsComponent->SetTrackCurvature(SmoothedCurvature);
}

void ARailsTrain::DrawPhysicsDebug() {
  if (!PhysicsComponent) {
    return;
  }

  FString DebugText = FString::Printf(
      TEXT("=== TRAIN PHYSICS DEBUG ===\n") TEXT(
          "Speed: %.1f km/h (%.1f m/s)\n") TEXT("Acceleration: %.2f m/s^2\n")
          TEXT("Mass: %.0f kg\n") TEXT("\n") TEXT("Forces:\n") TEXT(
              " Tractive: %.0f N\n") TEXT(" Braking: %.0f N\n")
              TEXT(" Total Resistance: %.0f N\n") TEXT("\n") TEXT(
                  "Resistance Breakdown:\n") TEXT(" Rolling: %.0f N\n")
                  TEXT(" Air Drag: %.0f N\n")
                      TEXT(" Grade: %.0f N (%.1f deg)\n") TEXT(
                          " Curve: %.0f N (%.2f)\n") TEXT("\n") TEXT("Track:\n")
                          TEXT(" Grade: %.2f deg\n") TEXT(" Curvature: %.2f\n")
                              TEXT("\n") TEXT("Status:\n") TEXT(" Engine: %s\n")
                                  TEXT(" Direction: %s\n")
                                      TEXT(" Wheel Slip: %s\n") TEXT(
                                          " Stopping Distance: %.0f m\n")
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
      PhysicsComponent->GetDirection() > 0 ? TEXT("Forward") : TEXT("Reverse"),
      PhysicsComponent->PhysicsState.bIsWheelSlipping ? TEXT("YES")
                                                      : TEXT("NO"),
      PhysicsComponent->CalculateStoppingDistance(),
      PhysicsComponent->PhysicsState.DistanceTraveled, PassengersInside.Num());

  GEngine->AddOnScreenDebugMessage(
      -1, 0.0f,
      PhysicsComponent->PhysicsState.bIsWheelSlipping ? FColor::Red
                                                      : FColor::Green,
      DebugText);

  // Draw grade visualization
  const FVector TrainLocation = GetActorLocation();
  const FVector UpVector = GetActorUpVector();
  const FVector ForwardVector = GetActorForwardVector();

  const float GradeVisualizationLength = 500.0f;

  const FVector GradeEndPoint =
      TrainLocation +
      ForwardVector * GradeVisualizationLength *
          FMath::Cos(FMath::DegreesToRadians(SmoothedGrade)) +
      UpVector * GradeVisualizationLength *
          FMath::Sin(FMath::DegreesToRadians(SmoothedGrade));

  const FColor GradeColor =
      SmoothedGrade > 0.0f
          ? FColor::Red
          : (SmoothedGrade < 0.0f ? FColor::Green : FColor::White);

  DrawDebugLine(GetWorld(), TrainLocation, GradeEndPoint, GradeColor, false,
                -1.0f, 0, 5.0f);
}

void ARailsTrain::StartTrain() {
  if (PhysicsComponent) {
    CurrentThrottle = 0.5f;
    CurrentBrake = 0.0f;
  }
  TrainState = ETrainState::Accelerating;
}

void ARailsTrain::StopTrain() {
  if (PhysicsComponent) {
    CurrentThrottle = 0.0f;
    CurrentBrake = 1.0f;
  }
  TrainState = ETrainState::Stopped;
}

float ARailsTrain::GetCurrentSpeedKmh() const {
  return PhysicsComponent ? PhysicsComponent->GetVelocityKmh() : 0.0f;
}

float ARailsTrain::GetReverseMultiplier() const {
  return PhysicsComponent ? PhysicsComponent->GetDirection() : 1.0f;
}

bool ARailsTrain::IsCharacterOnTrain(ACharacter *Character) const {
  ARailsPlayerCharacter *PlayerChar = Cast<ARailsPlayerCharacter>(Character);
  if (!PlayerChar) {
    return false;
  }

  for (const TWeakObjectPtr<ARailsPlayerCharacter> &WeakPassenger :
       PassengersInside) {
    if (WeakPassenger.IsValid() && WeakPassenger.Get() == PlayerChar) {
      return true;
    }
  }
  return false;
}

TArray<ARailsPlayerCharacter *> ARailsTrain::GetPassengers() const {
  TArray<ARailsPlayerCharacter *> ValidPassengers;
  for (const TWeakObjectPtr<ARailsPlayerCharacter> &WeakPassenger :
       PassengersInside) {
    if (WeakPassenger.IsValid()) {
      ValidPassengers.Add(WeakPassenger.Get());
    }
  }
  return ValidPassengers;
}

void ARailsTrain::ApplyThrottle(float ThrottleValue) {
  CurrentThrottle = FMath::Clamp(ThrottleValue, -1.0f, 1.0f);

  if (PhysicsComponent) {
    PhysicsComponent->SetThrottle(FMath::Max(0.0f, CurrentThrottle));
  }
}

void ARailsTrain::ApplyBrake(float BrakeValue) {
  CurrentBrake = FMath::Clamp(BrakeValue, 0.0f, 1.0f);

  if (PhysicsComponent) {
    PhysicsComponent->SetBrake(CurrentBrake);
  }
}

void ARailsTrain::EmergencyBrake() {
  CurrentThrottle = 0.0f;
  CurrentBrake = 1.0f;

  if (PhysicsComponent) {
    PhysicsComponent->EmergencyBrake();
  }

  TrainState = ETrainState::Decelerating;
}

float ARailsTrain::GetStoppingDistance() const {
  return PhysicsComponent ? PhysicsComponent->CalculateStoppingDistance()
                          : 0.0f;
}

void ARailsTrain::AddWagons(int32 Count) {
  if (!WagonClass) {
    UE_LOG(LogEpochRails, Error,
           TEXT("Train %s: WagonClass not set!"), *GetName());
    return;
  }

  if (!CachedSplineComponent) {
    UE_LOG(LogEpochRails, Error,
           TEXT("Train %s: No spline component available - cannot add wagons!"),
           *GetName());
    return;
  }

  for (int32 i = 0; i < Count; i++) {
    AActor *LastVehicle = this;
    if (AttachedWagons.Num() > 0) {
      LastVehicle = AttachedWagons.Last();
    }

    FVector SpawnLocation = FVector::ZeroVector;
    FRotator SpawnRotation = FRotator::ZeroRotator;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWagon *NewWagon = GetWorld()->SpawnActor<AWagon>(
        WagonClass, SpawnLocation, SpawnRotation, SpawnParams);
    if (!NewWagon) {
      UE_LOG(LogTemp, Error, TEXT("Failed to spawn wagon!"));
      continue;
    }

    NewWagon->Initialize(CachedSplineComponent, LastVehicle);

    if (AWagon *LastWagon = Cast<AWagon>(LastVehicle)) {
      LastWagon->SetNextWagon(NewWagon);
    }

    AttachedWagons.Add(NewWagon);

    if (PhysicsComponent) {
      PhysicsComponent->AddWagons(1);
    }

    UE_LOG(LogTemp, Log, TEXT("Spawned wagon %d following spline"), i + 1);
  }

  UE_LOG(LogTemp, Log, TEXT("Added %d wagon(s). Total: %d"), Count,
         AttachedWagons.Num());
}

void ARailsTrain::RemoveWagons(int32 Count) {
  if (AttachedWagons.Num() == 0) {
    UE_LOG(LogTemp, Warning, TEXT("No wagons to remove!"));
    return;
  }

  const int32 RemoveCount = FMath::Min(Count, AttachedWagons.Num());
  for (int32 i = 0; i < RemoveCount; i++) {
    AWagon *LastWagon = AttachedWagons.Last();
    if (!LastWagon) {
      continue;
    }

    if (PhysicsComponent) {
      PhysicsComponent->RemoveWagons(1);
    }

    LastWagon->DetachFromChain();
    LastWagon->Destroy();

    AttachedWagons.RemoveAt(AttachedWagons.Num() - 1);
  }

  UE_LOG(LogTemp, Log, TEXT("Removed %d wagon(s). Remaining: %d"), RemoveCount,
         AttachedWagons.Num());
}

// ===== Control Panel / engine / direction =====

void ARailsTrain::ToggleEngine() {
  bEngineRunning = !bEngineRunning;

  if (!bEngineRunning) {
    CurrentThrottle = 0.0f;
    TrainState = ETrainState::Stopped;
  }

  UE_LOG(LogTemp, Log, TEXT("Engine %s"),
         bEngineRunning ? TEXT("Started") : TEXT("Stopped"));
}

void ARailsTrain::ToggleReverse() {
  const float CurrentSpeedKmh = GetCurrentSpeedKmh();

  if (CurrentSpeedKmh > 5.0f) {
    UE_LOG(
        LogTemp, Warning,
        TEXT("Cannot reverse while moving (%.1f km/h) - Stop the train first!"),
        CurrentSpeedKmh);
    CurrentBrake = 1.0f;
    CurrentThrottle = 0.0f;
    return;
  }

  if (PhysicsComponent) {
    float CurrentDirection = PhysicsComponent->GetDirection();
    float NewDirection = CurrentDirection * -1.0f;
    PhysicsComponent->SetDirection(NewDirection);

    UE_LOG(LogTemp, Log, TEXT("Direction: %s"),
           NewDirection > 0 ? TEXT("Forward") : TEXT("Reverse"));
  }
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

  const float AccelRate = GetCurrentGearAccelerationRate();
  CurrentThrottle =
      FMath::Clamp(CurrentThrottle + (Amount * AccelRate), 0.0f, 1.0f);
  TrainState = ETrainState::Accelerating;

  UE_LOG(LogTemp, Log, TEXT("Throttle: %.2f (Gear: %d, Accel Rate: %.2f)"),
         CurrentThrottle, CurrentGear, AccelRate);
}

void ARailsTrain::StartBraking() {
  bBrakeButtonHeld = true;
  CurrentBrake = 1.0f;
  CurrentThrottle = FMath::Max(0.0f, CurrentThrottle - 0.1f);
  TrainState = ETrainState::Decelerating;

  UE_LOG(LogTemp, Log, TEXT("Braking started"));
}

void ARailsTrain::StopBraking() {
  bBrakeButtonHeld = false;
  CurrentBrake = 0.0f;
  UE_LOG(LogTemp, Log, TEXT("Braking stopped"));
}

// ===== Passenger management =====

bool ARailsTrain::IsPassengerInside(ARailsPlayerCharacter *Character) const {
  if (!Character) {
    return false;
  }

  for (const TWeakObjectPtr<ARailsPlayerCharacter> &WeakPassenger :
       PassengersInside) {
    if (WeakPassenger.IsValid() && WeakPassenger.Get() == Character) {
      return true;
    }
  }
  return false;
}

void ARailsTrain::OnPlayerEnterTrain(ARailsPlayerCharacter *Character) {
  if (!Character) {
    return;
  }

  if (IsPassengerInside(Character)) {
    UE_LOG(LogEpochRails, Warning,
           TEXT("Player %s already registered as passenger"),
           *Character->GetName());
    return;
  }

  PassengersInside.Add(Character);
  SwitchInputMappingContext(Character, true);

  UE_LOG(LogEpochRails, Log, TEXT("Player %s entered train %s - Jump disabled"),
         *Character->GetName(), *GetName());
}

void ARailsTrain::OnPlayerExitTrain(ARailsPlayerCharacter *Character) {
  if (!Character) {
    return;
  }

  if (!IsPassengerInside(Character)) {
    UE_LOG(LogEpochRails, Warning,
           TEXT("Player %s was not registered as passenger"),
           *Character->GetName());
    return;
  }

  // Remove using RemoveAll with predicate to handle TWeakObjectPtr
  PassengersInside.RemoveAll([Character](const TWeakObjectPtr<ARailsPlayerCharacter> &WeakPassenger) {
    return WeakPassenger.IsValid() && WeakPassenger.Get() == Character;
  });

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
    if (DefaultInputMappingContext) {
      Subsystem->RemoveMappingContext(DefaultInputMappingContext);
      UE_LOG(LogEpochRails, Log, TEXT("Removed default IMC: %s"),
             *DefaultInputMappingContext->GetFName().ToString());
    }

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
    if (TrainPassengerInputMappingContext) {
      Subsystem->RemoveMappingContext(TrainPassengerInputMappingContext);
      UE_LOG(LogEpochRails, Log, TEXT("Removed passenger IMC: %s"),
             *TrainPassengerInputMappingContext->GetFName().ToString());
    }

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

// ===== UI =====

void ARailsTrain::InitializeSpeedometer() {
  if (!SpeedometerWidgetComponent) {
    UE_LOG(LogTemp, Warning,
           TEXT("ARailsTrain::InitializeSpeedometer - "
                "SpeedometerWidgetComponent is null!"));
    return;
  }

  if (SpeedometerWidgetClass) {
    SpeedometerWidgetComponent->SetWidgetClass(SpeedometerWidgetClass);
  } else {
    UE_LOG(LogTemp, Warning,
           TEXT("ARailsTrain::InitializeSpeedometer - SpeedometerWidgetClass "
                "not set!"));
    return;
  }

  if (UUserWidget *UserWidget =
          SpeedometerWidgetComponent->GetUserWidgetObject()) {
    CachedSpeedometerWidget = Cast<UTrainSpeedometerWidget>(UserWidget);
    if (CachedSpeedometerWidget) {
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

  ControlPanelWidgetComponent->SetWidgetClass(ControlPanelWidgetClass);

  ControlPanelWidgetComponent->SetCollisionEnabled(
      ECollisionEnabled::QueryOnly);
  ControlPanelWidgetComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
  ControlPanelWidgetComponent->SetCollisionResponseToChannel(ECC_Visibility,
                                                             ECR_Block);

  UE_LOG(LogTemp, Log, TEXT("Control panel initialized successfully"));
}

void ARailsTrain::UpdateSpeedometerDisplay() {
  if (CachedSpeedometerWidget) {
    const float SpeedKmh = GetCurrentSpeedKmh();
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

// ===== Gear system =====

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

    if (PhysicsComponent) {
      PhysicsComponent->SetGear(CurrentGear);
    }

    if (CurrentGear == 0) {
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
    return 0.1f;
  }
  return GearAccelerationRates[CurrentGear - 1];
}
