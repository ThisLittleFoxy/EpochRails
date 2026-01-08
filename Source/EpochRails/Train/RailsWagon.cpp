// RailsWagon.cpp

#include "RailsWagon.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"

#include "RailsTrain.h"

ARailsWagon::ARailsWagon() {
  PrimaryActorTick.bCanEverTick = true;

  // Root component
  Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
  SetRootComponent(Root);

  // Platform mesh - the floor
  PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
  PlatformMesh->SetupAttachment(Root);
  PlatformMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  PlatformMesh->SetCollisionProfileName(TEXT("BlockAll"));

  // Movement component
  Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
  Movement->UpdatedComponent = Root;

  // Platform trigger for detecting players
  PlatformTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("PlatformTrigger"));
  PlatformTrigger->SetupAttachment(Root);
  PlatformTrigger->SetBoxExtent(FVector(200.0f, 100.0f, 150.0f));
  PlatformTrigger->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
  PlatformTrigger->SetCollisionProfileName(TEXT("Trigger"));
  PlatformTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  PlatformTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
  PlatformTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

  // Buildable zone visualization
  BuildableZone = CreateDefaultSubobject<UBoxComponent>(TEXT("BuildableZone"));
  BuildableZone->SetupAttachment(Root);
  BuildableZone->SetBoxExtent(FVector(PlatformSize.X * 0.5f, PlatformSize.Y * 0.5f, MaxBuildHeight * 0.5f));
  BuildableZone->SetRelativeLocation(FVector(0.0f, 0.0f, MaxBuildHeight * 0.5f));
  BuildableZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  BuildableZone->SetHiddenInGame(true);
  BuildableZone->ShapeColor = FColor::Green;

  // Coupler attachment points
  FrontCoupler = CreateDefaultSubobject<USceneComponent>(TEXT("FrontCoupler"));
  FrontCoupler->SetupAttachment(Root);
  FrontCoupler->SetRelativeLocation(FVector(PlatformSize.X * 0.5f + 25.0f, 0.0f, 0.0f));

  RearCoupler = CreateDefaultSubobject<USceneComponent>(TEXT("RearCoupler"));
  RearCoupler->SetupAttachment(Root);
  RearCoupler->SetRelativeLocation(FVector(-PlatformSize.X * 0.5f - 25.0f, 0.0f, 0.0f));
}

void ARailsWagon::BeginPlay() {
  Super::BeginPlay();
}

void ARailsWagon::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  if (LeaderVehicle.IsValid() && CachedSpline) {
    UpdateMovement(DeltaTime);
  }
}

// ===== Chain API =====

void ARailsWagon::AttachToLeader(AActor *Leader, USplineComponent *Spline) {
  if (!Leader || !Spline) {
    UE_LOG(LogTemp, Warning, TEXT("ARailsWagon::AttachToLeader - Invalid leader or spline"));
    return;
  }

  LeaderVehicle = Leader;
  CachedSpline = Spline;

  // Initialize position behind the leader
  float LeaderDistance = GetLeaderSplineDistance();
  CurrentSplineDistance = FMath::Max(0.0f, LeaderDistance - FollowDistance);

  // Set initial position on spline
  FVector InitialLocation = CachedSpline->GetLocationAtDistanceAlongSpline(
      CurrentSplineDistance, ESplineCoordinateSpace::World);
  FRotator InitialRotation = CachedSpline->GetRotationAtDistanceAlongSpline(
      CurrentSplineDistance, ESplineCoordinateSpace::World);

  SetActorLocationAndRotation(InitialLocation, InitialRotation);

  UE_LOG(LogTemp, Log, TEXT("Wagon attached to %s at distance %.1f"),
         *Leader->GetName(), CurrentSplineDistance);
}

void ARailsWagon::Detach() {
  // Notify next wagon if exists
  if (NextWagon.IsValid()) {
    NextWagon->Detach();
  }

  LeaderVehicle.Reset();
  CachedSpline = nullptr;
  NextWagon.Reset();

  UE_LOG(LogTemp, Log, TEXT("Wagon detached"));
}

