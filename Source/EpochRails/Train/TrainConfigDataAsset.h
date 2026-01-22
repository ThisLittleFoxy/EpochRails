// TrainConfigDataAsset.h
// Data asset for train configuration - allows designers to tune parameters without code changes

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TrainConfigDataAsset.generated.h"

/**
 * Configuration data asset for train parameters.
 * Allows designers to configure train composition, movement, and building settings.
 */
UCLASS(BlueprintType)
class EPOCHRAILS_API UTrainConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// ===== Composition =====

	/** Number of wagons/cars to spawn (excluding locomotive) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Composition", meta = (ClampMin = "0", ClampMax = "50"))
	int32 WagonCount = 3;

	/** Gap between coupled vehicles in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Composition", meta = (ClampMin = "10.0", ClampMax = "200.0"))
	float CouplingGap = 50.0f;

	/** Length of each wagon in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Composition", meta = (ClampMin = "100.0"))
	float WagonLength = 400.0f;

	/** Width of each wagon in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Composition", meta = (ClampMin = "100.0"))
	float WagonWidth = 200.0f;

	/** Length of the locomotive in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Composition", meta = (ClampMin = "100.0"))
	float LocomotiveLength = 600.0f;

	// ===== Movement =====

	/** Maximum forward speed in cm/s (200 km/h = 5555.56 cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float MaxSpeed = 5555.56f;

	/** Maximum reverse speed in cm/s (typically lower than forward) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float MaxReverseSpeed = 2777.78f;

	/** Acceleration rate in cm/s^2 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float Acceleration = 200.0f;

	/** Deceleration rate when braking in cm/s^2 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float BrakeDeceleration = 400.0f;

	/** Deceleration rate for emergency brake in cm/s^2 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float EmergencyBrakeDeceleration = 1000.0f;

	/** Natural deceleration when no throttle/brake applied (friction) in cm/s^2 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float NaturalDeceleration = 50.0f;

	// ===== Building =====

	/** Grid cell size for snap-to-grid building mode in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building", meta = (ClampMin = "10.0"))
	float BuildGridSize = 50.0f;

	/** Maximum height for structures built on wagons in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building", meta = (ClampMin = "50.0"))
	float MaxBuildHeight = 300.0f;

	/** Rotation snap angle in degrees for grid building mode */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building", meta = (ClampMin = "1.0", ClampMax = "90.0"))
	float BuildRotationSnap = 15.0f;

	// ===== Visual/Debug =====

	/** Color for the track spline visualization */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	FLinearColor SplineDebugColor = FLinearColor(0.8f, 0.4f, 0.0f, 1.0f);

	/** Whether to show debug visualization in game */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	bool bShowDebugInGame = false;
};
