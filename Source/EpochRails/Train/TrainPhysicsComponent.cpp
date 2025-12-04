// Copyright Epic Games, Inc. All Rights Reserved.

#include "TrainPhysicsComponent.h"
#include "Kismet/KismetMathLibrary.h"

UTrainPhysicsComponent::UTrainPhysicsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UTrainPhysicsComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize total mass
	UpdateTotalMass();
}

void UTrainPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DeltaTime > 0.0f)
	{
		// Update physics simulation
		UpdatePhysics(DeltaTime);
	}
}

void UTrainPhysicsComponent::SetThrottle(float NewThrottle)
{
	CurrentThrottle = FMath::Clamp(NewThrottle, 0.0f, 1.0f);
}

void UTrainPhysicsComponent::SetBrake(float NewBrake)
{
	CurrentBrake = FMath::Clamp(NewBrake, 0.0f, 1.0f);
}

void UTrainPhysicsComponent::SetTrackGrade(float GradeDegrees)
{
	CurrentGradeDegrees = FMath::Clamp(GradeDegrees, -45.0f, 45.0f);
}

void UTrainPhysicsComponent::SetTrackCurvature(float Curvature)
{
	CurrentCurvature = FMath::Clamp(Curvature, 0.0f, 1.0f);
}

float UTrainPhysicsComponent::GetVelocityKmh() const
{
	// Convert m/s to km/h
	return PhysicsState.Velocity * 3.6f;
}

void UTrainPhysicsComponent::AddWagons(int32 Count)
{
	if (Count > 0)
	{
		PhysicsParameters.WagonCount += Count;
		UpdateTotalMass();
	}
}

void UTrainPhysicsComponent::RemoveWagons(int32 Count)
{
	if (Count > 0)
	{
		PhysicsParameters.WagonCount = FMath::Max(0, PhysicsParameters.WagonCount - Count);
		UpdateTotalMass();
	}
}

void UTrainPhysicsComponent::SetWagonMass(float NewWagonMass)
{
	if (NewWagonMass > 0.0f)
	{
		PhysicsParameters.WagonMass = NewWagonMass;
		UpdateTotalMass();
	}
}

void UTrainPhysicsComponent::EmergencyBrake()
{
	CurrentThrottle = 0.0f;
	CurrentBrake = 1.0f;
}

void UTrainPhysicsComponent::ResetPhysics()
{
	PhysicsState.Velocity = 0.0f;
	PhysicsState.Acceleration = 0.0f;
	PhysicsState.DistanceTraveled = 0.0f;
	CurrentThrottle = 0.0f;
	CurrentBrake = 0.0f;
}

float UTrainPhysicsComponent::CalculateStoppingDistance() const
{
	if (PhysicsState.Velocity <= 0.0f)
	{
		return 0.0f;
	}

	// Using kinematic equation: d = v² / (2 * a)
	// where a is deceleration from maximum braking
	float MaxDeceleration = PhysicsParameters.MaxBrakingForce / PhysicsState.TotalMass;
	
	// Add rolling and air resistance to braking
	float TotalDeceleration = MaxDeceleration + 
		(PhysicsState.RollingResistance + PhysicsState.AirDragResistance) / PhysicsState.TotalMass;

	return (PhysicsState.Velocity * PhysicsState.Velocity) / (2.0f * TotalDeceleration);
}

void UTrainPhysicsComponent::UpdateTotalMass()
{
	PhysicsState.TotalMass = PhysicsParameters.LocomotiveMass + 
		(PhysicsParameters.WagonMass * PhysicsParameters.WagonCount);
}

void UTrainPhysicsComponent::CalculateResistanceForces()
{
	PhysicsState.RollingResistance = CalculateRollingResistance();
	PhysicsState.AirDragResistance = CalculateAirDrag();
	PhysicsState.GradeResistance = CalculateGradeResistance();
	PhysicsState.CurveResistance = CalculateCurveResistance();
	
	PhysicsState.TotalResistance = PhysicsState.RollingResistance + 
		PhysicsState.AirDragResistance + 
		PhysicsState.GradeResistance + 
		PhysicsState.CurveResistance;
}

float UTrainPhysicsComponent::CalculateRollingResistance() const
{
	// Rolling resistance: Fr = Crr * m * g * cos(grade)
	float GradeRadians = FMath::DegreesToRadians(CurrentGradeDegrees);
	float CosGrade = FMath::Cos(GradeRadians);
	
	return PhysicsParameters.RollingResistanceCoefficient * 
		PhysicsState.TotalMass * 
		PhysicsParameters.Gravity * 
		CosGrade;
}

