#pragma once

#include "Components/ActorComponent.h"
#include "Components/SplineComponent.h"
#include "CoreMinimal.h"
#include "LocomotionComponent.generated.h"

// Delegate for location changed
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLocationChanged, FVector);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSpeedChanged, float, float);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EPOCHRAILS_API ULocomotionComponent : public UActorComponent {
  GENERATED_BODY()

public:
  ULocomotionComponent();

  virtual void BeginPlay() override;
  virtual void
  TickComponent(float DeltaTime, ELevelTick TickType,
                FActorComponentTickFunction *ThisTickFunction) override;

  // Set throttle input (-1 to 1)
  UFUNCTION(BlueprintCallable, Category = "Locomotion")
  void SetThrottle(float Value);

  // Apply brakes
  UFUNCTION(BlueprintCallable, Category = "Locomotion")
  void ApplyBrakes(float BrakeForce);

  // Get current speed
  UFUNCTION(BlueprintPure, Category = "Locomotion")
  float GetCurrentSpeed() const { return CurrentVelocity; }

  // Get forward direction
  UFUNCTION(BlueprintPure, Category = "Locomotion")
  FVector GetForwardDirection() const;

  // Get current position on spline
  UFUNCTION(BlueprintPure, Category = "Locomotion")
  float GetDistanceAlongSpline() const { return CurrentDistance; }

  // Set spline component for rail
  UFUNCTION(BlueprintCallable, Category = "Locomotion")
  void SetRailSpline(USplineComponent *NewSpline);

  // НОВОЕ: Установить начальную позицию на сплайне
  UFUNCTION(BlueprintCallable, Category = "Locomotion")
  void SetStartDistance(float Distance);

  // НОВОЕ: Переместиться к ближайшей точке на сплайне
  UFUNCTION(BlueprintCallable, Category = "Locomotion")
  void SnapToNearestPointOnSpline();

public:
  // Events
  FOnLocationChanged OnLocationChanged;
  FOnSpeedChanged OnSpeedChanged;

private:
  // Reference to rail spline component
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion",
            meta = (AllowPrivateAccess = "true"))
  USplineComponent *RailSpline = nullptr;

  // Current position along spline (in units)
  UPROPERTY(VisibleAnywhere, Category = "Locomotion")
  float CurrentDistance = 0.0f;

  // Current velocity (speed)
  UPROPERTY(VisibleAnywhere, Category = "Locomotion")
  float CurrentVelocity = 0.0f;

  // Throttle input (-1 to 1)
  UPROPERTY(VisibleAnywhere, Category = "Locomotion")
  float ThrottleInput = 0.0f;

  // Movement parameters
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
            meta = (AllowPrivateAccess = "true"))
  float MaxSpeed = 1000.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
            meta = (AllowPrivateAccess = "true"))
  float Acceleration = 200.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
            meta = (AllowPrivateAccess = "true"))
  float BrakingDeceleration = 300.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
            meta = (AllowPrivateAccess = "true"))
  float DragDeceleration = 50.0f; // Natural deceleration

  // НОВОЕ: Автоматически позиционировать на старте
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
            meta = (AllowPrivateAccess = "true"))
  bool bAutoPositionOnStart = true;

  // НОВОЕ: Стартовая дистанция на сплайне (0 = начало, 1 = конец при
  // использовании процентов)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
            meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
  float StartDistance = 0.0f;

  // НОВОЕ: Использовать проценты вместо абсолютной дистанции для StartDistance
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
            meta = (AllowPrivateAccess = "true"))
  bool bUsePercentageForStart = false;

  // Update position along spline
  void UpdatePosition(float DeltaTime);

  // Update actor rotation to match spline direction
  void UpdateRotation();

  // НОВОЕ: Инициализировать позицию при назначении сплайна
  void InitializePosition();
};
