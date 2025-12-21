// TrainControlPanelWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "TrainControlPanelWidget.generated.h"

class ARailsTrain;

/**
 * Interactive control panel widget for train operation
 * Allows player to control train through touch/click interface
 */
UCLASS()
class EPOCHRAILS_API UTrainControlPanelWidget : public UUserWidget {
  GENERATED_BODY()

public:
  UTrainControlPanelWidget(const FObjectInitializer &ObjectInitializer);

  /** Initialize widget with train reference */
  UFUNCTION(BlueprintCallable, Category = "Control Panel")
  void InitializePanel(ARailsTrain *Train);

  /** Update panel display with current train state */
  UFUNCTION(BlueprintCallable, Category = "Control Panel")
  void UpdatePanelDisplay();

protected:
  virtual void NativeConstruct() override;
  virtual void NativeTick(const FGeometry &MyGeometry, float InDeltaTime) override;

  // ========== Button Handlers ==========

  UFUNCTION()
  void OnThrottleUpPressed();

  UFUNCTION()
  void OnThrottleUpReleased();

  UFUNCTION()
  void OnThrottleDownPressed();

  UFUNCTION()
  void OnThrottleDownReleased();

  UFUNCTION()
  void OnBrakePressed();

  UFUNCTION()
  void OnBrakeReleased();

  UFUNCTION()
  void OnEmergencyBrakeClicked();

  UFUNCTION()
  void OnReverseToggleClicked();

  UFUNCTION()
  void OnEngineToggleClicked();

  // ========== UI Components (Bind in UMG) ==========

  /** Button to increase throttle */
  UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
  UButton *ThrottleUpButton;

  /** Button to decrease throttle */
  UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
  UButton *ThrottleDownButton;

  /** Button to apply brake */
  UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
  UButton *BrakeButton;

  /** Emergency brake button */
  UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
  UButton *EmergencyBrakeButton;

  /** Toggle reverse direction */
  UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
  UButton *ReverseToggleButton;

  /** Engine on/off toggle */
  UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
  UButton *EngineToggleButton;

  /** Current speed display */
  UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
  UTextBlock *SpeedText;

  /** Current throttle position display */
  UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
  UTextBlock *ThrottleText;

  /** Current direction display */
  UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
  UTextBlock *DirectionText;

  /** Engine status display */
  UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
  UTextBlock *EngineStatusText;

  /** Throttle position bar */
  UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
  UProgressBar *ThrottleBar;

  /** Brake position bar */
  UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
  UProgressBar *BrakeBar;

  /** Direction indicator image */
  UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
  UImage *DirectionIndicator;

  /** Engine status indicator */
  UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
  UImage *EngineIndicator;

  // ========== State Variables ==========

  /** Reference to controlled train */
  UPROPERTY()
  ARailsTrain *ControlledTrain;

  /** Current reverse mode (1 = forward, -1 = reverse) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control Panel")
  float ReverseMultiplier = 1.0f;

  /** Engine running state */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control Panel")
  bool bEngineRunning = false;

  /** Throttle change rate when button held */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control Panel")
  float ThrottleChangeRate = 0.5f; // 50% per second

  /** Is throttle up button currently pressed */
  bool bThrottleUpPressed = false;

  /** Is throttle down button currently pressed */
  bool bThrottleDownPressed = false;

  /** Is brake button currently pressed */
  bool bBrakePressed = false;

  // ========== Visual Settings ==========

  /** Color for forward direction indicator */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control Panel|Visual")
  FLinearColor ForwardColor = FLinearColor::Green;

  /** Color for reverse direction indicator */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control Panel|Visual")
  FLinearColor ReverseColor = FLinearColor::Yellow;

  /** Color for engine on indicator */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control Panel|Visual")
  FLinearColor EngineOnColor = FLinearColor::Green;

  /** Color for engine off indicator */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control Panel|Visual")
  FLinearColor EngineOffColor = FLinearColor::Red;

  /** Color for throttle bar */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control Panel|Visual")
  FLinearColor ThrottleBarColor = FLinearColor::Green;

  /** Color for brake bar */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control Panel|Visual")
  FLinearColor BrakeBarColor = FLinearColor::Red;

private:
  /** Update throttle based on held buttons */
  void UpdateThrottleInput(float DeltaTime);

  /** Update visual indicators */
  void UpdateVisualIndicators();

  /** Format speed for display */
  FText GetFormattedSpeed(float SpeedKmh) const;

  /** Format throttle for display */
  FText GetFormattedThrottle(float Throttle) const;
};
