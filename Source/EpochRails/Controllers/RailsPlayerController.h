// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RailsPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
class ARailsTrain;

/**
 * Basic PlayerController class for a third person game
 * Manages input mappings including train controls
 */
UCLASS(abstract)
class ARailsPlayerController : public APlayerController {
  GENERATED_BODY()

protected:
  // ========== Input Mapping Contexts ==========

  /** Input Mapping Contexts */
  UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
  TArray<UInputMappingContext *> DefaultMappingContexts;

  /** Input Mapping Contexts excluded on mobile */
  UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
  TArray<UInputMappingContext *> MobileExcludedMappingContexts;

  // ========== Train Control Input Actions ==========

  /** Train throttle input action (W/S axis) */
  UPROPERTY(EditAnywhere, Category = "Input|Train Controls")
  UInputAction *TrainThrottleAction;

  /** Train brake input action (Space) */
  UPROPERTY(EditAnywhere, Category = "Input|Train Controls")
  UInputAction *TrainBrakeAction;

  /** Train gear forward input action (E key) */
  UPROPERTY(EditAnywhere, Category = "Input|Train Controls")
  UInputAction *TrainGearForwardAction;

  /** Train gear reverse input action (Q key) */
  UPROPERTY(EditAnywhere, Category = "Input|Train Controls")
  UInputAction *TrainGearReverseAction;

  /** Train gear neutral input action (R key) */
  UPROPERTY(EditAnywhere, Category = "Input|Train Controls")
  UInputAction *TrainGearNeutralAction;

  // ========== Mobile Controls ==========

  /** Mobile controls widget to spawn */
  UPROPERTY(EditAnywhere, Category = "Input|Touch Controls")
  TSubclassOf<UUserWidget> MobileControlsWidgetClass;

  /** Pointer to the mobile controls widget */
  UPROPERTY()
  TObjectPtr<UUserWidget> MobileControlsWidget;

  /** If true, the player will use UMG touch controls even if not playing on
   * mobile platforms */
  UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
  bool bForceTouchControls = false;

  // ========== Lifecycle ==========

  virtual void BeginPlay() override;
  virtual void SetupInputComponent() override;
  virtual void OnPossess(APawn *InPawn) override;

  // ========== Helper Functions ==========

  /** Returns true if the player should use UMG touch controls */
  bool ShouldUseTouchControls() const;

  /** Bind all input actions from mapping contexts automatically */
  void BindInputActions();

  /** Get the train the player is currently on/controlling */
  ARailsTrain *GetControlledTrain() const;

  // ========== Character Input Handlers ==========

  /** Movement input handler */
  UFUNCTION()
  void Move(const FInputActionValue &Value);

  /** Look input handler */
  UFUNCTION()
  void Look(const FInputActionValue &Value);

  /** Jump input handler */
  UFUNCTION()
  void Jump(const FInputActionValue &Value);

  // ========== Train Input Handlers ==========

  /** Train throttle handler (W/S keys) */
  UFUNCTION()
  void OnTrainThrottle(const FInputActionValue &Value);

  /** Train brake handler (Space key) */
  UFUNCTION()
  void OnTrainBrake(const FInputActionValue &Value);

  /** Train gear forward handler (E key) */
  UFUNCTION()
  void OnTrainGearForward();

  /** Train gear reverse handler (Q key) */
  UFUNCTION()
  void OnTrainGearReverse();

  /** Train gear neutral handler (R key) */
  UFUNCTION()
  void OnTrainGearNeutral();
};
