// RailsTrain.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "RailsTrain.generated.h"

class UFloatingPawnMovement;
class USplineComponent;
class UBoxComponent;
class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;
class ARailsSplinePath;
class ARailsPlayerCharacter;
class ARailsWagon;

UCLASS(Blueprintable)
class EPOCHRAILS_API ARailsTrain : public APawn {
  GENERATED_BODY()

public:
  ARailsTrain();

  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

  // ===== Movement API =====
  UFUNCTION(BlueprintCallable, Category = "Train")
  void StartTrain();

  UFUNCTION(BlueprintCallable, Category = "Train")
  void StopTrain();

  UFUNCTION(BlueprintPure, Category = "Train")
  float GetSpeed() const { return Speed; }

  UFUNCTION(BlueprintCallable, Category = "Train")
  void SetSpeed(float NewSpeed) { Speed = NewSpeed; }

  UFUNCTION(BlueprintPure, Category = "Train")
  bool IsStopped() const { return bStop; }

  // ===== Passenger management =====
  UFUNCTION(BlueprintCallable, Category = "Train|Passengers")
  bool IsPassengerInside(ARailsPlayerCharacter *Character) const;

  UFUNCTION(BlueprintCallable, Category = "Train|Passengers")
  void OnPlayerEnterTrain(ARailsPlayerCharacter *Character);

  UFUNCTION(BlueprintCallable, Category = "Train|Passengers")
  void OnPlayerExitTrain(ARailsPlayerCharacter *Character);

  // ===== Wagon API =====

  /** Add a wagon to the train. Returns the created wagon or nullptr on failure. */
  UFUNCTION(BlueprintCallable, Category = "Train|Wagons")
  ARailsWagon *AddWagon(TSubclassOf<ARailsWagon> WagonClass = nullptr);

  /** Remove the last wagon from the train. Returns true if successful. */
  UFUNCTION(BlueprintCallable, Category = "Train|Wagons")
  bool RemoveLastWagon();

  /** Get the number of attached wagons */
  UFUNCTION(BlueprintPure, Category = "Train|Wagons")
  int32 GetWagonCount() const { return AttachedWagons.Num(); }

  /** Get all attached wagons */
  UFUNCTION(BlueprintPure, Category = "Train|Wagons")
  TArray<ARailsWagon *> GetAttachedWagons() const;

  /** Get the current distance along the spline (needed by wagons) */
  UFUNCTION(BlueprintPure, Category = "Train|Path")
  float GetCurrentSplineDistance() const;

  /** Get the rear coupler attachment point */
  UFUNCTION(BlueprintPure, Category = "Train|Wagons")
  USceneComponent *GetRearCoupler() const { return RearCoupler; }

protected:
  // ===== Components =====
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<USceneComponent> Root = nullptr;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<UStaticMeshComponent> BodyMesh = nullptr;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<UFloatingPawnMovement> Movement = nullptr;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<UBoxComponent> InteriorTrigger = nullptr;

  /** Rear coupler for attaching wagons */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<USceneComponent> RearCoupler = nullptr;

  // ===== Wagon settings =====

  /** Default wagon class to spawn when AddWagon is called without a class */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Wagons")
  TSubclassOf<ARailsWagon> DefaultWagonClass;

  /** All wagons attached to this train */
  UPROPERTY(BlueprintReadOnly, Category = "Train|Wagons")
  TArray<TObjectPtr<ARailsWagon>> AttachedWagons;

  // ===== Path settings =====
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Path")
  TObjectPtr<ARailsSplinePath> ActivePath = nullptr;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Path")
  float LookAheadDistance = 50.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Path")
  float StopTolerance = 50.0f;

  // ===== Movement settings =====
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Movement")
  float Speed = 1.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Movement")
  bool bStop = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Movement")
  bool bAutoStart = false;

  // ===== Input settings =====
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Input")
  TObjectPtr<UInputMappingContext> DefaultInputMappingContext = nullptr;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Input")
  TObjectPtr<UInputMappingContext> TrainPassengerInputMappingContext = nullptr;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Input")
  int32 IMCPriority = 0;

  // ===== Core logic =====
  UFUNCTION(BlueprintCallable, Category = "Train|Path")
  void UpdatePath();

  USplineComponent *GetActiveSpline() const;

  // ===== Passenger helpers =====
  void SwitchInputMappingContext(ARailsPlayerCharacter *Character, bool bInsideTrain);
  UEnhancedInputLocalPlayerSubsystem *GetInputSubsystem(ARailsPlayerCharacter *Character) const;

  UFUNCTION()
  void OnInteriorBeginOverlap(UPrimitiveComponent *OverlappedComponent,
                              AActor *OtherActor,
                              UPrimitiveComponent *OtherComp,
                              int32 OtherBodyIndex, bool bFromSweep,
                              const FHitResult &SweepResult);

  UFUNCTION()
  void OnInteriorEndOverlap(UPrimitiveComponent *OverlappedComponent,
                            AActor *OtherActor,
                            UPrimitiveComponent *OtherComp,
                            int32 OtherBodyIndex);

private:
  TArray<TWeakObjectPtr<ARailsPlayerCharacter>> PassengersInside;
};