float ARailsWagon::GetLeaderSplineDistance() const {
  if (!LeaderVehicle.IsValid()) {
    return 0.0f;
  }

  // Check if leader is a train
  if (ARailsTrain *Train = Cast<ARailsTrain>(LeaderVehicle.Get())) {
    return Train->GetCurrentSplineDistance();
  }

  // Check if leader is another wagon
  if (ARailsWagon *PrevWagon = Cast<ARailsWagon>(LeaderVehicle.Get())) {
    return PrevWagon->GetCurrentSplineDistance();
  }

  return 0.0f;
}

void ARailsWagon::UpdateMovement(float DeltaTime) {
  if (!CachedSpline) {
    return;
  }

  // Get leader's position on spline
  float LeaderDistance = GetLeaderSplineDistance();

  // Target position is behind the leader
  float TargetDistance = FMath::Max(0.0f, LeaderDistance - FollowDistance);

  // Smoothly interpolate to target distance
  CurrentSplineDistance = FMath::FInterpTo(CurrentSplineDistance, TargetDistance, DeltaTime, InterpSpeed);

  // Get target location and rotation from spline
  FVector TargetLocation = CachedSpline->GetLocationAtDistanceAlongSpline(
      CurrentSplineDistance, ESplineCoordinateSpace::World);
  FRotator TargetRotation = CachedSpline->GetRotationAtDistanceAlongSpline(
      CurrentSplineDistance, ESplineCoordinateSpace::World);

  // Calculate movement delta
  FVector CurrentLocation = GetActorLocation();
  FVector Delta = TargetLocation - CurrentLocation;

  // Apply movement using FloatingPawnMovement
  FHitResult Hit;
  Movement->SafeMoveUpdatedComponent(Delta, TargetRotation.Quaternion(), true, Hit);
}

// ===== Structure Placement API =====

bool ARailsWagon::CanPlaceStructure(const FVector &WorldLocation, const FVector &StructureExtent) const {
  // Convert to local coordinates
  FVector LocalLocation = GetActorTransform().InverseTransformPosition(WorldLocation);

  // Check X bounds (length)
  float HalfLength = PlatformSize.X * 0.5f;
  if (FMath::Abs(LocalLocation.X) + StructureExtent.X > HalfLength) {
    return false;
  }

  // Check Y bounds (width)
  float HalfWidth = PlatformSize.Y * 0.5f;
  if (FMath::Abs(LocalLocation.Y) + StructureExtent.Y > HalfWidth) {
    return false;
  }

  // Check Z bounds (height) - must be above platform and below max height
  if (LocalLocation.Z < 0.0f || LocalLocation.Z + StructureExtent.Z > MaxBuildHeight) {
    return false;
  }

  return true;
}

bool ARailsWagon::PlaceStructure(AActor *Structure) {
  if (!Structure) {
    return false;
  }

  // Attach to this wagon - structure will now move with the wagon
  Structure->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

  // Track for serialization/saving
  PlacedStructures.Add(Structure);

  UE_LOG(LogTemp, Log, TEXT("Structure %s placed on wagon"), *Structure->GetName());
  return true;
}

bool ARailsWagon::RemoveStructure(AActor *Structure) {
  if (!Structure) {
    return false;
  }

  // Check if this structure is on this wagon
  int32 Index = PlacedStructures.IndexOfByPredicate([Structure](const TWeakObjectPtr<AActor> &WeakActor) {
    return WeakActor.IsValid() && WeakActor.Get() == Structure;
  });

  if (Index == INDEX_NONE) {
    return false;
  }

  // Detach from wagon
  Structure->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

  // Remove from tracking
  PlacedStructures.RemoveAt(Index);

  UE_LOG(LogTemp, Log, TEXT("Structure %s removed from wagon"), *Structure->GetName());
  return true;
}

TArray<AActor *> ARailsWagon::GetPlacedStructures() const {
  TArray<AActor *> Result;

  for (const TWeakObjectPtr<AActor> &WeakActor : PlacedStructures) {
    if (WeakActor.IsValid()) {
      Result.Add(WeakActor.Get());
    }
  }

  return Result;
}

FBox ARailsWagon::GetBuildableZoneBounds() const {
  FVector HalfExtent(PlatformSize.X * 0.5f, PlatformSize.Y * 0.5f, MaxBuildHeight);
  FVector Min(-HalfExtent.X, -HalfExtent.Y, 0.0f);
  FVector Max(HalfExtent.X, HalfExtent.Y, MaxBuildHeight);

  return FBox(Min, Max);
}
