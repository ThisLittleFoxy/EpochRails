// RailsTrain.cpp
// Implementation with simulated physics

#include "RailsTrain.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "RailsSplinePath.h"

ARailsTrain::ARailsTrain() {
  PrimaryActorTick.bCanEverTick = true;

  // Create components
  TrainRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TrainRoot"));
  RootComponent = TrainRoot;

  TrainBodyMesh =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrainBodyMesh"));
  TrainBodyMesh->SetupAttachment(TrainRoot);
  TrainBodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  TrainBodyMesh->SetCollisionObjectType(ECC_WorldDynamic);

  PlatformMesh =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
  PlatformMesh->SetupAttachment(TrainBodyMesh);
  PlatformMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  PlatformMesh->SetCollisionObjectType(ECC_WorldDynamic);
  PlatformMesh->SetSimulatePhysics(false);

  BoardingZone = CreateDefaultSubobject<UBoxComponent>(TEXT("BoardingZone"));
  BoardingZone->SetupAttachment(PlatformMesh);
  BoardingZone->SetBoxExtent(FVector(200.0f, 150.0f, 100.0f));
  BoardingZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  BoardingZone->SetCollisionResponseToAllChannels(ECR_Ignore);
  BoardingZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}


void ARailsTrain::BeginPlay() {
  Super::BeginPlay();

  if (BoardingZone) {
    BoardingZone->OnComponentBeginOverlap.AddDynamic(
        this, &ARailsTrain::OnBoardingZoneBeginOverlap);
    BoardingZone->OnComponentEndOverlap.AddDynamic(
        this, &ARailsTrain::OnBoardingZoneEndOverlap);
  }

  if (bAutoStart) {
    StartTrain();
  }
}

void ARailsTrain::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  if (SplinePathRef) {
    UpdateTrainMovement(DeltaTime);
  }
}

void ARailsTrain::UpdateTrainMovement(float DeltaTime) {
  if (!SplinePathRef)
    return;

  USplineComponent *Spline = SplinePathRef->GetSpline();
  if (!Spline)
    return;

  float SplineLength = Spline->GetSplineLength();
  if (SplineLength <= 0.0f)
    return;

  // Update physics simulation
  UpdateSimulatedPhysics(DeltaTime);

  // Update position
  CurrentDistance += CurrentSpeed * DeltaTime;

  // Handle looping
  if (CurrentDistance >= SplineLength) {
    if (bLoopPath) {
      CurrentDistance = FMath::Fmod(CurrentDistance, SplineLength);
    } else {
      CurrentDistance = SplineLength;
      CurrentSpeed = 0.0f;
      TrainState = ETrainState::Stopped;
    }
  } else if (CurrentDistance < 0.0f) {
    if (bLoopPath) {
      CurrentDistance = SplineLength + CurrentDistance;
    } else {
      CurrentDistance = 0.0f;
      CurrentSpeed = 0.0f;
      TrainState = ETrainState::Stopped;
    }
  }

  MoveToDistance(CurrentDistance);
}

void ARailsTrain::UpdateSimulatedPhysics(float DeltaTime) {
  float TargetMaxSpeed = 0.0f;
  float GearDirection = 0.0f;

  switch (CurrentGear) {
  case ETrainGear::Forward:
    TargetMaxSpeed = MaxForwardSpeed;
    GearDirection = 1.0f;
    break;
  case ETrainGear::Reverse:
    TargetMaxSpeed = MaxReverseSpeed;
    GearDirection = -1.0f;
    break;
  default:
    break;
  }

  // 1. ENGINE FORCE
  float EngineForce = 0.0f;
  if (CurrentGear != ETrainGear::Neutral && ThrottlePosition > 0.0f) {
    float SpeedFactor = 1.0f - FMath::Abs(CurrentSpeed / TargetMaxSpeed);
    SpeedFactor = FMath::Max(SpeedFactor, 0.1f);
    EngineForce =
        ThrottlePosition * EnginePowerCoefficient * SpeedFactor * GearDirection;
  }

  // 2. BRAKE FORCE
  float BrakeForce = 0.0f;
  if (BrakePosition > 0.0f) {
    float BrakeDirection = -FMath::Sign(CurrentSpeed);
    if (FMath::Abs(CurrentSpeed) < 10.0f) {
      BrakeDirection = -GearDirection;
    }
    BrakeForce = BrakePosition * BrakePowerCoefficient * BrakeDirection;
  }

  // 3. ROLLING FRICTION
  float RollingFriction = 0.0f;
  if (FMath::Abs(CurrentSpeed) > 1.0f) {
    RollingFriction = -RollingFrictionCoefficient * FMath::Sign(CurrentSpeed);
  }

  // 4. AIR DRAG
  float AirDrag = 0.0f;
  if (FMath::Abs(CurrentSpeed) > 10.0f) {
    float SpeedSquared = CurrentSpeed * CurrentSpeed;
    AirDrag = -AirDragCoefficient * SpeedSquared * FMath::Sign(CurrentSpeed) /
              10000.0f;
  }

  // 5. SLOPE EFFECT
  float SlopeForce = 0.0f;
  if (GravityMultiplier > 0.0f) {
    CurrentSlopeAngle = CalculateSlopeEffect();
    SlopeForce = -FMath::Sin(FMath::DegreesToRadians(CurrentSlopeAngle)) *
                 GravityMultiplier * 0.5f;
  }

  // TOTAL FORCE
  TotalAppliedForce =
      EngineForce + BrakeForce + RollingFriction + AirDrag + SlopeForce;

  // APPLY INERTIA
  float InertiaFactor = GetInertiaFactor();
  SimulatedAcceleration = TotalAppliedForce / InertiaFactor;

  // UPDATE SPEED
  CurrentSpeed += SimulatedAcceleration * 100.0f * DeltaTime;

  // CLAMP SPEED
  if (CurrentGear == ETrainGear::Forward) {
    CurrentSpeed = FMath::Clamp(CurrentSpeed, -100.0f, TargetMaxSpeed);
  } else if (CurrentGear == ETrainGear::Reverse) {
    CurrentSpeed = FMath::Clamp(CurrentSpeed, -TargetMaxSpeed, 100.0f);
  }

  // UPDATE STATE
  if (FMath::Abs(CurrentSpeed) < MinSpeedThreshold) {
    CurrentSpeed = 0.0f;
    TrainState = ETrainState::Stopped;
  } else if (SimulatedAcceleration > 0.1f) {
    TrainState = ETrainState::Accelerating;
  } else if (SimulatedAcceleration < -0.1f || BrakePosition > 0.0f) {
    TrainState = ETrainState::Decelerating;
  } else {
    TrainState = ETrainState::Moving;
  }
}

