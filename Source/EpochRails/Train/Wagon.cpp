// Wagon.cpp

#include "Wagon.h"
#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "RailsTrain.h"

AWagon::AWagon() {
  PrimaryActorTick.bCanEverTick = true;

  // Create root
  WagonRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WagonRoot"));
  RootComponent = WagonRoot;

  // Create FloatingPawnMovement for smooth interpolated movement
  MovementComponent =
      CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
  MovementComponent->UpdatedComponent = WagonRoot;
  MovementComponent->MaxSpeed = 5000.0f;
  MovementComponent->Acceleration = 2000.0f;
  MovementComponent->Deceleration = 4000.0f;
  MovementComponent->TurningBoost = 0.0f;

  // Create front attachment point
  FrontAttachmentPoint =
      CreateDefaultSubobject<USceneComponent>(TEXT("FrontAttachmentPoint"));
  FrontAttachmentPoint->SetupAttachment(WagonRoot);
  FrontAttachmentPoint->SetRelativeLocation(FVector(400.0f, 0.0f, 0.0f));

  // Create rear attachment point
  RearAttachmentPoint =
      CreateDefaultSubobject<USceneComponent>(TEXT("RearAttachmentPoint"));
  RearAttachmentPoint->SetupAttachment(WagonRoot);
  RearAttachmentPoint->SetRelativeLocation(FVector(-400.0f, 0.0f, 0.0f));

// Create platform mesh
  PlatformMesh =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
  PlatformMesh->SetupAttachment(WagonRoot);

  PlatformMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  PlatformMesh->SetCollisionProfileName(
      TEXT("BlockAllDynamic")); // ��� BlockAll
  PlatformMesh->SetGenerateOverlapEvents(false);
  PlatformMesh->SetCanEverAffectNavigation(false);
  PlatformMesh->SetSimulatePhysics(false);
  PlatformMesh->SetEnableGravity(false);



  // Create building zone
  BuildingZone = CreateDefaultSubobject<UBoxComponent>(TEXT("BuildingZone"));
  BuildingZone->SetupAttachment(WagonRoot);
  BuildingZone->SetBoxExtent(FVector(400.0f, 200.0f, 200.0f));
  BuildingZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  BuildingZone->SetCollisionResponseToAllChannels(ECR_Ignore);
  BuildingZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

  // Create wheel meshes
  for (int32 i = 0; i < 4; i++) {
    FString WheelName = FString::Printf(TEXT("Wheel_%d"), i);
    UStaticMeshComponent *Wheel =
        CreateDefaultSubobject<UStaticMeshComponent>(FName(*WheelName));
    Wheel->SetupAttachment(WagonRoot);
    Wheel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WheelMeshes.Add(Wheel);
  }
}

void AWagon::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  if (!CachedSplineComponent)
    return;

  // Clamp delta time to prevent large jumps
  const float SafeDeltaTime = FMath::Min(DeltaTime, 0.033f);

  const float SplineLength = CachedSplineComponent->GetSplineLength();

  // Directly use target distance - smoothness comes from position interpolation
  CurrentSplineDistance = TargetDistance;

  // Wrap spline distance
  while (CurrentSplineDistance < 0.0f)
    CurrentSplineDistance += SplineLength;
  while (CurrentSplineDistance >= SplineLength)
    CurrentSplineDistance -= SplineLength;

  // Calculate target position on spline
  FVector TargetLocation =
      CachedSplineComponent->GetLocationAtDistanceAlongSpline(
          CurrentSplineDistance, ESplineCoordinateSpace::World);

  const FVector Up = CachedSplineComponent->GetUpVectorAtDistanceAlongSpline(
      CurrentSplineDistance, ESplineCoordinateSpace::World);
  TargetLocation += Up * HeightOffset;

  const FRotator TargetRotation =
      CachedSplineComponent->GetRotationAtDistanceAlongSpline(
          CurrentSplineDistance, ESplineCoordinateSpace::World);

  // Move directly to target - spline provides smooth path, no collision needed
  SetActorLocationAndRotation(TargetLocation, TargetRotation);
}


