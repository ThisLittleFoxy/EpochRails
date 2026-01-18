// RailTrainMovementComponent.h
// Component responsible for train movement along spline with reverse support

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RailTrainMovementComponent.generated.h"

class USplineComponent;
class UTrainConfigDataAsset;
class ARollingStockActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrainSpeedChanged, float, NewSpeed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrainDirectionChanged, bool, bIsReversing);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTrainStopped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEmergencyBrakeActivated);

/**
 * Movement component for trains moving along a spline.
 * Handles throttle, braking, reverse movement, and car positioning.
 *
 * Key concepts:
 * - TrainDistance: Position of locomotive along the spline (cm)
 * - SpeedSigned: Positive = forward, negative = reverse movement
 * - Reverse mode: Train moves backward but orientation stays "forward looking"
 */
UCLASS(ClassGroup = (Train), meta = (BlueprintSpawnableComponent))
class EPOCHRAILS_API URailTrainMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URailTrainMovementComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===== Configuration =====

	/** Set the spline to move along */
	UFUNCTION(BlueprintCallable, Category = "Train|Movement")
	void SetTrackSpline(USplineComponent* InSpline);

	/** Set the configuration data asset */
	UFUNCTION(BlueprintCallable, Category = "Train|Movement")
	void SetConfig(const UTrainConfigDataAsset* InConfig);

	/** Set the cars array for transform updates */
	void SetCars(const TArray<ARollingStockActor*>& InCars);

	// ===== Control API =====

	/**
	 * Set throttle input.
	 * @param ThrottleValue 0.0 to 1.0 (0 = no throttle, 1 = full power)
	 */
	UFUNCTION(BlueprintCallable, Category = "Train|Control")
	void SetThrottle(float ThrottleValue);

	/**
	 * Set brake input.
	 * @param BrakeValue 0.0 to 1.0 (0 = no brake, 1 = full brake)
	 */
	UFUNCTION(BlueprintCallable, Category = "Train|Control")
	void SetBrake(float BrakeValue);

	/**
	 * Toggle reverse mode.
	 * When in reverse, positive throttle moves train backward.
	 */
	UFUNCTION(BlueprintCallable, Category = "Train|Control")
	void ToggleReverse();

	/**
	 * Set reverse mode directly.
	 */
	UFUNCTION(BlueprintCallable, Category = "Train|Control")
	void SetReverse(bool bReverse);

	/**
	 * Activate emergency brake - immediate maximum braking.
	 */
	UFUNCTION(BlueprintCallable, Category = "Train|Control")
	void EmergencyBrake();

	/**
	 * Release emergency brake.
	 */
	UFUNCTION(BlueprintCallable, Category = "Train|Control")
	void ReleaseEmergencyBrake();

	// ===== State Queries =====

	/** Get current speed (always positive) in cm/s */
	UFUNCTION(BlueprintCallable, Category = "Train|State")
	float GetSpeed() const { return FMath::Abs(SpeedSigned); }

	/** Get speed in km/h (always positive) */
	UFUNCTION(BlueprintCallable, Category = "Train|State")
	float GetSpeedKmh() const { return FMath::Abs(SpeedSigned) * 0.036f; }

	/** Get signed speed (positive = forward, negative = reversing) */
	UFUNCTION(BlueprintCallable, Category = "Train|State")
	float GetSpeedSigned() const { return SpeedSigned; }

	/** Get current position along spline */
	UFUNCTION(BlueprintCallable, Category = "Train|State")
	float GetTrainDistance() const { return TrainDistance; }

	/** Check if train is in reverse mode */
	UFUNCTION(BlueprintCallable, Category = "Train|State")
	bool IsReversing() const { return bIsReversing; }

	/** Check if train is moving */
	UFUNCTION(BlueprintCallable, Category = "Train|State")
	bool IsMoving() const { return FMath::Abs(SpeedSigned) > KINDA_SMALL_NUMBER; }

	/** Check if emergency brake is active */
	UFUNCTION(BlueprintCallable, Category = "Train|State")
	bool IsEmergencyBrakeActive() const { return bEmergencyBrake; }

	/** Get current throttle value */
	UFUNCTION(BlueprintCallable, Category = "Train|State")
	float GetThrottle() const { return CurrentThrottle; }

	/** Get current brake value */
	UFUNCTION(BlueprintCallable, Category = "Train|State")
	float GetBrakeValue() const { return CurrentBrake; }

	// ===== Events =====

	/** Broadcast when speed changes significantly */
	UPROPERTY(BlueprintAssignable, Category = "Train|Events")
	FOnTrainSpeedChanged OnSpeedChanged;

	/** Broadcast when direction changes */
	UPROPERTY(BlueprintAssignable, Category = "Train|Events")
	FOnTrainDirectionChanged OnDirectionChanged;

	/** Broadcast when train comes to a complete stop */
	UPROPERTY(BlueprintAssignable, Category = "Train|Events")
	FOnTrainStopped OnTrainStopped;

	/** Broadcast when emergency brake is activated */
	UPROPERTY(BlueprintAssignable, Category = "Train|Events")
	FOnEmergencyBrakeActivated OnEmergencyBrakeActivated;

	// ===== Transform Calculation =====

	/**
	 * Get world transform at a distance along the spline.
	 * @param Distance Distance in cm along spline
	 * @param bForwardOrientation If true, orient forward along spline (not reversed for reverse mode)
	 * @return World transform at that distance
	 */
	UFUNCTION(BlueprintCallable, Category = "Train|Movement")
	FTransform GetTransformAtDistance(float Distance, bool bForwardOrientation = true) const;

	/**
	 * Get spline tangent at current train position.
	 * Useful for VFX/animations that need direction.
	 */
	UFUNCTION(BlueprintCallable, Category = "Train|Movement")
	FVector GetCurrentTangent() const;

