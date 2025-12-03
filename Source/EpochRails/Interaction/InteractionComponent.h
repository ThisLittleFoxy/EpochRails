// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class IInteractableInterface;
class ARailsPlayerCharacter;

/**
 * Component that handles interaction detection and execution
 * Add this to your player character to enable interaction with objects
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EPOCHRAILS_API UInteractionComponent : public UActorComponent {
  GENERATED_BODY()

public:
  // Constructor
  UInteractionComponent();

  // Called every frame
  virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                             FActorComponentTickFunction *ThisTickFunction) override;

  /**
   * Attempt to interact with the currently focused object
   * Call this from your input action
   * @return true if interaction was successful
   */
  UFUNCTION(BlueprintCallable, Category = "Interaction")
  bool TryInteract();

  /**
   * Get the currently focused interactable object
   * @return The actor being looked at, or nullptr if none
   */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  AActor *GetFocusedActor() const { return FocusedActor.Get(); }

  /**
   * Check if player is currently looking at an interactable object
   * @return true if an interactable object is in focus
   */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  bool HasFocusedActor() const { return FocusedActor.IsValid(); }

  /**
   * Get the name of the currently focused interactable object
   * @return Display name, or empty text if no object in focus
   */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  FText GetFocusedActorName() const;

  /**
   * Get the interaction action text for the currently focused object
   * @return Action prompt text, or empty text if no object in focus
   */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  FText GetFocusedActorAction() const;

  /**
   * Check if the currently focused object can be interacted with
   * @return true if interaction is currently possible
   */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  bool CanInteractWithFocusedActor() const;

protected:
  // Called when the game starts
  virtual void BeginPlay() override;

  /**
   * Perform line trace to detect interactable objects
   * @param OutHitResult - The hit result if trace was successful
   * @return true if trace hit something
   */
  bool PerformInteractionTrace(FHitResult &OutHitResult);

  /**
   * Update the currently focused actor
   * Handles focus begin/end events
   * @param NewFocusActor - The new actor to focus on (can be nullptr)
   */
  void UpdateFocusedActor(AActor *NewFocusActor);

protected:
  // ========== Interaction Settings ==========

  /** Maximum distance for interaction detection (in cm) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  float DefaultInteractionDistance = 300.0f;

  /** How often to check for interactable objects (in seconds) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  float InteractionCheckFrequency = 0.1f;

  /** Collision channel to use for interaction traces */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  TEnumAsByte<ECollisionChannel> InteractionTraceChannel = ECC_Visibility;

  /** Enable debug visualization for interaction traces */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Debug")
  bool bShowDebugTrace = false;

  /** Duration to show debug lines (in seconds) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Debug",
            meta = (EditCondition = "bShowDebugTrace"))
  float DebugTraceDuration = 0.1f;

private:
  /** Currently focused interactable actor */
  TWeakObjectPtr<AActor> FocusedActor;

  /** Timer for interaction checks */
  float InteractionCheckTimer;

  /** Cached player character reference */
  UPROPERTY()
  ARailsPlayerCharacter *OwningCharacter;
};
