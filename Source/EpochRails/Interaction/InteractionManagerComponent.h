// InteractionManagerComponent.h
// Manages interaction detection and input for player character

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "InteractionManagerComponent.generated.h"

// Forward declarations
class UInteractableComponent;
class ACharacter;
class APlayerController;
class UCameraComponent;

/**
 * Component that manages interaction detection and input
 * Attach to player Character to enable interaction system
 */
UCLASS(ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class EPOCHRAILS_API UInteractionManagerComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UInteractionManagerComponent();

protected:
  // ========== DETECTION SETTINGS ==========

  /** Use raycast from camera for precise selection */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Interaction|Detection")
  bool bUseRaycast = true;

  /** Maximum raycast distance */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Interaction|Detection",
            meta = (ClampMin = "100.0", ClampMax = "1000.0",
                    EditCondition = "bUseRaycast"))
  float RaycastDistance = 300.0f;

  /** How often to perform raycast check (in seconds, 0 = every frame) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Interaction|Detection",
            meta = (ClampMin = "0.0", ClampMax = "0.5",
                    EditCondition = "bUseRaycast"))
  float RaycastUpdateInterval = 0.1f;

  /** Raycast from screen center offset */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Interaction|Detection")
  FVector2D ScreenCenterOffset = FVector2D(0.0f, 0.0f);

  /** Collision channel for raycast */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Interaction|Detection")
  TEnumAsByte<ECollisionChannel> RaycastChannel = ECC_Visibility;

  /** Draw debug line for raycast */
  UPROPERTY(EditAnywhere, Category = "Interaction|Debug")
  bool bDrawDebugRaycast = false;

  // ========== STATE ==========

  /** All interactables currently in range */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|State")
  TArray<UInteractableComponent *> NearbyInteractables;

  /** Currently focused interactable (closest to screen center) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|State")
  UInteractableComponent *FocusedInteractable = nullptr;

  /** Timer for raycast throttling */
  float RaycastTimer = 0.0f;

  /** Cached owner character */
  UPROPERTY()
  ACharacter *OwnerCharacter = nullptr;

  /** Cached player controller */
  UPROPERTY()
  APlayerController *PlayerController = nullptr;

  /** Cached camera component */
  UPROPERTY()
  UCameraComponent *CameraComponent = nullptr;

  // ========== LIFECYCLE ==========

  virtual void BeginPlay() override;
  virtual void
  TickComponent(float DeltaTime, ELevelTick TickType,
                FActorComponentTickFunction *ThisTickFunction) override;

  // ========== DETECTION LOGIC ==========

  /**
   * Update list of nearby interactables
   * Queries all InteractionTriggers overlapping character
   */
  void UpdateNearbyInteractables();

  /**
   * Select best interactable using raycast from camera
   * Returns interactable closest to screen center
   */
  UInteractableComponent *SelectBestInteractableWithRaycast();

  /**
   * Select best interactable without raycast
   * Returns closest interactable to character
   */
  UInteractableComponent *SelectBestInteractableByDistance();

  /**
   * Perform raycast from camera/screen center
   */
  bool PerformRaycast(FHitResult &OutHit);

  /**
   * Get camera location and forward vector for raycast
   */
  void GetCameraRaycastData(FVector &OutLocation, FVector &OutDirection);

public:
  // ========== PUBLIC API ==========

  /**
   * Handle interaction input (E key pressed)
   * Call this from PlayerController input handler
   */
  UFUNCTION(BlueprintCallable, Category = "Interaction")
  void HandleInteractInput();

  /**
   * Get currently focused interactable
   */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  UInteractableComponent *GetFocusedInteractable() const {
    return FocusedInteractable;
  }

  /**
   * Get all nearby interactables in range
   */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  TArray<UInteractableComponent *> GetNearbyInteractables() const {
    return NearbyInteractables;
  }

  /**
   * Check if there's an interactable in focus
   */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  bool HasFocusedInteractable() const { return FocusedInteractable != nullptr; }

  /**
   * Manually set focused interactable (useful for UI/controllers)
   */
  UFUNCTION(BlueprintCallable, Category = "Interaction")
  void SetFocusedInteractable(UInteractableComponent *Interactable);

  /**
   * Force update of focused interactable
   */
  UFUNCTION(BlueprintCallable, Category = "Interaction")
  void UpdateFocusedInteractable();

  /**
   * Get interaction prompt text from focused interactable
   */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  FText GetInteractionPrompt() const;

  /**
   * Get owner character
   */
  UFUNCTION(BlueprintPure, Category = "Interaction")
  ACharacter *GetOwnerCharacter() const { return OwnerCharacter; }

protected:
  /**
   * Update focused interactable based on detection method
   */
  void UpdateFocus();

  /**
   * Set new focused interactable (handles focus gain/loss)
   */
  void SetFocus(UInteractableComponent *NewFocus);

  /**
   * Clear current focus
   */
  void ClearFocus();
};
