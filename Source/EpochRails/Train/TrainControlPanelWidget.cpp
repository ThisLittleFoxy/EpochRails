// TrainControlPanelWidget.cpp

#include "TrainControlPanelWidget.h"
#include "Train/RailsTrain.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "EpochRails.h"

UTrainControlPanelWidget::UTrainControlPanelWidget(
    const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer) {
  // Enable ticking for continuous updates
  bIsFocusable = false;
}

void UTrainControlPanelWidget::NativeConstruct() {
  Super::NativeConstruct();

  // Bind button events
  if (ThrottleUpButton) {
    ThrottleUpButton->OnPressed.AddDynamic(this,
                                           &UTrainControlPanelWidget::OnThrottleUpPressed);
    ThrottleUpButton->OnReleased.AddDynamic(this,
                                            &UTrainControlPanelWidget::OnThrottleUpReleased);
  }

  if (ThrottleDownButton) {
    ThrottleDownButton->OnPressed.AddDynamic(
        this, &UTrainControlPanelWidget::OnThrottleDownPressed);
    ThrottleDownButton->OnReleased.AddDynamic(
        this, &UTrainControlPanelWidget::OnThrottleDownReleased);
  }

  if (BrakeButton) {
    BrakeButton->OnPressed.AddDynamic(this,
                                      &UTrainControlPanelWidget::OnBrakePressed);
    BrakeButton->OnReleased.AddDynamic(
        this, &UTrainControlPanelWidget::OnBrakeReleased);
  }

  if (EmergencyBrakeButton) {
    EmergencyBrakeButton->OnClicked.AddDynamic(
        this, &UTrainControlPanelWidget::OnEmergencyBrakeClicked);
  }

  if (ReverseToggleButton) {
    ReverseToggleButton->OnClicked.AddDynamic(
        this, &UTrainControlPanelWidget::OnReverseToggleClicked);
  }

  if (EngineToggleButton) {
    EngineToggleButton->OnClicked.AddDynamic(
        this, &UTrainControlPanelWidget::OnEngineToggleClicked);
  }

  // Initialize visual state
  UpdateVisualIndicators();

  UE_LOG(LogEpochRails, Log,
         TEXT("TrainControlPanelWidget: NativeConstruct completed"));
}

void UTrainControlPanelWidget::NativeTick(const FGeometry &MyGeometry,
                                          float InDeltaTime) {
  Super::NativeTick(MyGeometry, InDeltaTime);

  if (!ControlledTrain) {
    return;
  }

  // Update throttle based on held buttons
  UpdateThrottleInput(InDeltaTime);

  // Update display
  UpdatePanelDisplay();
}

void UTrainControlPanelWidget::InitializePanel(ARailsTrain *Train) {
  ControlledTrain = Train;

  if (ControlledTrain) {
    UE_LOG(LogEpochRails, Log,
           TEXT("TrainControlPanelWidget: Initialized with train %s"),
           *ControlledTrain->GetName());
  } else {
    UE_LOG(LogEpochRails, Warning,
           TEXT("TrainControlPanelWidget: Initialized with null train!"));
  }
}

void UTrainControlPanelWidget::UpdatePanelDisplay() {
  if (!ControlledTrain) {
    return;
  }

  // Update speed display
  if (SpeedText) {
    float SpeedKmh = ControlledTrain->GetCurrentSpeedKmh();
    SpeedText->SetText(GetFormattedSpeed(SpeedKmh));
  }

  // Update throttle display
  if (ThrottleText) {
    float Throttle = ControlledTrain->GetThrottlePosition();
    ThrottleText->SetText(GetFormattedThrottle(Throttle));
  }

  // Update throttle bar
  if (ThrottleBar) {
    float Throttle = ControlledTrain->GetThrottlePosition();
    ThrottleBar->SetPercent(FMath::Abs(Throttle));
    ThrottleBar->SetFillColorAndOpacity(ThrottleBarColor);
  }

  // Update brake bar
  if (BrakeBar) {
    float Brake = ControlledTrain->GetBrakePosition();
    BrakeBar->SetPercent(Brake);
    BrakeBar->SetFillColorAndOpacity(BrakeBarColor);
  }

  // Update direction text
  if (DirectionText) {
    FString DirectionString =
        ReverseMultiplier > 0.0f ? TEXT("FORWARD") : TEXT("REVERSE");
    DirectionText->SetText(FText::FromString(DirectionString));
  }

  // Update engine status text
  if (EngineStatusText) {
    FString StatusString = bEngineRunning ? TEXT("RUNNING") : TEXT("OFF");
    EngineStatusText->SetText(FText::FromString(StatusString));
  }

  // Update visual indicators
  UpdateVisualIndicators();
}

// ========== Button Handlers ==========

void UTrainControlPanelWidget::OnThrottleUpPressed() {
  if (!bEngineRunning) {
    UE_LOG(LogEpochRails, Warning,
           TEXT("TrainControlPanel: Cannot increase throttle - engine is off"));
    return;
  }

  bThrottleUpPressed = true;
  UE_LOG(LogEpochRails, Log, TEXT("TrainControlPanel: Throttle UP pressed"));
}

