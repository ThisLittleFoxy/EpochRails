// RailsTrain.h
// Train with simulated physics control system

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Interaction/InteractionManagerComponent.h"  // ADD THIS LINE
#include "RailsTrain.generated.h"

UENUM(BlueprintType)
enum class ETrainState : uint8 {
  Stopped UMETA(DisplayName = "Stopped"),
  Moving UMETA(DisplayName = "Moving"),
  Accelerating UMETA(DisplayName = "Accelerating"),
  Decelerating UMETA(DisplayName = "Decelerating")
};

UENUM(BlueprintType)
enum class ETrainGear : uint8 {
  Neutral UMETA(DisplayName = "Neutral (N)"),
  Forward UMETA(DisplayName = "Forward (F)"),
  Reverse UMETA(DisplayName = "Reverse (R)")
};

/**
 * Base class for trains that move along spline paths
 * Characters can board and ride on train platforms
 * Features simulated physics with realistic inertia and control
 */
UCLASS(Blueprintable)
class EPOCHRAILS_API ARailsTrain : public AActor {
  GENERATED_BODY()

public:
  ARailsTrain();

protected:
  // Interaction system (ADD THIS)
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction",
            meta = (AllowPrivateAccess = "true"))
  UInteractionManagerComponent *InteractionManager;

  // ========== Components ==========

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USceneComponent *TrainRoot;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UStaticMeshComponent *TrainBodyMesh;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UStaticMeshComponent *PlatformMesh;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  class UBoxComponent *BoardingZone;

  // ========== Movement Settings ==========

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
  class ARailsSplinePath *SplinePathRef;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
  float CurrentDistance = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
  bool bLoopPath = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
  bool bAutoStart = false;

  // ========== Simulated Physics Settings ==========

  /** Simulated train mass (affects acceleration/braking) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Simulation",
            meta = (ClampMin = "1000.0", UIMin = "10000.0", UIMax = "100000.0"))
  float SimulatedMass = 50000.0f;

  /** Engine power coefficient */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Simulation",
            meta = (ClampMin = "0.1", UIMin = "0.5", UIMax = "3.0"))
  float EnginePowerCoefficient = 1.0f;

  /** Brake power coefficient */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Simulation",
            meta = (ClampMin = "0.1", UIMin = "0.5", UIMax = "5.0"))
  float BrakePowerCoefficient = 2.0f;

  /** Rolling friction coefficient */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Simulation",
            meta = (ClampMin = "0.0", ClampMax = "1.0"))
  float RollingFrictionCoefficient = 0.02f;

  /** Air drag coefficient */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Simulation",
            meta = (ClampMin = "0.0", ClampMax = "1.0"))
  float AirDragCoefficient = 0.001f;

  /** Gravity effect on slopes */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Simulation",
            meta = (ClampMin = "0.0", ClampMax = "2.0"))
  float GravityMultiplier = 1.0f;

  /** Use mass-based inertia */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Simulation")
  bool bUseMassInertia = true;

  // ========== Control Settings ==========

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control")
  ETrainGear CurrentGear = ETrainGear::Neutral;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control")
  float ThrottlePosition = 0.0f;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control")
  float BrakePosition = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control",
            meta = (ClampMin = "100.0", UIMin = "500.0", UIMax = "5000.0"))
  float MaxForwardSpeed = 2000.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control",
            meta = (ClampMin = "50.0", UIMin = "200.0", UIMax = "1000.0"))
  float MaxReverseSpeed = 500.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
  float MinSpeedThreshold = 1.0f;

  // ========== State ==========

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
  ETrainState TrainState = ETrainState::Stopped;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
  float CurrentSpeed = 0.0f;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
  TArray<class ACharacter *> PassengersOnBoard;

  // ========== Debug Info ==========

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
  float SimulatedAcceleration = 0.0f;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
  float CurrentSlopeAngle = 0.0f;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
  float TotalAppliedForce = 0.0f;

  // ========== Lifecycle ==========

  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

protected:
  // ========== Movement Functions ==========

  void UpdateTrainMovement(float DeltaTime);
  void UpdateSimulatedPhysics(float DeltaTime);
  void MoveToDistance(float Distance);
  float CalculateSlopeEffect() const;
  float GetInertiaFactor() const;

  // ========== Collision Events ==========

  UFUNCTION()
  void OnBoardingZoneBeginOverlap(UPrimitiveComponent *OverlappedComponent,
                                  AActor *OtherActor,
                                  UPrimitiveComponent *OtherComp,
                                  int32 OtherBodyIndex, bool bFromSweep,
                                  const FHitResult &SweepResult);

  UFUNCTION()
  void OnBoardingZoneEndOverlap(UPrimitiveComponent *OverlappedComponent,
                                AActor *OtherActor,
                                UPrimitiveComponent *OtherComp,
                                int32 OtherBodyIndex);

public:
  // ========== Control API ==========

  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void SetGear(ETrainGear NewGear);

  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void SetThrottle(float Position);

  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void SetBrake(float Position);

  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void EmergencyBrake();

  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void StartTrain();

  UFUNCTION(BlueprintCallable, Category = "Train Control")
  void StopTrain();

  // ========== Query API ==========

  UFUNCTION(BlueprintPure, Category = "Train Control")
  float GetCurrentSpeed() const { return CurrentSpeed; }

  UFUNCTION(BlueprintPure, Category = "Train Control")
  float GetSpeedKmh() const;

  UFUNCTION(BlueprintPure, Category = "Train Control")
  FString GetGearString() const;

  UFUNCTION(BlueprintPure, Category = "Train Control")
  ETrainState GetTrainState() const { return TrainState; }

  UFUNCTION(BlueprintPure, Category = "Train Control")
  bool IsCharacterOnTrain(ACharacter *Character) const;

  UFUNCTION(BlueprintPure, Category = "Train Control")
  TArray<ACharacter *> GetPassengers() const { return PassengersOnBoard; }
};
