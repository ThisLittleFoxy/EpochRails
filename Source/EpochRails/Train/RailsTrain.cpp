// RailsTrain.cpp
#include "RailsTrain.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "RailsSplinePath.h"

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
  TrainBodyMesh->SetCollisionObjectType(ECC_WorldDynamic);

  // Create platform mesh
  PlatformMesh =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
  PlatformMesh->SetupAttachment(TrainBodyMesh);
  PlatformMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  PlatformMesh->SetCollisionObjectType(ECC_WorldDynamic);

  // CRITICAL: Enable "Impart Base Angular Velocity" and "Impart Base Velocity"
  // This ensures characters inherit train's movement
  PlatformMesh->SetSimulatePhysics(false);

  // Create boarding zone
  BoardingZone = CreateDefaultSubobject<UBoxComponent>(TEXT("BoardingZone"));
  BoardingZone->SetupAttachment(PlatformMesh);
  BoardingZone->SetBoxExtent(FVector(200.0f, 150.0f, 100.0f));
  BoardingZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  BoardingZone->SetCollisionResponseToAllChannels(ECR_Ignore);
  BoardingZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ARailsTrain::BeginPlay() {
  Super::BeginPlay();

  // Bind overlap events
  if (BoardingZone) {
    BoardingZone->OnComponentBeginOverlap.AddDynamic(
        this, &ARailsTrain::OnBoardingZoneBeginOverlap);
    BoardingZone->OnComponentEndOverlap.AddDynamic(
        this, &ARailsTrain::OnBoardingZoneEndOverlap);
  }

  // Auto-start if enabled
  if (bAutoStart) {
    StartTrain();
  }
}

void ARailsTrain::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  if (TrainState != ETrainState::Stopped && SplinePathRef) {
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

  // Calculate target speed based on state
  float TargetSpeed = GetTargetSpeed();

  // Interpolate current speed towards target speed
  if (FMath::Abs(CurrentSpeed - TargetSpeed) > 1.0f) {
    float AccelRate = (FMath::Abs(CurrentSpeed) < FMath::Abs(TargetSpeed))
                          ? AccelerationRate
                          : DecelerationRate;
    CurrentSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime,
                                    AccelRate / 100.0f);
  } else {
    CurrentSpeed = TargetSpeed;
  }

  // Update distance along spline (can go negative for reverse)
  CurrentDistance += CurrentSpeed * DeltaTime;

  // Handle looping/boundaries
  if (CurrentDistance >= SplineLength) {
    if (bLoopPath) {
      CurrentDistance = FMath::Fmod(CurrentDistance, SplineLength);
    } else {
      CurrentDistance = SplineLength;
      StopTrain();
    }
  } else if (CurrentDistance < 0.0f) // Handle reverse out of bounds
  {
    if (bLoopPath) {
      CurrentDistance = SplineLength + CurrentDistance;
    } else {
      CurrentDistance = 0.0f;
      CurrentSpeed = 0.0f;
      TrainState = ETrainState::Stopped;
    }
  }

  // Move train to new position
  MoveToDistance(CurrentDistance);
}

void ARailsTrain::MoveToDistance(float Distance) {
  if (!SplinePathRef)
    return;

  USplineComponent *Spline = SplinePathRef->GetSpline();
  if (!Spline)
    return;

  // Get location and rotation at distance
  FVector NewLocation = Spline->GetLocationAtDistanceAlongSpline(
      Distance, ESplineCoordinateSpace::World);
  FRotator NewRotation = Spline->GetRotationAtDistanceAlongSpline(
      Distance, ESplineCoordinateSpace::World);

  // Set actor transform
  SetActorLocationAndRotation(NewLocation, NewRotation);
}

void ARailsTrain::ApplyThrottle(float ThrottleValue) {
  CurrentThrottle = FMath::Clamp(ThrottleValue, -1.0f, 1.0f);

  // Update train state based on throttle
  if (FMath::Abs(CurrentThrottle) > 0.01f) {
    TrainState = ETrainState::Accelerating;
  } else if (CurrentBrake > 0.01f) {
    TrainState = ETrainState::Decelerating;
  } else {
    TrainState = ETrainState::Moving;
  }
}

void ARailsTrain::ApplyBrake(float BrakeValue) {
  CurrentBrake = FMath::Clamp(BrakeValue, 0.0f, 1.0f);

  if (CurrentBrake > 0.01f) {
    TrainState = ETrainState::Decelerating;
  }
}

// Update GetTargetSpeed function
float ARailsTrain::GetTargetSpeed() const {
  // If braking, reduce speed
  if (CurrentBrake > 0.01f) {
    return 0.0f;
  }

  // Apply throttle to max speed (preserve sign for reverse)
  return MaxSpeed * CurrentThrottle; // Убрали FMath::Abs()!
}

void ARailsTrain::OnBoardingZoneBeginOverlap(
    UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult &SweepResult) {
  // Check if it's a character
  ACharacter *Character = Cast<ACharacter>(OtherActor);
  if (!Character)
    return;

  // Add to passengers list
  if (!PassengersOnBoard.Contains(Character)) {
    PassengersOnBoard.Add(Character);

    // Character movement component will automatically inherit platform movement
    // if "Impart Base Velocity" is enabled (which is default for ACharacter)
    UE_LOG(LogTemp, Log, TEXT("Character %s boarded train"),
           *Character->GetName());
  }
}

void ARailsTrain::OnBoardingZoneEndOverlap(
    UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex) {
  // Check if it's a character
  ACharacter *Character = Cast<ACharacter>(OtherActor);
  if (!Character)
    return;

  // Remove from passengers list
  if (PassengersOnBoard.Contains(Character)) {
    PassengersOnBoard.Remove(Character);
    UE_LOG(LogTemp, Log, TEXT("Character %s left train"),
           *Character->GetName());
  }
}

void ARailsTrain::StartTrain() {
  if (TrainState == ETrainState::Stopped) {
    TrainState = ETrainState::Accelerating;
    UE_LOG(LogTemp, Log, TEXT("Train started"));
  }
}

void ARailsTrain::StopTrain() {
  if (TrainState != ETrainState::Stopped) {
    TrainState = ETrainState::Decelerating;
    UE_LOG(LogTemp, Log, TEXT("Train stopping"));
  }
}

void ARailsTrain::SetSpeed(float NewSpeed) {
  CurrentSpeed = FMath::Clamp(NewSpeed, 0.0f, MaxSpeed);
}

bool ARailsTrain::IsCharacterOnTrain(ACharacter *Character) const {
  return PassengersOnBoard.Contains(Character);
}