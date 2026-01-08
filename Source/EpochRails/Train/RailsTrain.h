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
