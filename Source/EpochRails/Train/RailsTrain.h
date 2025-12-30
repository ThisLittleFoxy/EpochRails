// RailsTrain.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/SplineComponent.h"
#include "Components/WidgetComponent.h"

#include "TrainPhysicsComponent.h"
#include "TrainSpeedometerWidget.h"

#include "RailsTrain.generated.h"

class UBoxComponent;
class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;

class ARailsSplinePath;
class ARailsPlayerCharacter;
class AWagon;

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

  // ===== Lifecycle =====
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

  // ===== Passenger management =====
  UFUNCTION(BlueprintCallable, Category = "Train|Passengers")
  bool IsPassengerInside(ARailsPlayerCharacter *Character) const;

  UFUNCTION(BlueprintCallable, Category = "Train|Passengers")
  void OnPlayerEnterTrain(ARailsPlayerCharacter *Character);

  UFUNCTION(BlueprintCallable, Category = "Train|Passengers")
  void OnPlayerExitTrain(ARailsPlayerCharacter *Character);

  // ===== Public API: movement / control =====
  UFUNCTION(BlueprintCallable, Category = "Train|Control")
  void StartTrain();

  UFUNCTION(BlueprintCallable, Category = "Train|Control")
  void StopTrain();

  UFUNCTION(BlueprintPure, Category = "Train|Control")
  float GetCurrentSpeed() const { return CurrentSpeed; }

  UFUNCTION(BlueprintPure, Category = "Train|Control")
  float GetCurrentSpeedKmh() const;

  UFUNCTION(BlueprintPure, Category = "Train|Control")
  ETrainState GetTrainState() const { return TrainState; }

  UFUNCTION(BlueprintPure, Category = "Train|Control")
  bool IsCharacterOnTrain(ACharacter *Character) const;

  UFUNCTION(BlueprintPure, Category = "Train|Control")
  TArray<ARailsPlayerCharacter *> GetPassengers() const {
    return PassengersInside;
  }

  UFUNCTION(BlueprintCallable, Category = "Train|Control")
  void ApplyThrottle(float ThrottleValue);

  UFUNCTION(BlueprintCallable, Category = "Train|Control")
  void ApplyBrake(float BrakeValue);

  UFUNCTION(BlueprintPure, Category = "Train|Control")
  float GetThrottlePosition() const { return CurrentThrottle; }

  UFUNCTION(BlueprintPure, Category = "Train|Control")
  float GetBrakePosition() const { return CurrentBrake; }

  UFUNCTION(BlueprintCallable, Category = "Train|Control")
  void EmergencyBrake();

  UFUNCTION(BlueprintPure, Category = "Train|Control")
  float GetStoppingDistance() const;

  UFUNCTION(BlueprintPure, Category = "Train|Control")
  UTrainPhysicsComponent *GetPhysicsComponent() const {
    return PhysicsComponent;
  }

  // ===== Public API: wagons =====
  UFUNCTION(BlueprintCallable, Category = "Train|Wagons")
  void AddWagons(int32 Count);

  UFUNCTION(BlueprintCallable, Category = "Train|Wagons")
  void RemoveWagons(int32 Count);

  UFUNCTION(BlueprintPure, Category = "Train|Wagons")
  int32 GetWagonCount() const { return AttachedWagons.Num(); }

  UFUNCTION(BlueprintPure, Category = "Train|Wagons")
  TArray<AWagon *> GetAttachedWagons() const { return AttachedWagons; }

  UFUNCTION(BlueprintPure, Category = "Train|Wagons")
  USceneComponent *GetRearAttachmentPoint() const {
    return RearAttachmentPoint;
  }

  // ===== Public API: engine / direction / gears =====
  UFUNCTION(BlueprintCallable, Category = "Train|Control")
  void ToggleEngine();

  UFUNCTION(BlueprintCallable, Category = "Train|Control")
  void ToggleReverse();

  UFUNCTION(BlueprintPure, Category = "Train|Control")
  bool IsEngineRunning() const { return bEngineRunning; }

  UFUNCTION(BlueprintPure, Category = "Train|Control")
  float GetReverseMultiplier() const { return ReverseMultiplier; }

  UFUNCTION(BlueprintCallable, Category = "Train|Control")
  void IncreaseThrottle(float Amount = 0.1f);

  UFUNCTION(BlueprintCallable, Category = "Train|Control")
  void ShiftGearUp();

  UFUNCTION(BlueprintCallable, Category = "Train|Control")
  void ShiftGearDown();

  UFUNCTION(BlueprintPure, Category = "Train|Control")
  int32 GetCurrentGear() const { return CurrentGear; }

  UFUNCTION(BlueprintPure, Category = "Train|Control")
  float GetCurrentGearSpeedMultiplier() const;

  UFUNCTION(BlueprintPure, Category = "Train|Control")
  float GetCurrentGearAccelerationRate() const;

  UFUNCTION(BlueprintCallable, Category = "Train|Control")
  void StartBraking();

  UFUNCTION(BlueprintCallable, Category = "Train|Control")
  void StopBraking();

  // ===== Public API: UI =====
  UFUNCTION(BlueprintPure, Category = "Train|Control|UI")
  UTrainSpeedometerWidget *GetSpeedometerWidget() const {
    return CachedSpeedometerWidget;
  }

  UFUNCTION(BlueprintCallable, Category = "Train|Control|UI")
  void SetSpeedometerVisible(bool bVisible);

  UFUNCTION(BlueprintCallable, Category = "Train|Control|UI")
  void SetSpeedometerMaxSpeed(float NewMaxSpeed);

  // ===== Public API: spline =====
  UFUNCTION(BlueprintPure, Category = "Train|Control")
  float GetCurrentSplineDistance() const { return CurrentDistance; }

  // ===== Commands (debug) =====
  UFUNCTION(Exec, Category = "Train|Control")
  void AddWagonsToTrain(int32 Count) { AddWagons(Count); }

  UFUNCTION(Exec, Category = "Train|Control")
  void RemoveWagonsFromTrain(int32 Count) { RemoveWagons(Count); }

