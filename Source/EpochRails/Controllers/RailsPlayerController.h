// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RailsPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class ATrainActor;
class UBuildingComponent;
struct FInputActionValue;

/**
 * FPS PlayerController that handles all input and delegates to the controlled pawn.
 *
 * The controller processes input and calls methods on the pawn via IControllableCharacterInterface.
 * For basic movement (Move, Look, Jump), it uses standard Pawn/Character methods.
 * For extended actions (Sprint, Interact, Fire), it uses the interface.
 *
 * Your Blueprint Character should implement IControllableCharacterInterface to receive
 * Sprint, Interact, and Fire inputs.
 */
UCLASS(abstract)
class ARailsPlayerController : public APlayerController {
  GENERATED_BODY()

public:
  ARailsPlayerController();

protected:
  // ========== Input Mapping Contexts ==========

  /** Input Mapping Contexts applied to all platforms */
  UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
  TArray<UInputMappingContext *> DefaultMappingContexts;

  /** Input Mapping Contexts excluded on mobile (e.g., mouse look) */
  UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
  TArray<UInputMappingContext *> MobileExcludedMappingContexts;

  // ========== Touch Controls ==========

  /** Mobile controls widget to spawn */
  UPROPERTY(EditAnywhere, Category = "Input|Touch Controls")
  TSubclassOf<UUserWidget> MobileControlsWidgetClass;

  /** Pointer to the mobile controls widget */
  UPROPERTY()
  TObjectPtr<UUserWidget> MobileControlsWidget;

  /** If true, force UMG touch controls even on non-mobile platforms */
  UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
  bool bForceTouchControls = false;

  // ========== UI State ==========

  /** Currently interacting with UI widget */
  UPROPERTY(BlueprintReadOnly, Category = "UI")
  bool bIsInteractingWithUI = false;

  // ========== Lifecycle ==========

  virtual void BeginPlay() override;
  virtual void SetupInputComponent() override;
  virtual void OnPossess(APawn *InPawn) override;

  // ========== Input Setup ==========

  /** Returns true if the player should use UMG touch controls */
  bool ShouldUseTouchControls() const;

  /** Bind all input actions from mapping contexts automatically */
  void BindInputActions();

  // ========== Input Handlers ==========

  /** Movement input - calculates direction from camera and applies to pawn */
  UFUNCTION()
  void HandleMove(const FInputActionValue &Value);

  /** Look input - applies yaw/pitch to controller rotation */
  UFUNCTION()
  void HandleLook(const FInputActionValue &Value);

  /** Jump started */
  UFUNCTION()
  void HandleJumpStarted(const FInputActionValue &Value);

  /** Jump completed */
  UFUNCTION()
  void HandleJumpCompleted(const FInputActionValue &Value);

  /** Sprint started */
  UFUNCTION()
  void HandleSprintStarted(const FInputActionValue &Value);

  /** Sprint completed */
  UFUNCTION()
  void HandleSprintCompleted(const FInputActionValue &Value);

  /** Interact pressed */
  UFUNCTION()
  void HandleInteract(const FInputActionValue &Value);

  /** Fire started */
  UFUNCTION()
  void HandleFireStarted(const FInputActionValue &Value);

  /** Fire completed */
  UFUNCTION()
  void HandleFireCompleted(const FInputActionValue &Value);

  // ========== Train Control Handlers ==========

  /** Train throttle input (0-1 axis) */
  UFUNCTION()
  void HandleTrainThrottle(const FInputActionValue &Value);

  /** Train brake input (0-1 axis) */
  UFUNCTION()
  void HandleTrainBrake(const FInputActionValue &Value);

  /** Toggle train reverse */
  UFUNCTION()
  void HandleTrainReverse(const FInputActionValue &Value);

  /** Emergency brake */
  UFUNCTION()
  void HandleTrainEmergencyBrake(const FInputActionValue &Value);

  // ========== Building Handlers ==========

  /** Toggle build mode */
  UFUNCTION()
  void HandleToggleBuildMode(const FInputActionValue &Value);

  /** Place structure in build mode */
  UFUNCTION()
  void HandleBuildPlace(const FInputActionValue &Value);

  /** Rotate structure preview */
  UFUNCTION()
  void HandleBuildRotate(const FInputActionValue &Value);

  /** Toggle grid snap mode */
  UFUNCTION()
  void HandleBuildToggleSnap(const FInputActionValue &Value);

  /** Cancel build mode */
  UFUNCTION()
  void HandleBuildCancel(const FInputActionValue &Value);

  // ========== Train State ==========

  /** Currently controlled train (set when operating console) */
  UPROPERTY()
  TObjectPtr<ATrainActor> ControlledTrain;

public:
  /** Show/hide mouse cursor and set appropriate input mode */
  UFUNCTION(BlueprintCallable, Category = "UI")
  void SetMouseCursorVisible(bool bVisible);

  /** Set the train being controlled (called by TrainConsole) */
  UFUNCTION(BlueprintCallable, Category = "Train")
  void SetControlledTrain(ATrainActor* Train);

  /** Get the currently controlled train */
  UFUNCTION(BlueprintCallable, Category = "Train")
  ATrainActor* GetControlledTrain() const { return ControlledTrain; }

  /** Get building component from controlled pawn */
  UFUNCTION(BlueprintCallable, Category = "Building")
  UBuildingComponent* GetBuildingComponent() const;
};