// Copyright Epic Games, Inc. All Rights Reserved.

#include "Train/RailSplineActor.h"
#include "Components/SplineComponent.h"

ARailSplineActor::ARailSplineActor() {
  PrimaryActorTick.bCanEverTick = false;

  // Create root component
  USceneComponent *Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
  SetRootComponent(Root);

  // Create spline component
  RailSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RailSpline"));
  RailSpline->SetupAttachment(Root);
  RailSpline->SetClosedLoop(false);

  // Set default spline properties
  RailSpline->SetUnselectedSplineSegmentColor(FLinearColor(0.2f, 0.5f, 1.0f));
  RailSpline->SetSelectedSplineSegmentColor(FLinearColor(1.0f, 0.5f, 0.2f));
  RailSpline->SetDrawDebug(true);
}

void ARailSplineActor::BeginPlay() {
  Super::BeginPlay();

  if (!RailSpline) {
    UE_LOG(LogTemp, Error,
           TEXT("RailSplineActor: RailSpline component is null!"));
    return;
  }

  UE_LOG(LogTemp, Log, TEXT("RailSplineActor: Initialized with length %.2f"),
         GetRailLength());
}

float ARailSplineActor::GetRailLength() const {
  if (!RailSpline) {
    UE_LOG(LogTemp, Warning,
           TEXT("RailSplineActor: RailSpline is null in GetRailLength!"));
    return 0.0f;
  }

  return RailSpline->GetSplineLength();
}

bool ARailSplineActor::IsPointNearRail(FVector Point, float Threshold) const {
  if (!RailSpline) {
    return false;
  }

  // Find closest point on spline
  const float InputKey = RailSpline->FindInputKeyClosestToWorldLocation(Point);
  const FVector ClosestPoint = RailSpline->GetLocationAtSplineInputKey(
      InputKey, ESplineCoordinateSpace::World);

  // Calculate distance
  const float Distance = FVector::Dist(Point, ClosestPoint);

  return Distance <= Threshold;
}
