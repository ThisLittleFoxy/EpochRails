// TrainSpeedometerWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TrainSpeedometerWidget.generated.h"

/**
 * Color mode for speedometer progress bar
 */
UENUM(BlueprintType)
enum class ESpeedometerColorMode : uint8
{
	// Single solid color
	Solid UMETA(DisplayName = "Solid Color"),
	// Gradient from green to yellow to red based on speed
	Gradient UMETA(DisplayName = "Speed Gradient"),
	// Custom color curve (set in blueprint)
	Custom UMETA(DisplayName = "Custom Curve")
};

/**
 * Display units for speedometer
 */
UENUM(BlueprintType)
enum class ESpeedDisplayUnit : uint8
{
	KilometersPerHour UMETA(DisplayName = "km/h"),
	MetersPerSecond UMETA(DisplayName = "m/s"),
	MilesPerHour UMETA(DisplayName = "mph")
};

/**
 * Advanced widget that displays train speed as a progress bar and numerical value
 * Features:
 * - Smooth animations and interpolation
 * - Multiple color modes (solid, gradient, custom)
 * - Configurable display units (km/h, m/s, mph)
 * - Warning zones and overspeed alerts
 * - Customizable appearance
 */
UCLASS()
class EPOCHRAILS_API UTrainSpeedometerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTrainSpeedometerWidget(const FObjectInitializer& ObjectInitializer);

	// ========== Core Functions ==========

	/** Update speedometer with new speed value */
	UFUNCTION(BlueprintCallable, Category = "Speedometer")
	void UpdateSpeed(float SpeedKmh, float MaxSpeedKmh);

	/** Set maximum speed (updates the scale) */
	UFUNCTION(BlueprintCallable, Category = "Speedometer")
	void SetMaxSpeed(float NewMaxSpeed);

	/** Force immediate update without interpolation */
	UFUNCTION(BlueprintCallable, Category = "Speedometer")
	void SetSpeedImmediate(float SpeedKmh);

	// ========== Getters for UI Binding ==========

	/** Get normalized speed for progress bar (0.0 to 1.0) */
	UFUNCTION(BlueprintPure, Category = "Speedometer")
	float GetSpeedPercent() const { return VisualSpeedPercent; }

	/** Get formatted speed text (e.g., "120 km/h") */
	UFUNCTION(BlueprintPure, Category = "Speedometer")
	FText GetSpeedText() const;

	/** Get current speed in selected display units */
	UFUNCTION(BlueprintPure, Category = "Speedometer")
	float GetDisplaySpeed() const;

	/** Get current speed in km/h (raw value) */
	UFUNCTION(BlueprintPure, Category = "Speedometer")
	float GetCurrentSpeedKmh() const { return CurrentSpeed; }

	/** Get maximum speed in km/h */
	UFUNCTION(BlueprintPure, Category = "Speedometer")
	float GetMaxSpeedKmh() const { return MaxSpeed; }

	/** Get progress bar fill color based on current settings */
	UFUNCTION(BlueprintPure, Category = "Speedometer")
	FLinearColor GetProgressBarColor() const;

	/** Check if train is in overspeed state */
	UFUNCTION(BlueprintPure, Category = "Speedometer")
	bool IsOverspeed() const;

	/** Check if train is in warning zone */
	UFUNCTION(BlueprintPure, Category = "Speedometer")
	bool IsInWarningZone() const;

	/** Get warning zone visibility (for blinking animation) */
	UFUNCTION(BlueprintPure, Category = "Speedometer")
	float GetWarningVisibility() const;

	// ========== Display Settings ==========

	/** Display unit for speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Display")
	ESpeedDisplayUnit DisplayUnit = ESpeedDisplayUnit::KilometersPerHour;

	/** Number of decimal places (0-2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Display", 
		meta = (ClampMin = "0", ClampMax = "2"))
	int32 DecimalPlaces = 0;

	/** Show unit text (km/h, m/s, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Display")
	bool bShowUnitText = true;

	/** Show max speed indicator */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Display")
	bool bShowMaxSpeedIndicator = true;

	// ========== Color Settings ==========

	/** Color mode for progress bar */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Colors")
	ESpeedometerColorMode ColorMode = ESpeedometerColorMode::Gradient;

	/** Solid color (used when ColorMode = Solid) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Colors",
		meta = (EditCondition = "ColorMode == ESpeedometerColorMode::Solid"))
	FLinearColor SolidColor = FLinearColor::Green;

	/** Low speed color (0-33% of max speed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Colors",
		meta = (EditCondition = "ColorMode == ESpeedometerColorMode::Gradient"))
	FLinearColor LowSpeedColor = FLinearColor::Green;

	/** Medium speed color (33-66% of max speed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Colors",
		meta = (EditCondition = "ColorMode == ESpeedometerColorMode::Gradient"))
	FLinearColor MediumSpeedColor = FLinearColor::Yellow;

	/** High speed color (66-100% of max speed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Colors",
		meta = (EditCondition = "ColorMode == ESpeedometerColorMode::Gradient"))
	FLinearColor HighSpeedColor = FLinearColor::Red;

	/** Overspeed color (above max speed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Colors")
	FLinearColor OverspeedColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	// ========== Warning Settings ==========

	/** Enable overspeed warning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Warnings")
	bool bEnableOverspeedWarning = true;

	/** Overspeed threshold (percentage of max speed, e.g., 1.1 = 110%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Warnings",
		meta = (ClampMin = "1.0", ClampMax = "2.0", EditCondition = "bEnableOverspeedWarning"))
	float OverspeedThreshold = 1.05f;

	/** Enable warning zone (yellow zone before overspeed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Warnings")
	bool bEnableWarningZone = true;

	/** Warning zone start (percentage of max speed, e.g., 0.9 = 90%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Warnings",
		meta = (ClampMin = "0.5", ClampMax = "1.0", EditCondition = "bEnableWarningZone"))
	float WarningZoneStart = 0.90f;

	/** Warning blink speed when in warning zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Warnings",
		meta = (ClampMin = "0.5", ClampMax = "10.0", EditCondition = "bEnableWarningZone"))
	float WarningBlinkSpeed = 2.0f;

	// ========== Animation Settings ==========

	/** Smooth interpolation speed for visual updates (higher = faster) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Animation",
		meta = (ClampMin = "0.1", ClampMax = "20.0"))
	float SmoothingSpeed = 5.0f;

	/** Enable elastic animation (bouncy effect) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Animation")
	bool bUseElasticAnimation = false;

	/** Elastic animation strength (only if elastic enabled) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Animation",
		meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bUseElasticAnimation"))
	float ElasticStrength = 0.3f;

	/** Enable pulse animation on speed changes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Animation")
	bool bEnablePulseAnimation = true;

	/** Pulse animation duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Animation",
		meta = (ClampMin = "0.1", ClampMax = "2.0", EditCondition = "bEnablePulseAnimation"))
	float PulseDuration = 0.3f;

	// ========== Advanced Settings ==========

	/** Minimum speed to display (filter out near-zero values) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Advanced",
		meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float MinDisplaySpeed = 0.5f;

	/** Round speed to nearest value (0 = no rounding, 5 = round to 5s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Advanced",
		meta = (ClampMin = "0", ClampMax = "10"))
	int32 RoundToNearest = 0;

	/** Update rate in seconds (0 = every frame) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer|Advanced",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UpdateRate = 0.0f;

protected:
	// ========== Protected Data ==========

	/** Current actual speed in km/h */
	UPROPERTY(BlueprintReadOnly, Category = "Speedometer")
	float CurrentSpeed = 0.0f;

	/** Maximum speed in km/h */
	UPROPERTY(BlueprintReadOnly, Category = "Speedometer")
	float MaxSpeed = 100.0f;

	/** Visual speed (interpolated for smooth animation) */
	UPROPERTY(BlueprintReadOnly, Category = "Speedometer")
	float VisualSpeed = 0.0f;

	/** Visual speed as percentage (0.0 to 1.0+) */
	UPROPERTY(BlueprintReadOnly, Category = "Speedometer")
	float VisualSpeedPercent = 0.0f;

	// ========== Internal State ==========

