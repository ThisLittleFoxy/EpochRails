#include "Train/RailSplineActor.h"
#include "Components/SplineComponent.h"

ARailSplineActor::ARailSplineActor() {
  PrimaryActorTick.bCanEverTick = false;

  RailSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RailSpline"));
  SetRootComponent(RailSpline);
}

void ARailSplineActor::BeginPlay() { Super::BeginPlay(); }

void ARailSplineActor::Tick(float DeltaTime) { Super::Tick(DeltaTime); }
