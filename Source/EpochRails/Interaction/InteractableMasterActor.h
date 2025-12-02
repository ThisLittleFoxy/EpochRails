// InteractableMasterActor.h

// Universal interactable actor - supports all interaction types

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractionTypes.h"
#include "InteractableMasterActor.generated.h"

// Forward declarations
class UInteractableComponent;
class USphereComponent;
class UStaticMeshComponent;
class ARailsTrain;

/**
 * Master interactable actor that can represent any type of interactable
 * Add your custom mesh and select interaction type in editor
 * Automatically creates appropriate interactable component
 */
UCLASS(Blueprintable)
class EPOCHRAILS_API AInteractableMasterActor : public AActor {
  GENERATED_BODY()

public:
  AInteractableMasterActor();

protected:
  // ========== COMPONENTS ==========

  /** Root scene component */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USceneComponent *SceneRoot;

  /** Static mesh for visual representation - assign any mesh you want */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UStaticMeshComponent *InteractableMesh;

  /** Interaction trigger zone */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USphereComponent *InteractionTrigger;

  /** Dynamically created interactable component based on type */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UInteractableComponent *InteractableComponent;

  // ========== SETTINGS ==========

  /** Type of interaction this actor provides */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
  EInteractionType InteractionType = EInteractionType::None;

  /** Custom interaction prompt text (optional, uses default if empty) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
  FText CustomInteractionPrompt;

  // ========== DRIVER SEAT SPECIFIC ==========

  /** For DriverSeat: auto-find parent train */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Interactable|Driver Seat")
  bool bAutoFindParentTrain = true;

  /** For DriverSeat: manually assigned train */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Interactable|Driver Seat")
  ARailsTrain *AssignedTrain = nullptr;

  // ========== DOOR SPECIFIC ==========

  /** For Door: animation duration */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Door")
  float DoorAnimationDuration = 1.0f;

  /** For Door: open position offset */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Door")
  FVector DoorOpenOffset = FVector(0, 100, 0);

  // ========== LIFECYCLE ==========

  virtual void OnConstruction(const FTransform &Transform) override;
  virtual void BeginPlay() override;

#if WITH_EDITOR
  virtual void
  PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;
#endif

protected:
  /**
   * Create appropriate interactable component based on type
   */
  void CreateInteractableComponent();

  /**
   * Configure created interactable component with settings
   */
  void ConfigureInteractableComponent();

  /**
   * Clean up old interactable component
   */
  void CleanupInteractableComponent();

public:
  // ========== PUBLIC API ==========

  /** Get the interactable component */
  UFUNCTION(BlueprintPure, Category = "Interactable")
  UInteractableComponent *GetInteractableComponent() const {
    return InteractableComponent;
  }

  /** Get the mesh component */
  UFUNCTION(BlueprintPure, Category = "Interactable")
  UStaticMeshComponent *GetMeshComponent() const { return InteractableMesh; }

  /** Get interaction trigger */
  UFUNCTION(BlueprintPure, Category = "Interactable")
  USphereComponent *GetInteractionTrigger() const { return InteractionTrigger; }
};
