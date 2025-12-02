// InteractableComponent.h
// Base component for all interactable objects

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "InteractionTypes.h"
#include "InteractableComponent.generated.h"

// Forward declarations
class USphereComponent;
class UPrimitiveComponent;
class ACharacter;

/**
 * Delegate for interaction events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionEvent, ACharacter *,
                                            Character);

/**
 * Base component that makes any actor interactable
 * Attach this to StaticMesh, SkeletalMesh, or any Actor to enable interaction
 */
UCLASS(ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class EPOCHRAILS_API UInteractableComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UInteractableComponent();

protected:
  // ========== CORE SETTINGS ==========

  /** Type of interaction this component provides */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  EInteractionType InteractionType = EInteractionType::Custom;

  /** Interaction settings (radius, raycast, etc.) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  FInteractionSettings Settings;

  /** Animation settings for interaction */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Interaction|Animation")
  FInteractionAnimationSettings AnimationSettings;

  // ========== COMPONENTS ==========

  /** Trigger sphere for detecting nearby players */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USphereComponent *InteractionTrigger;

  // ========== STATE ==========

  /** Is this interactable currently enabled */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|State")
  bool bIsEnabled = true;

  /** Is this interactable currently focused by player */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|State")
  bool bIsFocused = false;

  /** Is this interactable currently being interacted with */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|State")
  bool bIsInteracting = false;

  /** Character currently interacting with this object */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|State")
  ACharacter *InteractingCharacter = nullptr;

  // ========== VISUAL SETTINGS ==========

  /** Component to highlight (auto-detected if null) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Visual")
  UPrimitiveComponent *HighlightComponent;

  /** Enable debug visualization in editor */
  UPROPERTY(EditAnywhere, Category = "Interaction|Debug")
  bool bShowDebugSphere = true;

  // ========== LIFECYCLE ==========

  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  // ========== TRIGGER EVENTS ==========

  UFUNCTION()
  void OnTriggerBeginOverlap(UPrimitiveComponent *OverlappedComponent,
                             AActor *OtherActor, UPrimitiveComponent *OtherComp,
                             int32 OtherBodyIndex, bool bFromSweep,
                             const FHitResult &SweepResult);

  UFUNCTION()
  void OnTriggerEndOverlap(UPrimitiveComponent *OverlappedComponent,
                           AActor *OtherActor, UPrimitiveComponent *OtherComp,
                           int32 OtherBodyIndex);

public:
  // ========== EVENTS (BlueprintAssignable) ==========

  /** Called when interaction starts */
  UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
  FOnInteractionEvent OnInteractionStarted;

  /** Called when interaction ends */
  UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
  FOnInteractionEvent OnInteractionEnded;

  /** Called when player focuses on this interactable */
  UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
  FOnInteractionEvent OnFocusGained;

  /** Called when player unfocuses from this interactable */
  UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
  FOnInteractionEvent OnFocusLost;

  // ========== INTERACTION API ==========

  /**
   * Main interaction function - called when player presses E
   * Override in child classes for specific behavior
   */
  UFUNCTION(BlueprintCallable, Category = "Interaction")
  virtual void Interact(ACharacter *Character);

  /**
   * Blueprint implementable event for custom interaction logic
   */
  UFUNCTION(BlueprintImplementableEvent, Category = "Interaction",
            meta = (DisplayName = "On Interact"))
  void OnInteractBP(ACharacter *Character);

  /**
   * Can this interactable be used right now?
   */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  virtual bool CanInteract(ACharacter *Character) const;

  /**
   * Blueprint implementable condition check
   */
  UFUNCTION(BlueprintImplementableEvent, BlueprintPure,
            Category = "Interaction", meta = (DisplayName = "Can Interact"))
  bool CanInteractBP(ACharacter *Character) const;

  // ========== FOCUS MANAGEMENT ==========

  /**
   * Set focus state (called by InteractionManager)
   */
  UFUNCTION(BlueprintCallable, Category = "Interaction")
  void SetFocused(bool bFocused, ACharacter *Character);

  /**
   * Enable/disable highlight on component
   */
  UFUNCTION(BlueprintCallable, Category = "Interaction")
  void SetHighlighted(bool bHighlight);

  // ========== QUERY FUNCTIONS ==========

  /** Get interaction type */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  EInteractionType GetInteractionType() const { return InteractionType; }

  /** Get interaction prompt text */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  FText GetInteractionPrompt() const { return Settings.InteractionPrompt; }

  /** Is this interactable currently enabled */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  bool IsEnabled() const { return bIsEnabled; }

  /** Is this interactable currently focused */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  bool IsFocused() const { return bIsFocused; }

  /** Is someone currently interacting with this */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  bool IsInteracting() const { return bIsInteracting; }

  /** Get interaction settings */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  FInteractionSettings GetSettings() const { return Settings; }

  /** Get trigger component */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  USphereComponent *GetTrigger() const { return InteractionTrigger; }

  /** Get the character currently interacting */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  ACharacter *GetInteractingCharacter() const { return InteractingCharacter; }

  // ========== UTILITY ==========

  /**
   * Check if character is on a moving train
   */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  bool IsCharacterOnMovingTrain(ACharacter *Character) const;

  /**
   * Auto-detect primitive component for highlighting
   */
  UFUNCTION(BlueprintCallable, Category = "Interaction")
  void AutoDetectHighlightComponent();

protected:
  /**
   * Internal function to start interaction
   */
  virtual void StartInteraction(ACharacter *Character);

  /**
   * Internal function to end interaction
   */
  virtual void EndInteraction(ACharacter *Character);
};
