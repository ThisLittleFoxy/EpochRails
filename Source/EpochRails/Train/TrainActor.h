// TrainActor.h
// Main train actor containing spline, movement component, and cars

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrainActor.generated.h"

class USplineComponent;
class UStaticMeshComponent;
class USceneComponent;
class URailTrainMovementComponent;
class UTrainConfigDataAsset;
class ARollingStockActor;

/**
 * Main train actor.
 * Contains the track spline, movement component, and manages all cars.
 * The train consists of a locomotive (this actor) and N wagons (ARollingStockActor).
 *
 * Tick synchronization: Ticks in TG_PrePhysics to update before character.
 */
UCLASS(BlueprintType, Blueprintable)
class EPOCHRAILS_API ATrainActor : public AActor
{
	GENERATED_BODY()

public:
	ATrainActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;

	// ===== Components =====

	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	/** Locomotive body mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> LocomotiveMesh;

	/**
	 * Track spline - defines the path the train follows.
	 * Edit this in the editor to create your track layout.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USplineComponent> TrackSpline;

	/** Movement component - handles physics and car transforms */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URailTrainMovementComponent> MovementComponent;

	/** Rear coupler for wagon attachment reference */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> RearCoupler;

	// ===== Configuration =====

	/** Train configuration data asset */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Config")
	TObjectPtr<UTrainConfigDataAsset> Config;

	/** Class to spawn for wagons (can be overridden with BP) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Config")
	TSubclassOf<ARollingStockActor> WagonClass;

	/** Whether to spawn wagons automatically on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Config")
	bool bAutoSpawnWagons = true;

	// ===== Car Management =====

	/** Get all spawned cars */
	UFUNCTION(BlueprintCallable, Category = "Train")
	const TArray<ARollingStockActor*>& GetCars() const { return Cars; }

	/** Get number of cars */
	UFUNCTION(BlueprintCallable, Category = "Train")
	int32 GetCarCount() const { return Cars.Num(); }

	/** Get car by index (0 = first car after locomotive) */
	UFUNCTION(BlueprintCallable, Category = "Train")
	ARollingStockActor* GetCar(int32 Index) const;

	/** Spawn all wagons based on config */
	UFUNCTION(BlueprintCallable, Category = "Train")
	void SpawnCars();

	/** Destroy all spawned wagons */
	UFUNCTION(BlueprintCallable, Category = "Train")
	void DestroyCars();

	/** Add a single wagon at the end */
	UFUNCTION(BlueprintCallable, Category = "Train")
	ARollingStockActor* AddCar();

	/** Remove the last wagon */
	UFUNCTION(BlueprintCallable, Category = "Train")
	void RemoveLastCar();

	// ===== Movement Control (delegates to MovementComponent) =====

	UFUNCTION(BlueprintCallable, Category = "Train|Control")
	void SetThrottle(float ThrottleValue);

	UFUNCTION(BlueprintCallable, Category = "Train|Control")
	void SetBrake(float BrakeValue);

	UFUNCTION(BlueprintCallable, Category = "Train|Control")
	void ToggleReverse();

	UFUNCTION(BlueprintCallable, Category = "Train|Control")
	void SetReverse(bool bReverse);

	UFUNCTION(BlueprintCallable, Category = "Train|Control")
	void EmergencyBrake();

	UFUNCTION(BlueprintCallable, Category = "Train|Control")
	void ReleaseEmergencyBrake();

	// ===== State Queries (delegates to MovementComponent) =====

	UFUNCTION(BlueprintCallable, Category = "Train|State")
	float GetSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Train|State")
	float GetSpeedKmh() const;

	UFUNCTION(BlueprintCallable, Category = "Train|State")
	bool IsReversing() const;

	UFUNCTION(BlueprintCallable, Category = "Train|State")
	bool IsMoving() const;

	UFUNCTION(BlueprintCallable, Category = "Train|State")
	float GetTrainDistance() const;

	// ===== Spline Access =====

	/** Get the track spline component */
	UFUNCTION(BlueprintCallable, Category = "Train")
	USplineComponent* GetTrackSpline() const { return TrackSpline; }

	/** Get spline length in cm */
	UFUNCTION(BlueprintCallable, Category = "Train")
	float GetTrackLength() const;

	/** Get rear coupler component (for external attachments) */
	UFUNCTION(BlueprintCallable, Category = "Train")
	USceneComponent* GetRearCoupler() const { return RearCoupler; }

private:
	/** Calculate distance offset for a car at given index */
	float CalculateCarOffset(int32 CarIndex) const;

	/** Initialize movement component with config */
	void InitializeMovement();

	/** Spawned cars (wagons) */
	UPROPERTY()
	TArray<ARollingStockActor*> Cars;
};
