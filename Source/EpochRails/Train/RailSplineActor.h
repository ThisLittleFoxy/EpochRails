#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RailSplineActor.generated.h"

class USplineComponent;

UCLASS()
class EPOCHRAILS_API ARailSplineActor : public AActor {
  GENERATED_BODY()

public:
  ARailSplineActor();

protected:
  virtual void BeginPlay() override;

public:
  virtual void Tick(float DeltaTime) override;

  // Spline that defines the rail path
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rail")
  USplineComponent *RailSpline;
};
