// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/SplineComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RailSplineActor.generated.h"

UCLASS()
class EPOCHRAILS_API ARailSplineActor : public AActor {
  GENERATED_BODY()

public:
  ARailSplineActor();

protected:
  virtual void BeginPlay() override;

public:
  // Spline that defines the rail path
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rail")
  TObjectPtr<USplineComponent> RailSpline;

  // Get spline component (for LocomotionComponent)
  UFUNCTION(BlueprintPure, Category = "Rail")
  USplineComponent *GetSplineComponent() const { return RailSpline; }

  // Get the total length of the rail
  UFUNCTION(BlueprintPure, Category = "Rail")
  float GetRailLength() const;

  // Check if a point is near this rail
  UFUNCTION(BlueprintPure, Category = "Rail")
  bool IsPointNearRail(FVector Point, float Threshold = 500.0f) const;
};
