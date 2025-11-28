// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "LocomotionComponent.generated.h"

class ARailSplineActor;
class USplineComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EPOCHRAILS_API ULocomotionComponent : public UActorComponent {
  GENERATED_BODY()

public:
  ULocomotionComponent();

protected:
  virtual void BeginPlay() override;

public:
  virtual void
  TickComponent(float DeltaTime, ELevelTick TickType,
                FActorComponentTickFunction *ThisTickFunction) override;

  // Rail spline management
  UFUNCTION(BlueprintCallable, Category = "Locomotion")
  void SetRailSpline(ARailSplineActor *NewRailSpline);

  UFUNCTION(BlueprintPure, Category = "Locomotion")
  ARailSplineActor *GetRailSpline() const { return CurrentRailSpline.Get(); }

  // Movement control
  UFUNCTION(BlueprintCallable, Category = "Locomotion")
  void SetThrottle(float ThrottleValue);

  UFUNCTION(BlueprintCallable, Category = "Locomotion")
  void ApplyBrake(float BrakeValue);

  UFUNCTION(BlueprintCallable, Category = "Locomotion")
  void ResetMovement();

  // State queries
  UFUNCTION(BlueprintPure, Category = "Locomotion")
  float GetCurrentSpeed() const { return CurrentSpeed; }

  UFUNCTION(BlueprintPure, Category = "Locomotion")
  float GetCurrentSpeedKmH() const {
    return CurrentSpeed * 0.036f;
  } // cm/s to km/h

  UFUNCTION(BlueprintPure, Category = "Locomotion")
  float GetDistanceAlongSpline() const { return DistanceAlongSpline; }

  UFUNCTION(BlueprintPure, Category = "Locomotion")
  bool IsMoving() const { return FMath::Abs(CurrentSpeed) > 1.0f; }

protected:
  // Rail spline reference (weak pointer to prevent dangling references)
  UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
  TWeakObjectPtr<ARailSplineActor> CurrentRailSpline;

  // Cached spline component for performance
  UPROPERTY()
  TWeakObjectPtr<USplineComponent> CachedSplineComponent;

  // Movement parameters
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Physics")
  float MaxSpeed;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Physics")
  float Acceleration;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Physics")
  float Deceleration;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Physics")
  float BrakingForce;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Physics")
  float Mass;

  // Inertia system
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Physics")
  float InertiaDamping;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Physics")
  float FrictionCoefficient;

  // Current state
  UPROPERTY(BlueprintReadOnly, Category = "Locomotion|State")
  float CurrentSpeed;

  UPROPERTY(BlueprintReadOnly, Category = "Locomotion|State")
  float TargetSpeed;

  UPROPERTY(BlueprintReadOnly, Category = "Locomotion|State")
  float DistanceAlongSpline;

  UPROPERTY(BlueprintReadOnly, Category = "Locomotion|State")
  float CurrentThrottle;

  UPROPERTY(BlueprintReadOnly, Category = "Locomotion|State")
  float CurrentBrake;

  // Debug visualization
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Debug")
  bool bShowDebugInfo;

private:
  // Internal update methods
  void UpdateMovement(float DeltaTime);
  void UpdatePhysics(float DeltaTime);
  void UpdatePositionOnSpline(float DeltaTime);
  void ApplyInertia(float DeltaTime);
  void ApplyFriction(float DeltaTime);

  // Utility methods
  bool ValidateSpline() const;
  void DrawDebugInfo() const;

  // Cache for performance optimization
  float CachedSplineLength;
  bool bSplineCacheValid;

  void InvalidateSplineCache();
  void UpdateSplineCache();
};
