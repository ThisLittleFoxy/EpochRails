// RailTrainMovementComponent.cpp

#include "RailTrainMovementComponent.h"
#include "TrainConfigDataAsset.h"
#include "RollingStockActor.h"

#include "Components/SplineComponent.h"

URailTrainMovementComponent::URailTrainMovementComponent()
{
	// Tick in PrePhysics to update before character movement
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void URailTrainMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URailTrainMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!TrackSpline)
	{
		return;
	}

	// Update physics (speed and distance)
	UpdatePhysics(DeltaTime);

	// Update all car transforms
	UpdateCarTransforms();

	// Detect stop event
	bool bIsMovingNow = IsMoving();
	if (bWasMovingLastFrame && !bIsMovingNow)
	{
		OnTrainStopped.Broadcast();
	}
	bWasMovingLastFrame = bIsMovingNow;

	// Broadcast speed changes (with threshold to avoid spam)
	float CurrentSpeed = GetSpeed();
	if (FMath::Abs(CurrentSpeed - LastBroadcastSpeed) > 10.0f) // 10 cm/s threshold
	{
		OnSpeedChanged.Broadcast(CurrentSpeed);
		LastBroadcastSpeed = CurrentSpeed;
	}
}

// ===== Configuration =====

void URailTrainMovementComponent::SetTrackSpline(USplineComponent* InSpline)
{
	TrackSpline = InSpline;
}

void URailTrainMovementComponent::SetConfig(const UTrainConfigDataAsset* InConfig)
{
	Config = InConfig;

	if (Config)
	{
		// Cache values for quick access
		MaxSpeed = Config->MaxSpeed;
		MaxReverseSpeed = Config->MaxReverseSpeed;
		Acceleration = Config->Acceleration;
		BrakeDeceleration = Config->BrakeDeceleration;
		EmergencyBrakeDeceleration = Config->EmergencyBrakeDeceleration;
		NaturalDeceleration = Config->NaturalDeceleration;
		LocomotiveHalfLength = Config->LocomotiveLength * 0.5f;
	}
}

void URailTrainMovementComponent::SetCars(const TArray<ARollingStockActor*>& InCars)
{
	Cars.Empty(InCars.Num());
	for (ARollingStockActor* Car : InCars)
	{
		Cars.Add(Car);
	}
}

// ===== Control API =====

void URailTrainMovementComponent::SetThrottle(float ThrottleValue)
{
	CurrentThrottle = FMath::Clamp(ThrottleValue, 0.0f, 1.0f);
}

void URailTrainMovementComponent::SetBrake(float BrakeValue)
{
	CurrentBrake = FMath::Clamp(BrakeValue, 0.0f, 1.0f);
}

void URailTrainMovementComponent::ToggleReverse()
{
	SetReverse(!bIsReversing);
}

void URailTrainMovementComponent::SetReverse(bool bReverse)
{
	if (bIsReversing != bReverse)
	{
		bIsReversing = bReverse;
		OnDirectionChanged.Broadcast(bIsReversing);
	}
}

void URailTrainMovementComponent::EmergencyBrake()
{
	if (!bEmergencyBrake)
	{
		bEmergencyBrake = true;
		OnEmergencyBrakeActivated.Broadcast();
	}
}

void URailTrainMovementComponent::ReleaseEmergencyBrake()
{
	bEmergencyBrake = false;
}

// ===== Physics Update =====

void URailTrainMovementComponent::UpdatePhysics(float DeltaTime)
{
	// Calculate acceleration
	float Accel = CalculateAcceleration();

	// Apply acceleration
	SpeedSigned += Accel * DeltaTime;

	// Determine max speed based on direction
	float EffectiveMaxSpeed = bIsReversing ? MaxReverseSpeed : MaxSpeed;

	// Clamp speed
	if (bIsReversing)
	{
		// In reverse mode, we move in negative direction
		SpeedSigned = FMath::Clamp(SpeedSigned, -EffectiveMaxSpeed, 0.0f);
	}
	else
	{
		// In forward mode, we move in positive direction
		SpeedSigned = FMath::Clamp(SpeedSigned, 0.0f, EffectiveMaxSpeed);
	}

	// If speed is very small and no throttle, stop completely
	if (FMath::Abs(SpeedSigned) < 1.0f && CurrentThrottle < KINDA_SMALL_NUMBER)
	{
		SpeedSigned = 0.0f;
	}

	// Update distance
	TrainDistance += SpeedSigned * DeltaTime;

	// Clamp to spline bounds
	ClampDistanceToSpline();
}