private:
	// Previous speed for elastic animation
	float PreviousVisualSpeed = 0.0f;
	
	// Velocity for elastic interpolation
	float ElasticVelocity = 0.0f;

	// Pulse animation time
	float PulseTimer = 0.0f;
	
	// Warning blink timer
	float WarningBlinkTimer = 0.0f;

	// Update accumulator for rate limiting
	float UpdateAccumulator = 0.0f;

	// Last update time
	float LastUpdateTime = 0.0f;

	// ========== Widget Lifecycle ==========

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ========== Helper Functions ==========

	/** Convert km/h to display unit */
	float ConvertToDisplayUnit(float SpeedKmh) const;

	/** Get unit text based on display unit */
	FString GetUnitText() const;

	/** Calculate gradient color based on speed percentage */
	FLinearColor CalculateGradientColor(float Percent) const;

	/** Update visual speed with interpolation */
	void UpdateVisualSpeed(float DeltaTime);

	/** Apply elastic spring effect to speed */
	float ApplyElasticEffect(float Current, float Target, float DeltaTime);

	/** Update pulse animation */
	void UpdatePulseAnimation(float DeltaTime);

	/** Round speed value if rounding is enabled */
	float ApplyRounding(float Speed) const;

	// ========== Blueprint Events ==========

public:
	/** Called when speed changes significantly */
	UFUNCTION(BlueprintImplementableEvent, Category = "Speedometer|Events")
	void OnSpeedChanged(float NewSpeed, float OldSpeed);

	/** Called when entering overspeed state */
	UFUNCTION(BlueprintImplementableEvent, Category = "Speedometer|Events")
	void OnOverspeedEnter();

	/** Called when leaving overspeed state */
	UFUNCTION(BlueprintImplementableEvent, Category = "Speedometer|Events")
	void OnOverspeedExit();

	/** Called when entering warning zone */
	UFUNCTION(BlueprintImplementableEvent, Category = "Speedometer|Events")
	void OnWarningZoneEnter();

	/** Called when leaving warning zone */
	UFUNCTION(BlueprintImplementableEvent, Category = "Speedometer|Events")
	void OnWarningZoneExit();

private:
	// State tracking for events
	bool bWasOverspeed = false;
	bool bWasInWarning = false;
};