void ARailsTrain::MoveToDistance(float Distance) {
  if (!SplinePathRef)
    return;

  USplineComponent *Spline = SplinePathRef->GetSpline();
  if (!Spline)
    return;

  FVector NewLocation = Spline->GetLocationAtDistanceAlongSpline(
      Distance, ESplineCoordinateSpace::World);
  FRotator NewRotation = Spline->GetRotationAtDistanceAlongSpline(
      Distance, ESplineCoordinateSpace::World);

  SetActorLocationAndRotation(NewLocation, NewRotation);
}

float ARailsTrain::CalculateSlopeEffect() const {
  if (!SplinePathRef)
    return 0.0f;

  USplineComponent *Spline = SplinePathRef->GetSpline();
  if (!Spline)
    return 0.0f;

  FRotator Rotation = Spline->GetRotationAtDistanceAlongSpline(
      CurrentDistance, ESplineCoordinateSpace::World);

  return Rotation.Pitch;
}

float ARailsTrain::GetInertiaFactor() const {
  if (!bUseMassInertia)
    return 1.0f;

  float NormalizedMass = SimulatedMass / 50000.0f;
  return FMath::Max(NormalizedMass, 0.1f);
}

// COLLISION EVENTS

void ARailsTrain::OnBoardingZoneBeginOverlap(
    UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult &SweepResult) {

  ACharacter *Character = Cast<ACharacter>(OtherActor);
  if (Character && !PassengersOnBoard.Contains(Character)) {
    PassengersOnBoard.Add(Character);
    UE_LOG(LogTemp, Log, TEXT("Character %s boarded train"),
           *Character->GetName());
  }
}

void ARailsTrain::OnBoardingZoneEndOverlap(
    UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex) {

  ACharacter *Character = Cast<ACharacter>(OtherActor);
  if (Character && PassengersOnBoard.Contains(Character)) {
    PassengersOnBoard.Remove(Character);
    UE_LOG(LogTemp, Log, TEXT("Character %s left train"),
           *Character->GetName());
  }
}

// CONTROL FUNCTIONS

void ARailsTrain::SetGear(ETrainGear NewGear) {
  if (FMath::Abs(CurrentSpeed) < 50.0f) {
    CurrentGear = NewGear;
    UE_LOG(LogTemp, Log, TEXT("Gear: %s"), *GetGearString());
  } else {
    UE_LOG(LogTemp, Warning, TEXT("Cannot change gear at %.1f km/h"),
           GetSpeedKmh());
  }
}

void ARailsTrain::SetThrottle(float Position) {
  ThrottlePosition = FMath::Clamp(Position, 0.0f, 1.0f);
}

void ARailsTrain::SetBrake(float Position) {
  BrakePosition = FMath::Clamp(Position, 0.0f, 1.0f);
}

void ARailsTrain::EmergencyBrake() {
  BrakePosition = 1.0f;
  ThrottlePosition = 0.0f;
  UE_LOG(LogTemp, Warning, TEXT("EMERGENCY BRAKE!"));
}

void ARailsTrain::StartTrain() {
  if (CurrentGear == ETrainGear::Neutral) {
    SetGear(ETrainGear::Forward);
  }
  SetThrottle(1.0f);
}

void ARailsTrain::StopTrain() { EmergencyBrake(); }

// QUERY FUNCTIONS

float ARailsTrain::GetSpeedKmh() const { return CurrentSpeed * 0.036f; }

FString ARailsTrain::GetGearString() const {
  switch (CurrentGear) {
  case ETrainGear::Forward:
    return TEXT("F");
  case ETrainGear::Reverse:
    return TEXT("R");
  case ETrainGear::Neutral:
    return TEXT("N");
  default:
    return TEXT("?");
  }
}

bool ARailsTrain::IsCharacterOnTrain(ACharacter *Character) const {
  return PassengersOnBoard.Contains(Character);
}