void UTrainControlPanelWidget::OnThrottleUpReleased() {
  bThrottleUpPressed = false;
  UE_LOG(LogEpochRails, Log, TEXT("TrainControlPanel: Throttle UP released"));
}

void UTrainControlPanelWidget::OnThrottleDownPressed() {
  bThrottleDownPressed = true;
  UE_LOG(LogEpochRails, Log, TEXT("TrainControlPanel: Throttle DOWN pressed"));
}

void UTrainControlPanelWidget::OnThrottleDownReleased() {
  bThrottleDownPressed = false;
  UE_LOG(LogEpochRails, Log,
         TEXT("TrainControlPanel: Throttle DOWN released"));
}

void UTrainControlPanelWidget::OnBrakePressed() {
  if (!ControlledTrain) {
    return;
  }

  bBrakePressed = true;
  ControlledTrain->ApplyBrake(1.0f); // Full brake
  UE_LOG(LogEpochRails, Log, TEXT("TrainControlPanel: Brake applied"));
}

void UTrainControlPanelWidget::OnBrakeReleased() {
  if (!ControlledTrain) {
    return;
  }

  bBrakePressed = false;
  ControlledTrain->ApplyBrake(0.0f); // Release brake
  UE_LOG(LogEpochRails, Log, TEXT("TrainControlPanel: Brake released"));
}

void UTrainControlPanelWidget::OnEmergencyBrakeClicked() {
  if (!ControlledTrain) {
    return;
  }

  ControlledTrain->EmergencyBrake();
  bEngineRunning = false; // Emergency brake also stops engine

  UE_LOG(LogEpochRails, Warning,
         TEXT("TrainControlPanel: EMERGENCY BRAKE activated!"));
}

void UTrainControlPanelWidget::OnReverseToggleClicked() {
  if (!ControlledTrain) {
    return;
  }

  // Only allow reverse toggle when train is stopped
  float CurrentSpeed = ControlledTrain->GetCurrentSpeedKmh();
  if (FMath::Abs(CurrentSpeed) > 1.0f) {
    UE_LOG(LogEpochRails, Warning,
           TEXT("TrainControlPanel: Cannot change direction while moving! "
                "Stop the train first."));
    return;
  }

  // Toggle direction
  ReverseMultiplier *= -1.0f;

  UE_LOG(LogEpochRails, Log,
         TEXT("TrainControlPanel: Direction changed to %s"),
         ReverseMultiplier > 0.0f ? TEXT("FORWARD") : TEXT("REVERSE"));
}

void UTrainControlPanelWidget::OnEngineToggleClicked() {
  bEngineRunning = !bEngineRunning;

  if (!bEngineRunning && ControlledTrain) {
    // Engine off - apply brake and zero throttle
    ControlledTrain->ApplyThrottle(0.0f);
    ControlledTrain->ApplyBrake(1.0f);
  }

  UE_LOG(LogEpochRails, Log, TEXT("TrainControlPanel: Engine %s"),
         bEngineRunning ? TEXT("started") : TEXT("stopped"));
}

// ========== Private Methods ==========

void UTrainControlPanelWidget::UpdateThrottleInput(float DeltaTime) {
  if (!ControlledTrain || !bEngineRunning) {
    return;
  }

  float CurrentThrottle = ControlledTrain->GetThrottlePosition();
  float ThrottleChange = 0.0f;

  // Calculate throttle change based on buttons
  if (bThrottleUpPressed) {
    ThrottleChange += ThrottleChangeRate * DeltaTime;
  }
  if (bThrottleDownPressed) {
    ThrottleChange -= ThrottleChangeRate * DeltaTime;
  }

  // Apply change
  if (FMath::Abs(ThrottleChange) > SMALL_NUMBER) {
    float NewThrottle = FMath::Clamp(CurrentThrottle + ThrottleChange, 0.0f, 1.0f);
    
    // Apply direction multiplier
    float DirectionalThrottle = NewThrottle * ReverseMultiplier;
    
    ControlledTrain->ApplyThrottle(DirectionalThrottle);
  }
}

void UTrainControlPanelWidget::UpdateVisualIndicators() {
  // Update direction indicator
  if (DirectionIndicator) {
    FLinearColor IndicatorColor =
        ReverseMultiplier > 0.0f ? ForwardColor : ReverseColor;
    DirectionIndicator->SetColorAndOpacity(IndicatorColor);
  }

  // Update engine indicator
  if (EngineIndicator) {
    FLinearColor IndicatorColor =
        bEngineRunning ? EngineOnColor : EngineOffColor;
    EngineIndicator->SetColorAndOpacity(IndicatorColor);
  }
}

FText UTrainControlPanelWidget::GetFormattedSpeed(float SpeedKmh) const {
  return FText::FromString(FString::Printf(TEXT("%.1f km/h"), SpeedKmh));
}

FText UTrainControlPanelWidget::GetFormattedThrottle(float Throttle) const {
  int32 ThrottlePercent = FMath::RoundToInt(FMath::Abs(Throttle) * 100.0f);
  return FText::FromString(FString::Printf(TEXT("%d%%"), ThrottlePercent));
}
