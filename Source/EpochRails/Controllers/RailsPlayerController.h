// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RailsPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;

/**
 * Basic PlayerController class for a third person game
 * Manages input mappings
 */
UCLASS(abstract)
class ARailsPlayerController : public APlayerController {
  GENERATED_BODY()

protected:
  /** Input Mapping Contexts */
  UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
  TArray<UInputMappingContext *> DefaultMappingContexts;

  /** Input Mapping Contexts */
  UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
  TArray<UInputMappingContext *> MobileExcludedMappingContexts;

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

  /** Gameplay initialization */
  virtual void BeginPlay() override;

  /** Input mapping context setup */
  virtual void SetupInputComponent() override;

  /** Called after possession */
  virtual void OnPossess(APawn *InPawn) override;

  /** Returns true if the player should use UMG touch controls */
  bool ShouldUseTouchControls() const;

  /** Bind all input actions from mapping contexts automatically */
  void BindInputActions();

  /** Movement input handler */
  UFUNCTION()
  void Move(const FInputActionValue &Value);

  /** Look input handler */
  UFUNCTION()
  void Look(const FInputActionValue &Value);

  /** Jump input handler */
  UFUNCTION()
  void Jump(const FInputActionValue &Value);
};