float UTrainPhysicsComponent::CalculateAirDrag() const
{
	// Air drag: Fd = 0.5 * ρ * Cd * A * v²
	// Simplified: Fd = k * v², where k = 0.5 * ρ * Cd * A
	float VelocitySquared = PhysicsState.Velocity * PhysicsState.Velocity;
	
	return 0.5f * PhysicsParameters.AirDensity * 
		PhysicsParameters.AirDragCoefficient * 
		VelocitySquared;
}

float UTrainPhysicsComponent::CalculateGradeResistance() const
{
	// Grade resistance: Fg = m * g * sin(grade)
	float GradeRadians = FMath::DegreesToRadians(CurrentGradeDegrees);
	float SinGrade = FMath::Sin(GradeRadians);
	
	return PhysicsState.TotalMass * PhysicsParameters.Gravity * SinGrade;
}

float UTrainPhysicsComponent::CalculateCurveResistance() const
{
	// Curve resistance increases with curvature and velocity
	// Simplified model: Fc = k * curvature * m * v
	if (CurrentCurvature > 0.0f)
	{
		return PhysicsParameters.CurveResistanceFactor * 
			CurrentCurvature * 
			PhysicsState.TotalMass * 
			PhysicsState.Velocity * 0.01f; // Scale factor
	}
	return 0.0f;
}

float UTrainPhysicsComponent::CalculateTractiveForce() const
{
	if (CurrentThrottle <= 0.0f)
	{
		return 0.0f;
	}

	// Tractive force reduces with velocity (power = force * velocity)
	// At low speeds: maximum force
	// At high speeds: force = power / velocity
	float MaxForce = PhysicsParameters.MaxTractiveForce * CurrentThrottle;
	
	// Simple power limit curve
	// Assuming constant power above certain speed
	const float TransitionSpeed = 10.0f; // m/s (~36 km/h)
	
	if (PhysicsState.Velocity < TransitionSpeed)
	{
		return MaxForce;
	}
	else
	{
		// Power = Force * Velocity, so Force = Power / Velocity
		float ConstantPower = MaxForce * TransitionSpeed;
		return ConstantPower / PhysicsState.Velocity;
	}
}

float UTrainPhysicsComponent::CalculateBrakingForce() const
{
	return PhysicsParameters.MaxBrakingForce * CurrentBrake;
}

bool UTrainPhysicsComponent::CheckWheelSlip(float NetForce) const
{
	// Check if applied force exceeds adhesion limit
	// Maximum adhesion force: Fa = μ * m * g * cos(grade)
	float GradeRadians = FMath::DegreesToRadians(CurrentGradeDegrees);
	float CosGrade = FMath::Cos(GradeRadians);
	
	float MaxAdhesionForce = PhysicsParameters.AdhesionCoefficient * 
		PhysicsState.TotalMass * 
		PhysicsParameters.Gravity * 
		CosGrade;
	
	return FMath::Abs(NetForce) > MaxAdhesionForce;
}

void UTrainPhysicsComponent::UpdatePhysics(float DeltaTime)
{
	// Calculate all resistance forces
	CalculateResistanceForces();
	
	// Calculate applied forces
	PhysicsState.AppliedTractiveForce = CalculateTractiveForce();
	PhysicsState.AppliedBrakingForce = CalculateBrakingForce();
	
	// Net force: F_net = F_tractive - F_braking - F_resistance
	float NetForce = PhysicsState.AppliedTractiveForce - 
		PhysicsState.AppliedBrakingForce - 
		PhysicsState.TotalResistance;
	
	// Check for wheel slip
	PhysicsState.bIsWheelSlipping = CheckWheelSlip(NetForce);
	
	if (PhysicsState.bIsWheelSlipping)
	{
		// Reduce effective force when slipping
		NetForce *= 0.5f;
	}
	
	// Calculate acceleration: F = m * a, so a = F / m
	PhysicsState.Acceleration = NetForce / PhysicsState.TotalMass;
	
	// Update velocity: v = v0 + a * dt
	float NewVelocity = PhysicsState.Velocity + (PhysicsState.Acceleration * DeltaTime);
	
	// Prevent negative velocity (train can't go backwards without reverse gear)
	NewVelocity = FMath::Max(0.0f, NewVelocity);
	
	// Update distance traveled
	if (NewVelocity > 0.0f)
	{
		PhysicsState.DistanceTraveled += NewVelocity * DeltaTime;
	}
	
	PhysicsState.Velocity = NewVelocity;
}

float UTrainPhysicsComponent::SmoothInput(float Current, float Target, float DeltaTime, float SmoothingSpeed)
{
	return FMath::FInterpTo(Current, Target, DeltaTime, SmoothingSpeed);
}
