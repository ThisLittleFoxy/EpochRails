#include "Train/RailSplineActor.h"
#include "Components/SplineComponent.h"

ARailSplineActor::ARailSplineActor() {
  PrimaryActorTick.bCanEverTick = false;

  // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Создаем компонент сплайна
  RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

  RailSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RailSpline"));
  RailSpline->SetupAttachment(RootComponent);

  // Настройки сплайна по умолчанию
  RailSpline->SetClosedLoop(false);
  RailSpline->Duration = 1.0f;

#if WITH_EDITORONLY_DATA
  // Видимость в редакторе
  RailSpline->bDrawDebug = true;
  RailSpline->SetUnselectedSplineSegmentColor(FLinearColor::Green);
  RailSpline->SetSelectedSplineSegmentColor(FLinearColor::Yellow);
#endif

  UE_LOG(LogTemp, Log, TEXT("ARailSplineActor: Spline component created"));
}

void ARailSplineActor::BeginPlay() {
  Super::BeginPlay();

  if (!RailSpline) {
    UE_LOG(LogTemp, Error,
           TEXT("ARailSplineActor: RailSpline is null after initialization!"));
    return;
  }

  const float Length = RailSpline->GetSplineLength();
  UE_LOG(LogTemp, Log,
         TEXT("ARailSplineActor '%s' initialized with length: %f"), *GetName(),
         Length);
}

float ARailSplineActor::GetRailLength() const {
  return RailSpline ? RailSpline->GetSplineLength() : 0.0f;
}

bool ARailSplineActor::IsPointNearRail(FVector Point, float Threshold) const {
  if (!RailSpline) {
    return false;
  }

  const float InputKey = RailSpline->FindInputKeyClosestToWorldLocation(Point);
  const FVector ClosestPoint = RailSpline->GetLocationAtSplineInputKey(
      InputKey, ESplineCoordinateSpace::World);
  const float Distance = FVector::Dist(Point, ClosestPoint);

  return Distance <= Threshold;
}
