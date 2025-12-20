// TrainSpeedometerWidget.cpp
#include "TrainSpeedometerWidget.h"
#include "Kismet/KismetMathLibrary.h"

UTrainSpeedometerWidget::UTrainSpeedometerWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Enable ticking for animations
	bIsVariable = true;
}

void UTrainSpeedometerWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Initialize visual speed to current speed (no animation on start)
	VisualSpeed = CurrentSpeed;
	VisualSpeedPercent = CurrentSpeed / FMath::Max(MaxSpeed, 1.0f);
	PreviousVisualSpeed = VisualSpeed;
}

void UTrainSpeedometerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	// Rate limiting
	if (UpdateRate > 0.0f)
	{
		UpdateAccumulator += InDeltaTime;
		if (UpdateAccumulator < UpdateRate)
		{
			return;
		}
		InDeltaTime = UpdateAccumulator;
		UpdateAccumulator = 0.0f;
	}
	
	// Update visual speed with smooth interpolation
	UpdateVisualSpeed(InDeltaTime);
	
	// Update pulse animation
	if (bEnablePulseAnimation)
	{
		UpdatePulseAnimation(InDeltaTime);
	}
	
	// Update warning blink animation
	if (bEnableWarningZone && IsInWarningZone())
	{
		WarningBlinkTimer += InDeltaTime * WarningBlinkSpeed;
	}
	else
	{
		WarningBlinkTimer = 0.0f;
	}
	
	// Check for state changes and fire events
	bool bIsCurrentlyOverspeed = IsOverspeed();
	if (bIsCurrentlyOverspeed != bWasOverspeed)
	{
		if (bIsCurrentlyOverspeed)
		{
			OnOverspeedEnter();
		}
		else
		{
			OnOverspeedExit();
		}
		bWasOverspeed = bIsCurrentlyOverspeed;
	}
	
	bool bIsCurrentlyInWarning = IsInWarningZone();
	if (bIsCurrentlyInWarning != bWasInWarning)
	{
		if (bIsCurrentlyInWarning)
		{
			OnWarningZoneEnter();
		}
		else
		{
			OnWarningZoneExit();
		}
		bWasInWarning = bIsCurrentlyInWarning;
	}
}

void UTrainSpeedometerWidget::UpdateSpeed(float SpeedKmh, float MaxSpeedKmh)
{
	float OldSpeed = CurrentSpeed;
	
	// Apply minimum display threshold
	if (FMath::Abs(SpeedKmh) < MinDisplaySpeed)
	{
		SpeedKmh = 0.0f;
	}
	
	CurrentSpeed = SpeedKmh;
	MaxSpeed = FMath::Max(MaxSpeedKmh, 1.0f);
	
	// Trigger pulse animation on significant change
	if (bEnablePulseAnimation && FMath::Abs(CurrentSpeed - OldSpeed) > 5.0f)
	{
		PulseTimer = 0.0f;
	}
	
	// Fire speed changed event
	if (FMath::Abs(CurrentSpeed - OldSpeed) > 0.1f)
	{
		OnSpeedChanged(CurrentSpeed, OldSpeed);
	}
}

void UTrainSpeedometerWidget::SetMaxSpeed(float NewMaxSpeed)
{
	MaxSpeed = FMath::Max(NewMaxSpeed, 1.0f);
}

void UTrainSpeedometerWidget::SetSpeedImmediate(float SpeedKmh)
{
	CurrentSpeed = SpeedKmh;
	VisualSpeed = SpeedKmh;
	PreviousVisualSpeed = SpeedKmh;
	VisualSpeedPercent = CurrentSpeed / FMath::Max(MaxSpeed, 1.0f);
}

float UTrainSpeedometerWidget::GetDisplaySpeed() const
{
	return ConvertToDisplayUnit(VisualSpeed);
}

FText UTrainSpeedometerWidget::GetSpeedText() const
{
	float DisplaySpeed = ApplyRounding(GetDisplaySpeed());
	
	FNumberFormattingOptions NumberFormat;
	NumberFormat.MinimumFractionalDigits = DecimalPlaces;
	NumberFormat.MaximumFractionalDigits = DecimalPlaces;
	
	if (bShowUnitText)
	{
		return FText::Format(
			FText::FromString(TEXT("{0} {1}")),
			FText::AsNumber(DisplaySpeed, &NumberFormat),
			FText::FromString(GetUnitText())
		);
	}
	else
	{
		return FText::AsNumber(DisplaySpeed, &NumberFormat);
	}
}

FLinearColor UTrainSpeedometerWidget::GetProgressBarColor() const
{
	FLinearColor ResultColor;
	
	switch (ColorMode)
	{
	case ESpeedometerColorMode::Solid:
		ResultColor = SolidColor;
		break;
		
	case ESpeedometerColorMode::Gradient:
		ResultColor = CalculateGradientColor(VisualSpeedPercent);
		break;
		
	case ESpeedometerColorMode::Custom:
		// Custom mode - can be overridden in Blueprint
		ResultColor = CalculateGradientColor(VisualSpeedPercent);
		break;
		
	default:
		ResultColor = FLinearColor::White;
		break;
	}
	
	// Override with overspeed color if applicable
	if (bEnableOverspeedWarning && IsOverspeed())
	{
		ResultColor = OverspeedColor;
	}
	
	return ResultColor;
}

