// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrainPhysicsComponent.generated.h"

/**
 * Physics simulation parameters for realistic train behavior
 */
USTRUCT(BlueprintType)
struct FTrainPhysicsParameters
{
	GENERATED_BODY()

	// Mass of the locomotive in kilograms (e.g., 80000 kg for average locomotive)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Mass")
	float LocomotiveMass = 80000.0f;

	// Mass per wagon in kilograms (e.g., 20000 kg empty, 80000 kg loaded)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Mass")
	float WagonMass = 50000.0f;

	// Number of wagons attached to locomotive
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Mass")
	int32 WagonCount = 0;

	// Maximum tractive force in Newtons (e.g., 400000 N for modern electric locomotive)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Power")
	float MaxTractiveForce = 400000.0f;

	// Maximum braking force in Newtons
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Power")
	float MaxBrakingForce = 600000.0f;

	// Rolling resistance coefficient (typical: 0.001-0.002)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Resistance")
	float RollingResistanceCoefficient = 0.0015f;

	// Air drag coefficient (Cd * A, typical: 5-10 for trains)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Resistance")
	float AirDragCoefficient = 7.0f;

	// Curve resistance factor (additional resistance on curves)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Resistance")
	float CurveResistanceFactor = 0.5f;

	// Adhesion coefficient (wheel-rail friction, typical: 0.25-0.35)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Adhesion")
	float AdhesionCoefficient = 0.30f;

	// Gravity constant (m/s²)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Environment")
	float Gravity = 9.81f;

	// Air density in kg/m³ (sea level: 1.225)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Environment")
	float AirDensity = 1.225f;
};

/**
 * Current state of train physics simulation
 */
USTRUCT(BlueprintType)
struct FTrainPhysicsState
{
	GENERATED_BODY()

	// Current velocity in m/s
	UPROPERTY(BlueprintReadOnly, Category = "Physics State")
	float Velocity = 0.0f;

	// Current acceleration in m/s²
	UPROPERTY(BlueprintReadOnly, Category = "Physics State")
	float Acceleration = 0.0f;

	// Total mass of train (locomotive + wagons) in kg
	UPROPERTY(BlueprintReadOnly, Category = "Physics State")
	float TotalMass = 0.0f;

	// Current applied tractive force in N
	UPROPERTY(BlueprintReadOnly, Category = "Physics State")
	float AppliedTractiveForce = 0.0f;

	// Current applied braking force in N
	UPROPERTY(BlueprintReadOnly, Category = "Physics State")
	float AppliedBrakingForce = 0.0f;

	// Total resistance force in N
	UPROPERTY(BlueprintReadOnly, Category = "Physics State")
	float TotalResistance = 0.0f;

	// Rolling resistance in N
	UPROPERTY(BlueprintReadOnly, Category = "Physics State")
	float RollingResistance = 0.0f;

	// Air drag resistance in N
	UPROPERTY(BlueprintReadOnly, Category = "Physics State")
	float AirDragResistance = 0.0f;

	// Grade resistance in N (from slope)
	UPROPERTY(BlueprintReadOnly, Category = "Physics State")
	float GradeResistance = 0.0f;

	// Curve resistance in N
	UPROPERTY(BlueprintReadOnly, Category = "Physics State")
	float CurveResistance = 0.0f;

	// Whether wheels are slipping (loss of adhesion)
	UPROPERTY(BlueprintReadOnly, Category = "Physics State")
	bool bIsWheelSlipping = false;

	// Distance traveled in meters
	UPROPERTY(BlueprintReadOnly, Category = "Physics State")
	float DistanceTraveled = 0.0f;
};

/**
 * Component that simulates realistic train physics
 * Handles mass, forces, resistance, adhesion, and slope effects
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EPOCHRAILS_API UTrainPhysicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTrainPhysicsComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Physics parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Physics")
	FTrainPhysicsParameters PhysicsParameters;

	// Current physics state (read-only)
	UPROPERTY(BlueprintReadOnly, Category = "Train Physics")
	FTrainPhysicsState PhysicsState;

	/**
	 * Set throttle input (0.0 to 1.0)
	 * 0.0 = no power, 1.0 = maximum power
	 */
	UFUNCTION(BlueprintCallable, Category = "Train Physics")
	void SetThrottle(float NewThrottle);

	/**
	 * Set brake input (0.0 to 1.0)
	 * 0.0 = no braking, 1.0 = maximum braking
	 */
	UFUNCTION(BlueprintCallable, Category = "Train Physics")
	void SetBrake(float NewBrake);

	/**
	 * Set track grade (slope) in degrees
	 * Positive = uphill, Negative = downhill
	 */
	UFUNCTION(BlueprintCallable, Category = "Train Physics")
	void SetTrackGrade(float GradeDegrees);

	/**
	 * Set track curvature (0.0 = straight, 1.0 = tight curve)
	 */
	UFUNCTION(BlueprintCallable, Category = "Train Physics")
	void SetTrackCurvature(float Curvature);

	/**
	 * Get current velocity in km/h
	 */
	UFUNCTION(BlueprintPure, Category = "Train Physics")
	float GetVelocityKmh() const;

	/**
	 * Get current velocity in m/s
	 */
	UFUNCTION(BlueprintPure, Category = "Train Physics")
	float GetVelocityMs() const { return PhysicsState.Velocity; }

	/**
	 * Add wagons to the train
	 */
	UFUNCTION(BlueprintCallable, Category = "Train Physics")
	void AddWagons(int32 Count);

	/**
	 * Remove wagons from the train
	 */
	UFUNCTION(BlueprintCallable, Category = "Train Physics")
	void RemoveWagons(int32 Count);

	/**
	 * Set wagon mass (useful for loaded/unloaded wagons)
	 */
	UFUNCTION(BlueprintCallable, Category = "Train Physics")
	void SetWagonMass(float NewWagonMass);

	/**
	 * Emergency brake (maximum braking force applied immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "Train Physics")
	void EmergencyBrake();

	/**
	 * Reset physics simulation (stop train, clear state)
	 */
	UFUNCTION(BlueprintCallable, Category = "Train Physics")
	void ResetPhysics();

	/**
	 * Calculate stopping distance at current velocity
	 */
	UFUNCTION(BlueprintPure, Category = "Train Physics")
	float CalculateStoppingDistance() const;

protected:
	virtual void BeginPlay() override;

private:
	// Input values
	float CurrentThrottle = 0.0f;
	float CurrentBrake = 0.0f;
	float CurrentGradeDegrees = 0.0f;
	float CurrentCurvature = 0.0f;

	// Calculate total mass of train
	void UpdateTotalMass();

	// Calculate all resistance forces
	void CalculateResistanceForces();

	// Calculate rolling resistance (depends on mass and coefficient)
	float CalculateRollingResistance() const;

	// Calculate air drag (depends on velocity squared)
	float CalculateAirDrag() const;

	// Calculate grade resistance from slope
	float CalculateGradeResistance() const;

	// Calculate curve resistance
	float CalculateCurveResistance() const;

	// Calculate tractive force based on throttle
	float CalculateTractiveForce() const;

	// Calculate braking force based on brake input
	float CalculateBrakingForce() const;

	// Check for wheel slip (adhesion limit)
	bool CheckWheelSlip(float NetForce) const;

	// Apply forces and update velocity
	void UpdatePhysics(float DeltaTime);

	// Smooth throttle/brake transitions
	float SmoothInput(float Current, float Target, float DeltaTime, float SmoothingSpeed = 2.0f);
};
