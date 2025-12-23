// RailsTrain.h

#pragma once

#include "Components/SplineComponent.h"
#include "Components/WidgetComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrainPhysicsComponent.h"
#include "TrainSpeedometerWidget.h"
#include "RailsTrain.generated.h"

class UBoxComponent;
class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;

UENUM(BlueprintType)
enum class ETrainState : uint8 {
  Stopped UMETA(DisplayName = "Stopped"),
  Moving UMETA(DisplayName = "Moving"),
  Accelerating UMETA(DisplayName = "Accelerating"),
  Decelerating UMETA(DisplayName = "Decelerating")
};

UCLASS(Blueprintable)
class EPOCHRAILS_API ARailsTrain : public AActor {
  GENERATED_BODY()

public:
  ARailsTrain();

protected:
  // ========== Components ==========

  /** Root scene component for the train */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USceneComponent *TrainRoot;

  /** Main body mesh of the train */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UStaticMeshComponent *TrainBodyMesh;

  /** Platform for characters to stand on */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UStaticMeshComponent *PlatformMesh;

  /** Collision for boarding detection */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  class UBoxComponent *BoardingZone;

/** Train physics simulation component */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components",
            meta = (AllowPrivateAccess = "true"))
  UTrainPhysicsComponent *PhysicsComponent;

  // ========== Movement Settings ==========

  /** Reference to the spline path to follow */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
  class ARailsSplinePath *SplinePathRef;

  /** Use physics simulation for movement (recommended) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
  bool bUsePhysicsSimulation = true;

  /** Current speed of the train (cm/s) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
  float CurrentSpeed = 0.0f;

  /** Current distance along spline (0.0 - SplineLength) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
  float CurrentDistance = 0.0f;

  /** Should the train loop when reaching the end? */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
  bool bLoopPath = true;

  /** Auto-start moving when game begins */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
  bool bAutoStart = true;

  // ========== Physics Settings ==========

  /** Sample points ahead for grade/curvature calculation */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics",
            meta = (ClampMin = "10.0", ClampMax = "500.0"))
  float PhysicsSampleDistance = 100.0f;

  /** Smoothing factor for grade changes (higher = smoother) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics",
            meta = (ClampMin = "0.1", ClampMax = "10.0"))
  float GradeSmoothingSpeed = 2.0f;

  /** Display debug physics info on screen */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Debug")
  bool bShowPhysicsDebug = false;

  // ========== State ==========

  /** Current state of the train */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
  ETrainState TrainState = ETrainState::Stopped;

  /** Current throttle position (-1.0 to 1.0) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
  float CurrentThrottle = 0.0f;

  /** Current brake position (0.0 to 1.0) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
  float CurrentBrake = 0.0f;

  /** Engine running state */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
  bool bEngineRunning = false;

  /** Reverse multiplier (1.0 = forward, -1.0 = reverse) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
  float ReverseMultiplier = 1.0f;
  // ========== Gear System ==========

  /** Current gear (0 = neutral, 1-3 = forward gears) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
  int32 CurrentGear = 0;

  /** Maximum number of gears */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Gears",
            meta = (ClampMin = "1", ClampMax = "10"))
  int32 MaxGears = 3;

  /** Speed multiplier for each gear (index 0 = gear 1) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Gears")
  TArray<float> GearSpeedMultipliers = {0.3f, 0.6f, 1.0f};

  /** Time delay between gear shifts (seconds) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Gears",
            meta = (ClampMin = "0.0", ClampMax = "5.0"))
  float GearShiftDelay = 0.5f;

  /** Throttle acceleration rate per gear */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Gears")
  TArray<float> GearAccelerationRates = {0.2f, 0.15f, 0.1f};

  // ===== Input Mapping Contexts for passengers =====

  /** Default IMC used outside the train (with jump) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Input")
  UInputMappingContext *DefaultInputMappingContext;

  /** IMC used inside the train as passenger (without jump) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Input")
  UInputMappingContext *TrainPassengerInputMappingContext;

  /** Priority for IMC when added to input subsystem */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Input")
  int32 IMCPriority = 0;

  // Legacy mode settings
  /** Maximum speed of the train (cm/s) - Used for legacy mode and speedometer
   * display */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Legacy",
            meta = (ClampMin = "0.0", EditCondition = "!bUsePhysicsSimulation"))
  float MaxSpeed = 2000.0f;

  /** Acceleration rate (cm/s²) - Used for legacy mode */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Legacy",
            meta = (ClampMin = "0.0", EditCondition = "!bUsePhysicsSimulation"))
  float AccelerationRate = 500.0f;

  /** Deceleration rate (cm/s²) - Used for legacy mode */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Legacy",
            meta = (ClampMin = "0.0", EditCondition = "!bUsePhysicsSimulation"))
  float DecelerationRate = 800.0f;

  // ========== UI Components ==========

  /** Control panel mesh for mounting UI elements */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|UI")
  UStaticMeshComponent *ControlPanelMesh;

  /** Widget component that displays speedometer on control panel */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|UI")
  UWidgetComponent *SpeedometerWidgetComponent;

  /** Widget component for interactive control panel */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|UI")
  UWidgetComponent *ControlPanelWidgetComponent;

  // ========== Speedometer Settings ==========

  /** Blueprint class for speedometer widget (set this in BP) */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Speedometer")
  TSubclassOf<UTrainSpeedometerWidget> SpeedometerWidgetClass;

  /** Position offset of speedometer relative to control panel */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Speedometer")
  FVector SpeedometerRelativeLocation = FVector(10.0f, 0.0f, 50.0f);

  /** Rotation of speedometer widget (to face player) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Speedometer")
  FRotator SpeedometerRelativeRotation = FRotator(0.0f, 180.0f, 0.0f);

  /** Screen size of speedometer widget (width x height) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Speedometer")
  FVector2D SpeedometerDrawSize = FVector2D(400.0f, 120.0f);

  /** Widget space mode (World or Screen) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Speedometer")
  EWidgetSpace SpeedometerWidgetSpace = EWidgetSpace::World;

  /** Maximum speed to display on speedometer (km/h) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Speedometer",
            meta = (ClampMin = "50.0", ClampMax = "500.0"))
  float SpeedometerMaxSpeed = 150.0f;

  /** Enable speedometer visibility */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Speedometer")
  bool bShowSpeedometer = true;

  // ========== Control Panel Settings ==========

  /** Blueprint widget class for control panel (set this in BP) */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|ControlPanel")
  TSubclassOf<UUserWidget> ControlPanelWidgetClass;

  /** Position offset of control panel relative to mesh */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|ControlPanel")
  FVector ControlPanelRelativeLocation = FVector(15.0f, 0.0f, 60.0f);

  /** Rotation of control panel widget */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|ControlPanel")
  FRotator ControlPanelRelativeRotation = FRotator(0.0f, 180.0f, 0.0f);

  /** Screen size of control panel widget */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|ControlPanel")
  FVector2D ControlPanelDrawSize = FVector2D(800.0f, 600.0f);

  /** Enable control panel visibility */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|ControlPanel")
  bool bShowControlPanel = true;