private:
	/** Update train physics (speed, distance) */
	void UpdatePhysics(float DeltaTime);

	/** Update all car transforms based on current distance */
	void UpdateCarTransforms();

	/** Calculate effective acceleration based on inputs */
	float CalculateAcceleration() const;

	/** Clamp distance to spline bounds with optional looping */
	void ClampDistanceToSpline();

	// ===== State =====

	/** Current position of locomotive along spline in cm */
	UPROPERTY(VisibleInstanceOnly, Category = "Train|Debug")
	float TrainDistance = 0.0f;

	/** Current speed - positive = moving toward spline end, negative = toward start */
	UPROPERTY(VisibleInstanceOnly, Category = "Train|Debug")
	float SpeedSigned = 0.0f;

	/** Current throttle input (0-1) */
	UPROPERTY(VisibleInstanceOnly, Category = "Train|Debug")
	float CurrentThrottle = 0.0f;

	/** Current brake input (0-1) */
	UPROPERTY(VisibleInstanceOnly, Category = "Train|Debug")
	float CurrentBrake = 0.0f;

	/** Is train in reverse mode */
	UPROPERTY(VisibleInstanceOnly, Category = "Train|Debug")
	bool bIsReversing = false;

	/** Is emergency brake active */
	UPROPERTY(VisibleInstanceOnly, Category = "Train|Debug")
	bool bEmergencyBrake = false;

	/** Was train moving last frame (for stop detection) */
	bool bWasMovingLastFrame = false;

	/** Last broadcast speed (to avoid spam) */
	float LastBroadcastSpeed = 0.0f;

	// ===== References =====

	/** The spline we're moving along */
	UPROPERTY()
	TObjectPtr<USplineComponent> TrackSpline;

	/** Configuration data */
	UPROPERTY()
	TObjectPtr<const UTrainConfigDataAsset> Config;

	/** Cars to update transforms for */
	TArray<TWeakObjectPtr<ARollingStockActor>> Cars;

	/** Locomotive length (half) for offset calculation */
	float LocomotiveHalfLength = 300.0f;

	// ===== Cached Config Values =====
	float MaxSpeed = 5555.56f;
	float MaxReverseSpeed = 2777.78f;
	float Acceleration = 200.0f;
	float BrakeDeceleration = 400.0f;
	float EmergencyBrakeDeceleration = 1000.0f;
	float NaturalDeceleration = 50.0f;
};
