// RailsSplinePath.cpp
#include "RailsSplinePath.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

ARailsSplinePath::ARailsSplinePath() {
  PrimaryActorTick.bCanEverTick = false;

  // Create root component
  USceneComponent *Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
  RootComponent = Root;

  // Create spline component
  SplineComponent =
      CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
  SplineComponent->SetupAttachment(RootComponent);
  SplineComponent->SetClosedLoop(false);

  // Set default spline points
  SplineComponent->ClearSplinePoints();
  SplineComponent->AddSplinePoint(FVector(0, 0, 0),
                                  ESplineCoordinateSpace::Local);
  SplineComponent->AddSplinePoint(FVector(1000, 0, 0),
                                  ESplineCoordinateSpace::Local);
}

FVector ARailsSplinePath::GetLocationAtDistance(float Distance) const {
  if (!SplineComponent)
    return FVector::ZeroVector;
  return SplineComponent->GetLocationAtDistanceAlongSpline(
      Distance, ESplineCoordinateSpace::World);
}

FRotator ARailsSplinePath::GetRotationAtDistance(float Distance) const {
  if (!SplineComponent)
    return FRotator::ZeroRotator;
  return SplineComponent->GetRotationAtDistanceAlongSpline(
      Distance, ESplineCoordinateSpace::World);
}

float ARailsSplinePath::GetSplineLength() const {
  if (!SplineComponent)
    return 0.0f;
  return SplineComponent->GetSplineLength();
}

#if WITH_EDITOR
void ARailsSplinePath::OnConstruction(const FTransform &Transform) {
  Super::OnConstruction(Transform);

  // Update visualization in editor
  if (bShowDebugVisualization && SplineComponent) {
    // Debug visualization is handled by DrawDebugHelpers in Tick or via editor
  }
}
#endif
