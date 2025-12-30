// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrainPhysicsComponent.generated.h"

/**
 * Physics simulation parameters for realistic train behavior
 */
/**
 * Simplified physics parameters for train behavior
 */
USTRUCT(BlueprintType)
struct FTrainPhysicsParameters
{
    GENERATED_BODY()

    // ========== Engine Settings ==========
    
    /** Engine power in kilowatts (kW) - affects acceleration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Engine",
        meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float EnginePowerKW = 500.0f;
    
    /** Maximum speed in km/h */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Engine",
        meta = (ClampMin = "10.0", ClampMax = "200.0"))
    float MaxSpeedKmh = 60.0f;
    
    /** Acceleration rate multiplier (higher = faster acceleration) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Engine",
        meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float AccelerationMultiplier = 1.0f;
    
    /** Braking power multiplier (higher = faster braking) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Engine",
        meta = (ClampMin = "0.5", ClampMax = "3.0"))
    float BrakingMultiplier = 1.5f;

	// ========== Gear System ==========

    /** Current gear (0 = neutral, 1+ = forward gears) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Gears")
    int32 CurrentGear = 0;

    /** Maximum number of gears */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Gears",
              meta = (ClampMin = "1", ClampMax = "10"))
    int32 MaxGears = 3;

    /** Maximum speed for each gear in km/h */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Gears")
    TArray<float> GearMaxSpeedsKmh = {30.0f, 60.0f, 100.0f};

    /** Acceleration multiplier for each gear (higher = faster in that gear) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Gears")
    TArray<float> GearAccelerationMultipliers = {1.5f, 1.2f, 1.0f};

	// ========== Direction Control ==========

    /** Direction multiplier (1.0 = forward, -1.0 = reverse) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Direction")
    float DirectionMultiplier = 1.0f;

    /** Maximum speed in reverse as percentage of forward speed (0.0-1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Direction",
              meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float ReverseSpeedLimitPercent = 0.25f;

    // ========== Train Configuration ==========
    
    /** Mass of the locomotive in kg */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Mass",
        meta = (ClampMin = "10000.0", ClampMax = "200000.0"))
    float LocomotiveMass = 50000.0f;
    
    /** Mass per wagon in kg */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Mass",
        meta = (ClampMin = "5000.0", ClampMax = "100000.0"))
    float WagonMass = 20000.0f;
    
    /** Number of wagons attached */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Mass")
    int32 WagonCount = 0;

    // ========== Resistance Settings (optional tuning) ==========
    
    /** Rolling resistance coefficient (0.001-0.003) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Advanced",
        meta = (ClampMin = "0.0005", ClampMax = "0.01"))
    float RollingResistance = 0.002f;
    
    /** Air drag coefficient (affects high-speed resistance) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Advanced",
        meta = (ClampMin = "1.0", ClampMax = "20.0"))
    float AirDragCoefficient = 5.0f;
    
    /** Gravity constant */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Advanced")
    float Gravity = 9.81f;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Physics|Debug")
        bool bEnableDebugLogs = false;

    /** Set current gear (0 = neutral, 1+ = gears) */
    UFUNCTION(BlueprintCallable, Category = "Train Physics")
        void SetGear(int32 NewGear);
    UFUNCTION(BlueprintPure, Category = "Train Physics")
    int32 GetCurrentGear() const { return PhysicsParameters.CurrentGear; }
    /** Get max speed for current gear */
    UFUNCTION(BlueprintPure, Category = "Train Physics")
    float GetCurrentGearMaxSpeed() const;

    /** Get acceleration multiplier for current gear */
    UFUNCTION(BlueprintPure, Category = "Train Physics")
    float GetCurrentGearAccelMultiplier() const;

	/** Set direction (1.0 = forward, -1.0 = reverse) */
    UFUNCTION(BlueprintCallable, Category = "Train Physics")
    void SetDirection(float Direction);

    /** Get current direction */
    UFUNCTION(BlueprintPure, Category = "Train Physics")
    float GetDirection() const { return PhysicsParameters.DirectionMultiplier; }

	// Fixed-step simulation
    float PhysicsAccumulator = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Train Physics|Substepping",
              meta = (ClampMin = "0.001", ClampMax = "0.05"))
    float FixedStepSeconds = 1.0f / 60.0f;

    UPROPERTY(EditAnywhere, Category = "Train Physics|Substepping",
              meta = (ClampMin = "1", ClampMax = "32"))
    int32 MaxSubstepsPerTick = 8;
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

	/** Internal absolute velocity (always positive) */
    float AbsoluteVelocity = 0.0f;

};
