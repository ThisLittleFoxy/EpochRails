#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
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
  USplineComponent *RailSpline;

  // Get the total length of the rail
  UFUNCTION(BlueprintPure, Category = "Rail")
  float GetRailLength() const;

  // Check if a point is near this rail
  UFUNCTION(BlueprintPure, Category = "Rail")
  bool IsPointNearRail(FVector Point, float Threshold = 500.0f) const;
};