protected:
  // ===== Components =====
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USceneComponent *TrainRoot = nullptr;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UStaticMeshComponent *TrainBodyMesh = nullptr;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UStaticMeshComponent *PlatformMesh = nullptr;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UBoxComponent *BoardingZone = nullptr;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components",
            meta = (AllowPrivateAccess = "true"))
  UTrainPhysicsComponent *PhysicsComponent = nullptr;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USceneComponent *RearAttachmentPoint = nullptr;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Train|Components")
  UBoxComponent *TrainInteriorTrigger = nullptr;

  // ===== Movement config =====
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
  ARailsSplinePath *SplinePathRef = nullptr;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
  float CurrentSpeed = 0.0f; // cm/s

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
  float CurrentDistance = 0.0f; // cm along spline

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
  bool bLoopPath = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
  bool bAutoStart = true;

  // ===== Physics config =====
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics",
            meta = (ClampMin = "10.0", ClampMax = "500.0"))
  float PhysicsSampleDistance = 100.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics",
            meta = (ClampMin = "0.1", ClampMax = "10.0"))
  float GradeSmoothingSpeed = 2.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Debug")
  bool bShowPhysicsDebug = false;

  // ===== State =====
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
  ETrainState TrainState = ETrainState::Stopped;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
  float CurrentThrottle = 0.0f;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
  float CurrentBrake = 0.0f;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
  bool bEngineRunning = false;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
  float ReverseMultiplier = 1.0f;

  // ===== Gears =====
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
  int32 CurrentGear = 0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Gears",
            meta = (ClampMin = "1", ClampMax = "10"))
  int32 MaxGears = 3;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Gears")
  TArray<float> GearSpeedMultipliers = {0.3f, 0.6f, 1.0f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Gears",
            meta = (ClampMin = "0.0", ClampMax = "5.0"))
  float GearShiftDelay = 0.5f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Gears")
  TArray<float> GearAccelerationRates = {0.2f, 0.15f, 0.1f};

  // ===== Train input (passengers) =====
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Input")
  UInputMappingContext *DefaultInputMappingContext = nullptr;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Input")
  UInputMappingContext *TrainPassengerInputMappingContext = nullptr;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Input")
  int32 IMCPriority = 0;

  // ===== UI components =====
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|UI")
  UStaticMeshComponent *ControlPanelMesh = nullptr;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|UI")
  UWidgetComponent *SpeedometerWidgetComponent = nullptr;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|UI")
  UWidgetComponent *ControlPanelWidgetComponent = nullptr;

  // ===== Speedometer settings =====
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Speedometer")
  TSubclassOf<UTrainSpeedometerWidget> SpeedometerWidgetClass;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Speedometer")
  FVector SpeedometerRelativeLocation = FVector(10.0f, 0.0f, 50.0f);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Speedometer")
  FRotator SpeedometerRelativeRotation = FRotator(0.0f, 180.0f, 0.0f);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Speedometer")
  FVector2D SpeedometerDrawSize = FVector2D(400.0f, 120.0f);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Speedometer")
  EWidgetSpace SpeedometerWidgetSpace = EWidgetSpace::World;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Speedometer",
            meta = (ClampMin = "50.0", ClampMax = "500.0"))
  float SpeedometerMaxSpeed = 150.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Speedometer")
  bool bShowSpeedometer = true;

  // ===== Control panel settings =====
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|ControlPanel")
  TSubclassOf<UUserWidget> ControlPanelWidgetClass;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|ControlPanel")
  FVector ControlPanelRelativeLocation = FVector(15.0f, 0.0f, 60.0f);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|ControlPanel")
  FRotator ControlPanelRelativeRotation = FRotator(0.0f, 180.0f, 0.0f);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|ControlPanel")
  FVector2D ControlPanelDrawSize = FVector2D(800.0f, 600.0f);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|ControlPanel")
  bool bShowControlPanel = true;

  // ===== Wagons =====
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Train|Wagons")
  TArray<AWagon *> AttachedWagons;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Train|Wagons")
  TSubclassOf<AWagon> WagonClass;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Wagons")
  float DefaultCouplingDistance = 300.0f;

  // ===== Internal helpers =====
  void UpdateTrainMovement(float DeltaTime);
  void UpdatePhysicsMovement(float DeltaTime);
  void MoveToDistance(float Distance);

  float CalculateTrackGrade();
  float CalculateTrackCurvature();
  void UpdatePhysicsParameters(float DeltaTime);

  void DrawPhysicsDebug();

  void InitializeSpeedometer();
  void InitializeControlPanel();
  void UpdateSpeedometerDisplay();

  void SwitchInputMappingContext(ARailsPlayerCharacter *Character,
                                 bool bInsideTrain);
  UEnhancedInputLocalPlayerSubsystem *
  GetInputSubsystem(ARailsPlayerCharacter *Character) const;

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

private:
  // ===== Cached / runtime-only =====
  USplineComponent *CachedSplineComponent = nullptr;

  float SmoothedGrade = 0.0f;
  float SmoothedCurvature = 0.0f;

  float TimeSinceLastGearShift = 0.0f;
  bool bBrakeButtonHeld = false;

  bool CanShiftGear() const;

  UPROPERTY()
  UTrainSpeedometerWidget *CachedSpeedometerWidget = nullptr;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Train|Passengers",
            meta = (AllowPrivateAccess = "true"))
  TArray<ARailsPlayerCharacter *> PassengersInside;
};
