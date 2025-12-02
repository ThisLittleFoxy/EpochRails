// RailsSplinePath.h
#pragma once

#include "Components/SplineComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RailsSplinePath.generated.h"

/**
 * Spline path for trains to follow
 * Can be placed in level and edited visually
 */
UCLASS(Blueprintable)
class EPOCHRAILS_API ARailsSplinePath : public AActor {
  GENERATED_BODY()

public:
  ARailsSplinePath();

protected:
  /** The spline component defining the path */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USplineComponent *SplineComponent;

  /** Mesh to visualize the path (optional) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  class USplineMeshComponent *PathVisualizationMesh;

  /** Show debug visualization in editor */
  UPROPERTY(EditAnywhere, Category = "Debug")
  bool bShowDebugVisualization = true;

  /** Color of debug visualization */
  UPROPERTY(EditAnywhere, Category = "Debug")
  FLinearColor DebugColor = FLinearColor::Yellow;

public:
  /** Get the spline component */
  UFUNCTION(BlueprintPure, Category = "Spline")
  USplineComponent *GetSpline() const { return SplineComponent; }

  /** Get location at distance along spline */
  UFUNCTION(BlueprintPure, Category = "Spline")
  FVector GetLocationAtDistance(float Distance) const;

  /** Get rotation at distance along spline */
  UFUNCTION(BlueprintPure, Category = "Spline")
  FRotator GetRotationAtDistance(float Distance) const;

  /** Get total spline length */
  UFUNCTION(BlueprintPure, Category = "Spline")
  float GetSplineLength() const;

#if WITH_EDITOR
  virtual void OnConstruction(const FTransform &Transform) override;
#endif
};
