// RailsTrain.h
#pragma once

#include "Components/SplineComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

  // ========== Movement Settings ==========

  /** Reference to the spline path to follow */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
  class ARailsSplinePath *SplinePathRef;

  /** Maximum speed of the train (cm/s) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
            meta = (ClampMin = "0.0"))
  float MaxSpeed = 2000.0f;

  /** Acceleration rate (cm/s?) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
            meta = (ClampMin = "0.0"))
  float AccelerationRate = 500.0f;

  /** Deceleration rate (cm/s?) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
            meta = (ClampMin = "0.0"))
  float DecelerationRate = 800.0f;

  /** Current speed of the train */
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

  // ========== Lifecycle ==========

  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

protected:
  // ========== Movement Functions ==========

  /** Update train position along spline */
  void UpdateTrainMovement(float DeltaTime);

  /** Move train to specific distance on spline */
  void MoveToDistance(float Distance);

  /** Calculate current target speed based on state */
  float GetTargetSpeed() const;

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

  /** Set train speed (will clamp to MaxSpeed) */
  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void SetSpeed(float NewSpeed);

  /** Get current train speed */
  UFUNCTION(BlueprintPure, Category = "Train Control")
  float GetCurrentSpeed() const { return CurrentSpeed; }

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
};