private:
  // ========== Private Variables ==========

  /** Cached spline component for performance */
  USplineComponent *CachedSplineComponent = nullptr;

  /** Current smoothed track grade */
  float SmoothedGrade = 0.0f;

  /** Current smoothed track curvature */
  float SmoothedCurvature = 0.0f;

  /** Time since last gear shift */
  float TimeSinceLastGearShift = 0.0f;

  /** Can shift gear right now? */
  bool CanShiftGear() const;

  /** Is brake button currently held? */
  bool bBrakeButtonHeld = false;
  // ========== Lifecycle ==========

public:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

protected:
  // ========== Movement Functions ==========

  /** Update train position along spline */
  void UpdateTrainMovement(float DeltaTime);

  /** Update train movement using physics simulation */
  void UpdatePhysicsMovement(float DeltaTime);

  /** Update train movement using legacy system */
  void UpdateLegacyMovement(float DeltaTime);

  /** Move train to specific distance on spline */
  void MoveToDistance(float Distance);

  /** Calculate current target speed based on state */
  float GetTargetSpeed() const;

  // ========== Physics Helper Functions ==========

  /** Calculate track grade at current position (in degrees) */
  float CalculateTrackGrade();

  /** Calculate track curvature at current position (0.0-1.0) */
  float CalculateTrackCurvature();

  /** Update physics component with current track parameters */
  void UpdatePhysicsParameters(float DeltaTime);

  /** Draw debug information for physics */
  void DrawPhysicsDebug();

  /** Initialize speedometer widget */
  void InitializeSpeedometer();

  /** Initialize control panel widget */
  void InitializeControlPanel();

  /** Update speedometer display with current train speed */
  void UpdateSpeedometerDisplay();

  /** List of passengers currently inside the train */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Train|Passengers")
  TArray<class ARailsPlayerCharacter *> PassengersInside;

  /** Trigger box that defines the interior space of the train */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Train|Components")
  class UBoxComponent *TrainInteriorTrigger;

  /** Switch input mapping context for a character */
  void SwitchInputMappingContext(class ARailsPlayerCharacter *Character,
                                 bool bInsideTrain);

  /** Get Enhanced Input subsystem for a character */
  class UEnhancedInputLocalPlayerSubsystem *
  GetInputSubsystem(class ARailsPlayerCharacter *Character) const;

  // Overlap handlers for train interior trigger
  UFUNCTION()
  void OnTrainInteriorBeginOverlap(UPrimitiveComponent *OverlappedComponent,
                                   AActor *OtherActor,
                                   UPrimitiveComponent *OtherComp,
                                   int32 OtherBodyIndex, bool bFromSweep,
                                   const FHitResult &SweepResult);

  UFUNCTION()
  void OnTrainInteriorEndOverlap(UPrimitiveComponent *OverlappedComponent,
                                 AActor *OtherActor,
                                 UPrimitiveComponent *OtherComp,
                                 int32 OtherBodyIndex);