void AWagon::Initialize(USplineComponent *Spline, AActor *Leader) {
  CachedSplineComponent = Spline;
  PreviousVehicle = Leader;

  if (!CachedSplineComponent || !Leader) {
    UE_LOG(LogTemp, Error, TEXT("Wagon: Invalid initialization parameters!"));
    return;
  }

  // Get leader's distance along spline (center distance)
  float LeaderDistance = 0.0f;

  if (AWagon *PrevWagon = Cast<AWagon>(Leader)) {
    LeaderDistance = PrevWagon->GetCurrentDistance();
    UE_LOG(LogTemp, Log,
           TEXT("Wagon: Following previous wagon at distance %.1f"),
           LeaderDistance);
  } else if (ARailsTrain *Train = Cast<ARailsTrain>(Leader)) {
    LeaderDistance = Train->GetCurrentSplineDistance();
    UE_LOG(LogTemp, Log, TEXT("Wagon: Following train at distance %.1f"),
           LeaderDistance);
  } else {
    UE_LOG(LogTemp, Error, TEXT("Wagon: Unknown leader type!"));
    return;
  }

  // Use the same logic as runtime follow: align my Front to leader Rear (+ gap)
  SetTargetDistance(LeaderDistance);
  CurrentSplineDistance = TargetDistance;

  // Wrap
  const float SplineLength = CachedSplineComponent->GetSplineLength();
  while (CurrentSplineDistance < 0.0f)
    CurrentSplineDistance += SplineLength;
  while (CurrentSplineDistance > SplineLength)
    CurrentSplineDistance -= SplineLength;

  // Set initial position immediately
  FVector InitialLocation =
      CachedSplineComponent->GetLocationAtDistanceAlongSpline(
          CurrentSplineDistance, ESplineCoordinateSpace::World);

  // Apply vertical offset along spline Up vector
  const FVector Up = CachedSplineComponent->GetUpVectorAtDistanceAlongSpline(
      CurrentSplineDistance, ESplineCoordinateSpace::World);
  InitialLocation += Up * HeightOffset;

  FRotator InitialRotation =
      CachedSplineComponent->GetRotationAtDistanceAlongSpline(
          CurrentSplineDistance, ESplineCoordinateSpace::World);

SetActorLocationAndRotation(InitialLocation, InitialRotation, false, nullptr,
                              ETeleportType::None);


  UE_LOG(LogTemp, Log,
         TEXT("Wagon initialized: LeaderCenter=%.1f, WagonTarget=%.1f "
              "(Gap=%.1f, HeightOffset=%.1f)"),
         LeaderDistance, CurrentSplineDistance, CouplingGap, HeightOffset);
}


void AWagon::SetTargetDistance(float LeaderCenterDistance) {
  if (!CachedSplineComponent || !PreviousVehicle)
    return;

  const float SplineLength = CachedSplineComponent->GetSplineLength();

  // Leader rear offset in cm along spline (use relative X, stable and cheap)
  float LeaderRearOffset = 0.0f;

  if (AWagon *PrevWagon = Cast<AWagon>(PreviousVehicle)) {
    if (PrevWagon->RearAttachmentPoint)
      LeaderRearOffset =
          FMath::Abs(PrevWagon->RearAttachmentPoint->GetRelativeLocation().X);
  } else if (ARailsTrain *Train = Cast<ARailsTrain>(PreviousVehicle)) {
    if (USceneComponent *TrainRearPoint = Train->GetRearAttachmentPoint()) {
      LeaderRearOffset = FMath::Abs(TrainRearPoint->GetRelativeLocation().X);
    }
  }


  const float LeaderRearDistance = LeaderCenterDistance - LeaderRearOffset;

  float MyFrontOffset = FollowDistance;
  if (FrontAttachmentPoint)
    MyFrontOffset = FMath::Abs(FrontAttachmentPoint->GetRelativeLocation().X);

  TargetDistance = LeaderRearDistance - (MyFrontOffset + CouplingGap);

  // Wrap
  while (TargetDistance < 0.0f)
    TargetDistance += SplineLength;
  while (TargetDistance >= SplineLength)
    TargetDistance -= SplineLength;

  // Propagate ideal leader center distance (NOT current)
  if (NextWagon && IsValid(NextWagon))
    NextWagon->SetTargetDistance(/*LeaderCenterDistance=*/TargetDistance);
}


void AWagon::SetNextWagon(AWagon *Wagon) { NextWagon = Wagon; }

float AWagon::GetTotalWeight() const { return WagonWeight; }

void AWagon::DetachFromChain() {
  // Disconnect from previous
  if (PreviousVehicle) {
    if (AWagon *PrevWagon = Cast<AWagon>(PreviousVehicle)) {
      PrevWagon->NextWagon = nullptr;
    }
  }

  // Disconnect from next
  if (NextWagon) {
    NextWagon->PreviousVehicle = nullptr;
    NextWagon = nullptr;
  }

  PreviousVehicle = nullptr;
  CachedSplineComponent = nullptr;

  UE_LOG(LogTemp, Log, TEXT("Wagon detached from chain"));
}