float URailTrainMovementComponent::CalculateAcceleration() const
{
	float Accel = 0.0f;

	// Emergency brake overrides everything
	if (bEmergencyBrake)
	{
		// Decelerate toward zero
		if (SpeedSigned > 0.0f)
		{
			return -EmergencyBrakeDeceleration;
		}
		else if (SpeedSigned < 0.0f)
		{
			return EmergencyBrakeDeceleration;
		}
		return 0.0f;
	}

	// Throttle acceleration
	if (CurrentThrottle > KINDA_SMALL_NUMBER)
	{
		if (bIsReversing)
		{
			// Accelerate in negative direction
			Accel = -Acceleration * CurrentThrottle;
		}
		else
		{
			// Accelerate in positive direction
			Accel = Acceleration * CurrentThrottle;
		}
	}

	// Brake deceleration
	if (CurrentBrake > KINDA_SMALL_NUMBER)
	{
		float BrakeForce = BrakeDeceleration * CurrentBrake;
		if (SpeedSigned > 0.0f)
		{
			Accel -= BrakeForce;
		}
		else if (SpeedSigned < 0.0f)
		{
			Accel += BrakeForce;
		}
	}

	// Natural deceleration (friction) when no throttle
	if (CurrentThrottle < KINDA_SMALL_NUMBER && FMath::Abs(SpeedSigned) > KINDA_SMALL_NUMBER)
	{
		if (SpeedSigned > 0.0f)
		{
			Accel -= NaturalDeceleration;
		}
		else
		{
			Accel += NaturalDeceleration;
		}
	}

	return Accel;
}

void URailTrainMovementComponent::ClampDistanceToSpline()
{
	if (!TrackSpline)
	{
		return;
	}

	float SplineLength = TrackSpline->GetSplineLength();

	// Clamp to valid range
	if (TrainDistance < 0.0f)
	{
		TrainDistance = 0.0f;
		SpeedSigned = 0.0f; // Stop at beginning
	}
	else if (TrainDistance > SplineLength)
	{
		TrainDistance = SplineLength;
		SpeedSigned = 0.0f; // Stop at end
	}
}

// ===== Transform Updates =====

void URailTrainMovementComponent::UpdateCarTransforms()
{
	if (!TrackSpline)
	{
		return;
	}

	// Update owning actor (locomotive) transform
	AActor* Owner = GetOwner();
	if (Owner)
	{
		FTransform LocoTransform = GetTransformAtDistance(TrainDistance, true);
		Owner->SetActorTransform(LocoTransform);
	}

	// Update all cars
	for (const TWeakObjectPtr<ARollingStockActor>& WeakCar : Cars)
	{
		if (ARollingStockActor* Car = WeakCar.Get())
		{
			// Car distance = train distance - car offset
			float CarDistance = TrainDistance - Car->DistanceOffset;
			CarDistance = FMath::Max(0.0f, CarDistance);

			FTransform CarTransform = GetTransformAtDistance(CarDistance, true);
			Car->SetActorTransform(CarTransform);
		}
	}
}

FTransform URailTrainMovementComponent::GetTransformAtDistance(float Distance, bool bForwardOrientation) const
{
	if (!TrackSpline)
	{
		return FTransform::Identity;
	}

	// Clamp distance
	float SplineLength = TrackSpline->GetSplineLength();
	Distance = FMath::Clamp(Distance, 0.0f, SplineLength);

	// Get location
	FVector Location = TrackSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

	// Get rotation - always forward along spline
	// This ensures locomotive "looks forward" even when reversing
	FRotator Rotation = TrackSpline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

	// Note: We do NOT flip rotation when reversing
	// The locomotive stays oriented forward on the track

	return FTransform(Rotation, Location);
}

FVector URailTrainMovementComponent::GetCurrentTangent() const
{
	if (!TrackSpline)
	{
		return FVector::ForwardVector;
	}

	FVector Tangent = TrackSpline->GetDirectionAtDistanceAlongSpline(TrainDistance, ESplineCoordinateSpace::World);

	// If we're reversing and moving, the actual movement direction is opposite
	if (bIsReversing && SpeedSigned < 0.0f)
	{
		Tangent = -Tangent;
	}

	return Tangent;
}