public:
  // ===== Passenger management =====

  /** Check if character is currently inside the train */
  UFUNCTION(BlueprintCallable, Category = "Train|Passengers")
  bool IsPassengerInside(ARailsPlayerCharacter *Character) const;

  /** Called when player enters the train interior */
  UFUNCTION(BlueprintCallable, Category = "Train|Passengers")
  void OnPlayerEnterTrain(ARailsPlayerCharacter *Character);

  /** Called when player exits the train interior */
  UFUNCTION(BlueprintCallable, Category = "Train|Passengers")
  void OnPlayerExitTrain(ARailsPlayerCharacter *Character);

  // ========== Public API ==========

  /** Start train movement */
  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void StartTrain();

  /** Stop train movement */
  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void StopTrain();

  /** Set train speed (will clamp to MaxSpeed) - Legacy mode only */
  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void SetSpeed(float NewSpeed);

  /** Get current train speed in cm/s */
  UFUNCTION(BlueprintPure, Category = "Train Control")
  float GetCurrentSpeed() const { return CurrentSpeed; }

  /** Get current train speed in km/h */
  UFUNCTION(BlueprintPure, Category = "Train Control")
  float GetCurrentSpeedKmh() const;

  /** Get train state */
  UFUNCTION(BlueprintPure, Category = "Train Control")
  ETrainState GetTrainState() const { return TrainState; }

  /** Check if character is on train */
  UFUNCTION(BlueprintPure, Category = "Train Control")
  bool IsCharacterOnTrain(ACharacter *Character) const;

  /** Get all passengers */
  UFUNCTION(BlueprintPure, Category = "Train Control")
  TArray<ARailsPlayerCharacter *> GetPassengers() const {
    return PassengersInside;
  }

  /** Apply throttle input (-1.0 to 1.0) */
  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void ApplyThrottle(float ThrottleValue);

  /** Apply brake input (0.0 to 1.0) */
  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void ApplyBrake(float BrakeValue);

  /** Get current throttle position */
  UFUNCTION(BlueprintPure, Category = "Train Control")
  float GetThrottlePosition() const { return CurrentThrottle; }

  /** Get current brake position */
  UFUNCTION(BlueprintPure, Category = "Train Control")
  float GetBrakePosition() const { return CurrentBrake; }

  /** Emergency brake - maximum braking immediately */
  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void EmergencyBrake();

  /** Get stopping distance at current velocity */
  UFUNCTION(BlueprintPure, Category = "Train Control")
  float GetStoppingDistance() const;

  /** Get reference to physics component */
  UFUNCTION(BlueprintPure, Category = "Train Control")
  UTrainPhysicsComponent *GetPhysicsComponent() const {
    return PhysicsComponent;
  }

  /** Add wagons to the train */
  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void AddWagons(int32 Count);

  /** Remove wagons from the train */
  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void RemoveWagons(int32 Count);

  // ========== NEW: Blueprint-Callable Control Functions ==========

  /** Toggle engine on/off */
  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void ToggleEngine();

  /** Toggle reverse direction */
  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void ToggleReverse();

  /** Get engine running state */
  UFUNCTION(BlueprintPure, Category = "Train Control")
  bool IsEngineRunning() const { return bEngineRunning; }

  /** Get reverse state (1.0 = forward, -1.0 = reverse) */
  UFUNCTION(BlueprintPure, Category = "Train Control")
  float GetReverseMultiplier() const { return ReverseMultiplier; }

  /** Apply throttle for button press (incremental) */
  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void IncreaseThrottle(float Amount = 0.1f);

  /** Cached reference to speedometer widget for fast access */
  UPROPERTY()
  UTrainSpeedometerWidget *CachedSpeedometerWidget = nullptr;

  // ========== Speedometer Control ==========

  /** Get reference to speedometer widget */
  UFUNCTION(BlueprintPure, Category = "Train Control|UI")
  UTrainSpeedometerWidget *GetSpeedometerWidget() const {
    return CachedSpeedometerWidget;
  }

  /** Set speedometer visibility */
  UFUNCTION(BlueprintCallable, Category = "Train Control|UI")
  void SetSpeedometerVisible(bool bVisible);

  /** Update speedometer maximum speed */
  UFUNCTION(BlueprintCallable, Category = "Train Control|UI")
  void SetSpeedometerMaxSpeed(float NewMaxSpeed);

  // ========== Gear Control ==========

  /** Shift to next gear (up) */
  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void ShiftGearUp();

  /** Shift to previous gear (down) */
  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void ShiftGearDown();

  /** Get current gear */
  UFUNCTION(BlueprintPure, Category = "Train Control")
  int32 GetCurrentGear() const { return CurrentGear; }

  /** Get gear speed multiplier */
  UFUNCTION(BlueprintPure, Category = "Train Control")
  float GetCurrentGearSpeedMultiplier() const;

  /** Get gear acceleration rate */
  UFUNCTION(BlueprintPure, Category = "Train Control")
  float GetCurrentGearAccelerationRate() const;

  /** Start braking (call on button press) */
  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void StartBraking();

  /** Stop braking (call on button release) */
  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void StopBraking();
};
