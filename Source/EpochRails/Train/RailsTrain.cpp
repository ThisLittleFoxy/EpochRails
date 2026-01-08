// RailsTrain.cpp

#include "RailsTrain.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/KismetMathLibrary.h"

#include "Character/RailsPlayerCharacter.h"
#include "RailsSplinePath.h"

ARailsTrain::ARailsTrain() {
  PrimaryActorTick.bCanEverTick = true;

  Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
  SetRootComponent(Root);

  BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
  BodyMesh->SetupAttachment(Root);
  BodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  BodyMesh->SetCollisionProfileName(TEXT("BlockAll"));

  Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
  Movement->UpdatedComponent = Root;

  InteriorTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteriorTrigger"));
  InteriorTrigger->SetupAttachment(Root);
  InteriorTrigger->SetBoxExtent(FVector(500.f, 250.f, 200.f));
  InteriorTrigger->SetCollisionProfileName(TEXT("Trigger"));
  InteriorTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  InteriorTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
  InteriorTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ARailsTrain::BeginPlay() {
  Super::BeginPlay();

  if (InteriorTrigger) {
    InteriorTrigger->OnComponentBeginOverlap.AddDynamic(
        this, &ARailsTrain::OnInteriorBeginOverlap);
    InteriorTrigger->OnComponentEndOverlap.AddDynamic(
        this, &ARailsTrain::OnInteriorEndOverlap);
  }

  if (bAutoStart) {
    StartTrain();
  }
}

void ARailsTrain::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  if (bStop) {
    return;
  }

  // Move forward using FloatingPawnMovement
  AddMovementInput(GetActorForwardVector(), Speed);

  // Update rotation to follow spline
  UpdatePath();
}

void ARailsTrain::StartTrain() {
  bStop = false;
}

void ARailsTrain::StopTrain() {
  bStop = true;
}

USplineComponent *ARailsTrain::GetActiveSpline() const {
  if (!IsValid(ActivePath)) {
    return nullptr;
  }
  return ActivePath->GetSpline();
}

void ARailsTrain::UpdatePath() {
  USplineComponent *Spline = GetActiveSpline();
  if (!Spline) {
    return;
  }

  const FVector ActorLocation = GetActorLocation();

  // Check if reached end of spline
  const int32 NumPoints = Spline->GetNumberOfSplinePoints();
  if (NumPoints <= 0) {
    return;
  }

  const int32 LastIndex = NumPoints - 1;
  const FVector EndLocation =
      Spline->GetLocationAtSplinePoint(LastIndex, ESplineCoordinateSpace::World);

  // Stop if reached the end
  if (EndLocation.Equals(ActorLocation, StopTolerance)) {
    bStop = true;
    return;
  }

  // Find tangent at closest point on spline
  const FVector Tangent =
      Spline->FindTangentClosestToWorldLocation(ActorLocation, ESplineCoordinateSpace::World);
  const FVector Dir = Tangent.GetSafeNormal();

  // Look ahead position
  const FVector LookAheadWorld = ActorLocation + (Dir * LookAheadDistance);

  // Find closest point on spline to look-ahead position
  const FVector Closest =
      Spline->FindLocationClosestToWorldLocation(LookAheadWorld, ESplineCoordinateSpace::World);

  // Rotate to face the target point
  const FRotator NewRot = UKismetMathLibrary::FindLookAtRotation(ActorLocation, Closest);
  SetActorRotation(NewRot);
}

// ===== Passenger management =====

bool ARailsTrain::IsPassengerInside(ARailsPlayerCharacter *Character) const {
  if (!Character) {
    return false;
  }

  for (const TWeakObjectPtr<ARailsPlayerCharacter> &WeakPassenger : PassengersInside) {
    if (WeakPassenger.IsValid() && WeakPassenger.Get() == Character) {
      return true;
    }
  }
  return false;
}

void ARailsTrain::OnPlayerEnterTrain(ARailsPlayerCharacter *Character) {
  if (!Character || IsPassengerInside(Character)) {
    return;
  }

  PassengersInside.Add(Character);
  SwitchInputMappingContext(Character, true);

  UE_LOG(LogTemp, Log, TEXT("Player %s entered train"), *Character->GetName());
}

void ARailsTrain::OnPlayerExitTrain(ARailsPlayerCharacter *Character) {
  if (!Character || !IsPassengerInside(Character)) {
    return;
  }

  PassengersInside.RemoveAll(
      [Character](const TWeakObjectPtr<ARailsPlayerCharacter> &WeakPassenger) {
        return WeakPassenger.IsValid() && WeakPassenger.Get() == Character;
      });

  SwitchInputMappingContext(Character, false);

  UE_LOG(LogTemp, Log, TEXT("Player %s exited train"), *Character->GetName());
}

void ARailsTrain::SwitchInputMappingContext(ARailsPlayerCharacter *Character, bool bInsideTrain) {
  if (!Character) {
    return;
  }

  UEnhancedInputLocalPlayerSubsystem *Subsystem = GetInputSubsystem(Character);
  if (!Subsystem) {
    return;
  }

  if (bInsideTrain) {
    if (DefaultInputMappingContext) {
      Subsystem->RemoveMappingContext(DefaultInputMappingContext);
    }
    if (TrainPassengerInputMappingContext) {
      Subsystem->AddMappingContext(TrainPassengerInputMappingContext, IMCPriority);
    }
  } else {
    if (TrainPassengerInputMappingContext) {
      Subsystem->RemoveMappingContext(TrainPassengerInputMappingContext);
    }
    if (DefaultInputMappingContext) {
      Subsystem->AddMappingContext(DefaultInputMappingContext, IMCPriority);
    }
  }
}

UEnhancedInputLocalPlayerSubsystem *ARailsTrain::GetInputSubsystem(
    ARailsPlayerCharacter *Character) const {
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

void ARailsTrain::OnInteriorBeginOverlap(UPrimitiveComponent *OverlappedComponent,
                                         AActor *OtherActor,
                                         UPrimitiveComponent *OtherComp,
                                         int32 OtherBodyIndex, bool bFromSweep,
                                         const FHitResult &SweepResult) {
  if (ARailsPlayerCharacter *Player = Cast<ARailsPlayerCharacter>(OtherActor)) {
    OnPlayerEnterTrain(Player);
  }
}

void ARailsTrain::OnInteriorEndOverlap(UPrimitiveComponent *OverlappedComponent,
                                       AActor *OtherActor,
                                       UPrimitiveComponent *OtherComp,
                                       int32 OtherBodyIndex) {
  if (ARailsPlayerCharacter *Player = Cast<ARailsPlayerCharacter>(OtherActor)) {
    OnPlayerExitTrain(Player);
  }
}
