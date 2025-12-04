// RailsTrain.h
#pragma once

#include "Components/SplineComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrainPhysicsComponent.h"
#include "RailsTrain.generated.h"

UENUM(BlueprintType)
enum class ETrainState : uint8 {
  Stopped UMETA(DisplayName = "Stopped"),
  Moving UMETA(DisplayName = "Moving"),
  Accelerating UMETA(DisplayName = "Accelerating"),
  Decelerating UMETA(DisplayName = "Decelerating")
};

/**
 * Base class for trains that move along spline paths
 * Characters can board and ride on train platforms
 * Does not modify character class - uses attachment system
 * 
 * NEW: Integrated with TrainPhysicsComponent for realistic physics simulation
 * Passengers work in train's local reference frame (relative space physics)
 */
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

  /** Physics component for realistic train simulation */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UTrainPhysicsComponent *PhysicsComponent;

  // ========== Movement Settings ==========

  /** Reference to the spline path to follow */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
  class ARailsSplinePath *SplinePathRef;

  /** Use physics simulation for movement (recommended) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
  bool bUsePhysicsSimulation = true;

  /** Maximum speed of the train (cm/s) - DEPRECATED: Use PhysicsComponent parameters instead */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Legacy",
            meta = (ClampMin = "0.0", EditCondition = "!bUsePhysicsSimulation"))
  float MaxSpeed = 2000.0f;

  /** Acceleration rate (cm/s²) - DEPRECATED: Use PhysicsComponent parameters instead */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Legacy",
            meta = (ClampMin = "0.0", EditCondition = "!bUsePhysicsSimulation"))
  float AccelerationRate = 500.0f;

  /** Deceleration rate (cm/s²) - DEPRECATED: Use PhysicsComponent parameters instead */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Legacy",
            meta = (ClampMin = "0.0", EditCondition = "!bUsePhysicsSimulation"))
  float DecelerationRate = 800.0f;

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

  /** Characters currently on the train platform */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
  TArray<class ACharacter *> PassengersOnBoard;

  /** Current throttle position (-1.0 to 1.0) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
  float CurrentThrottle = 0.0f;

  /** Current brake position (0.0 to 1.0) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
  float CurrentBrake = 0.0f;

  // ========== Private Variables ==========
private:
  /** Cached spline component for performance */
  USplineComponent *CachedSplineComponent = nullptr;

  /** Current smoothed track grade */
  float SmoothedGrade = 0.0f;

  /** Current smoothed track curvature */
  float SmoothedCurvature = 0.0f;

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

  /** Update passengers to maintain relative physics when jumping */
  void UpdatePassengersPhysics(float DeltaTime);

  /** Calculate track grade at current position (in degrees) */
  float CalculateTrackGrade();

  /** Calculate track curvature at current position (0.0-1.0) */
  float CalculateTrackCurvature();

  /** Update physics component with current track parameters */
  void UpdatePhysicsParameters(float DeltaTime);

  /** Draw debug information for physics */
  void DrawPhysicsDebug();

  // ========== Collision Events ==========

  /** Called when something enters boarding zone */
  UFUNCTION()
  void OnBoardingZoneBeginOverlap(UPrimitiveComponent *OverlappedComponent,
                                  AActor *OtherActor,
                                  UPrimitiveComponent *OtherComp,
                                  int32 OtherBodyIndex, bool bFromSweep,
                                  const FHitResult &SweepResult);

  /** Called when something leaves boarding zone */
  UFUNCTION()
  void OnBoardingZoneEndOverlap(UPrimitiveComponent *OverlappedComponent,
                                AActor *OtherActor,
                                UPrimitiveComponent *OtherComp,
                                int32 OtherBodyIndex);

public:
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
  TArray<ACharacter *> GetPassengers() const { return PassengersOnBoard; }

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
  UTrainPhysicsComponent *GetPhysicsComponent() const { return PhysicsComponent; }

  /** Add wagons to the train */
  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void AddWagons(int32 Count);

  /** Remove wagons from the train */
  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void RemoveWagons(int32 Count);
};
