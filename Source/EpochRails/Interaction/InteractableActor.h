// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "InteractableActor.generated.h"

class UStaticMeshComponent;
class ARailsPlayerCharacter;

/**
 * Base class for interactable actors
 * Extend this in Blueprint to create custom interactable objects
 * Provides default implementation of InteractableInterface with Blueprint events
 */
UCLASS(Blueprintable)
class EPOCHRAILS_API AInteractableActor : public AActor,
                                          public IInteractableInterface {
  GENERATED_BODY()

public:
  // Constructor
  AInteractableActor();

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

public:
  // ========== IInteractableInterface Implementation ==========

  /**
   * Called when player starts looking at this object
   * Override in Blueprint for custom behavior
   */
  virtual void OnInteractionFocusBegin_Implementation(
      ARailsPlayerCharacter *PlayerCharacter) override;

  /**
   * Called when player stops looking at this object
   * Override in Blueprint for custom behavior
   */
  virtual void OnInteractionFocusEnd_Implementation(
      ARailsPlayerCharacter *PlayerCharacter) override;

  /**
   * Called when player interacts with this object
   * Override in Blueprint for custom interaction logic
   * @return true if interaction was successful
   */
  virtual bool OnInteract_Implementation(
      ARailsPlayerCharacter *PlayerCharacter) override;

  /**
   * Get the name to display when player looks at this object
   */
  virtual FText GetInteractionName_Implementation() const override;

  /**
   * Get the action text to display (e.g., "Press E to open")
   */
  virtual FText GetInteractionAction_Implementation() const override;

  /**
   * Check if this object can currently be interacted with
   */
  virtual bool CanInteract_Implementation(
      ARailsPlayerCharacter *PlayerCharacter) const override;

  /**
   * Get the maximum interaction distance
   */
  virtual float GetInteractionDistance_Implementation() const override;

protected:
  // ========== Blueprint Events ==========

  /**
   * Blueprint event called when player starts looking at this object
   */
  UFUNCTION(BlueprintImplementableEvent, Category = "Interaction",
            meta = (DisplayName = "On Focus Begin"))
  void BP_OnInteractionFocusBegin(ARailsPlayerCharacter *PlayerCharacter);

  /**
   * Blueprint event called when player stops looking at this object
   */
  UFUNCTION(BlueprintImplementableEvent, Category = "Interaction",
            meta = (DisplayName = "On Focus End"))
  void BP_OnInteractionFocusEnd(ARailsPlayerCharacter *PlayerCharacter);

  /**
   * Blueprint event called when player interacts with this object
   * Return true if interaction was successful
   */
  UFUNCTION(BlueprintImplementableEvent, Category = "Interaction",
            meta = (DisplayName = "On Interact"))
  bool BP_OnInteract(ARailsPlayerCharacter *PlayerCharacter);

protected:
  // ========== Components ==========

  /** Root scene component */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USceneComponent *SceneRoot;

  /** Static mesh component (optional, for visual representation) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UStaticMeshComponent *MeshComponent;

  // ========== Interaction Settings ==========

  /** Display name for this interactable object */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  FText InteractionName = FText::FromString(TEXT("Interactable Object"));

  /** Action text to display (e.g., "Open", "Use", "Pickup") */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  FText InteractionAction = FText::FromString(TEXT("Interact"));

  /** Can this object currently be interacted with? */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  bool bCanInteract = true;

  /** Maximum distance from which this object can be interacted with (in cm) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  float MaxInteractionDistance = 300.0f;

  /** Enable debug logging for this interactable */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Debug")
  bool bEnableDebugLog = false;
};