bool UTrainSpeedometerWidget::IsOverspeed() const
{
	if (!bEnableOverspeedWarning)
	{
		return false;
	}
	
	return CurrentSpeed > (MaxSpeed * OverspeedThreshold);
}

bool UTrainSpeedometerWidget::IsInWarningZone() const
{
	if (!bEnableWarningZone)
	{
		return false;
	}
	
	float SpeedPercent = CurrentSpeed / FMath::Max(MaxSpeed, 1.0f);
	return SpeedPercent >= WarningZoneStart && SpeedPercent < OverspeedThreshold;
}

float UTrainSpeedometerWidget::GetWarningVisibility() const
{
	if (!IsInWarningZone())
	{
		return 0.0f;
	}
	
	// Sine wave for smooth blinking
	return (FMath::Sin(WarningBlinkTimer * PI) + 1.0f) * 0.5f;
}

// ========== Helper Functions ==========

float UTrainSpeedometerWidget::ConvertToDisplayUnit(float SpeedKmh) const
{
	switch (DisplayUnit)
	{
	case ESpeedDisplayUnit::KilometersPerHour:
		return SpeedKmh;
		
	case ESpeedDisplayUnit::MetersPerSecond:
		return SpeedKmh / 3.6f;
		
	case ESpeedDisplayUnit::MilesPerHour:
		return SpeedKmh * 0.621371f;
		
	default:
		return SpeedKmh;
	}
}

FString UTrainSpeedometerWidget::GetUnitText() const
{
	switch (DisplayUnit)
	{
	case ESpeedDisplayUnit::KilometersPerHour:
		return TEXT("km/h");
		
	case ESpeedDisplayUnit::MetersPerSecond:
		return TEXT("m/s");
		
	case ESpeedDisplayUnit::MilesPerHour:
		return TEXT("mph");
		
	default:
		return TEXT("km/h");
	}
}

FLinearColor UTrainSpeedometerWidget::CalculateGradientColor(float Percent) const
{
	// Three-zone gradient: Green -> Yellow -> Red
	if (Percent < 0.33f)
	{
		// Low speed zone (0-33%): Pure green to green-yellow
		float Alpha = Percent / 0.33f;
		return FLinearColor::LerpUsingHSV(LowSpeedColor, LowSpeedColor, Alpha);
	}
	else if (Percent < 0.66f)
	{
		// Medium speed zone (33-66%): Green-yellow to yellow
		float Alpha = (Percent - 0.33f) / 0.33f;
		return FLinearColor::LerpUsingHSV(LowSpeedColor, MediumSpeedColor, Alpha);
	}
	else
	{
		// High speed zone (66-100%): Yellow to red
		float Alpha = FMath::Min((Percent - 0.66f) / 0.34f, 1.0f);
		return FLinearColor::LerpUsingHSV(MediumSpeedColor, HighSpeedColor, Alpha);
	}
}

void UTrainSpeedometerWidget::UpdateVisualSpeed(float DeltaTime)
{
	float TargetSpeed = CurrentSpeed;
	
	if (bUseElasticAnimation)
	{
		// Elastic spring interpolation for bouncy effect
		VisualSpeed = ApplyElasticEffect(VisualSpeed, TargetSpeed, DeltaTime);
	}
	else
	{
		// Standard smooth interpolation
		VisualSpeed = FMath::FInterpTo(VisualSpeed, TargetSpeed, DeltaTime, SmoothingSpeed);
	}
	
	// Update percentage
	VisualSpeedPercent = FMath::Clamp(VisualSpeed / FMath::Max(MaxSpeed, 1.0f), 0.0f, 2.0f);
	
	// Store for next frame
	PreviousVisualSpeed = VisualSpeed;
}

float UTrainSpeedometerWidget::ApplyElasticEffect(float Current, float Target, float DeltaTime)
{
	// Spring physics for elastic animation
	const float SpringStiffness = SmoothingSpeed * 10.0f;
	const float Damping = 0.5f;
	
	// Calculate spring force
	float Displacement = Target - Current;
	float SpringForce = Displacement * SpringStiffness;
	
	// Apply damping
	ElasticVelocity += SpringForce * DeltaTime;
	ElasticVelocity *= FMath::Pow(Damping, DeltaTime);
	
	// Add elastic overshoot
	float ElasticOffset = ElasticVelocity * ElasticStrength;
	
	// Update position
	float NewSpeed = Current + (ElasticVelocity * DeltaTime);
	
	// Prevent oscillation when close to target
	if (FMath::Abs(Target - NewSpeed) < 0.1f && FMath::Abs(ElasticVelocity) < 0.1f)
	{
		ElasticVelocity = 0.0f;
		return Target;
	}
	
	return NewSpeed;
}

void UTrainSpeedometerWidget::UpdatePulseAnimation(float DeltaTime)
{
	if (PulseTimer < PulseDuration)
	{
		PulseTimer += DeltaTime;
	}
}

float UTrainSpeedometerWidget::ApplyRounding(float Speed) const
{
	if (RoundToNearest <= 0)
	{
		return Speed;
	}
	
	// Round to nearest N
	return FMath::RoundToFloat(Speed / RoundToNearest) * RoundToNearest;
}
