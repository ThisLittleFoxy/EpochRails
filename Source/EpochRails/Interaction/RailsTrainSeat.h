// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "RailsTrainSeat.generated.h"

class ARailsTrain;
class ARailsPlayerCharacter;
class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;
class UAnimMontage;

/**
 * Driver seat used to control the train.
 * Player can interact with this seat to start/stop driving the train.
 * Uses custom three-stage animation system: sit down -> idle sitting -> stand
 * up
 */
UCLASS()
class EPOCHRAILS_API ARailsTrainSeat : public AActor,
                                       public IInteractableInterface {
  GENERATED_BODY()

public:
  ARailsTrainSeat();

  virtual ~ARailsTrainSeat();

protected:
  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
  // ========== IInteractableInterface Implementation ==========

  /**
   * Called when player starts looking at this seat
   */
  virtual void OnInteractionFocusBegin_Implementation(
      ARailsPlayerCharacter *PlayerCharacter) override;

  /**
   * Called when player stops looking at this seat
   */
  virtual void OnInteractionFocusEnd_Implementation(
      ARailsPlayerCharacter *PlayerCharacter) override;

  /**
   * Called when player interacts with this seat
   */
  virtual bool
  OnInteract_Implementation(ARailsPlayerCharacter *PlayerCharacter) override;

  /**
   * Get the name to display when player looks at this seat
   */
  virtual FText GetInteractionName_Implementation() const override;

  /**
   * Get the action text to display
   */
  virtual FText GetInteractionAction_Implementation() const override;

  /**
   * Check if this seat can currently be interacted with
   */
  virtual bool CanInteract_Implementation(
      ARailsPlayerCharacter *PlayerCharacter) const override;

  /**
   * Get the maximum interaction distance
   */
  virtual float GetInteractionDistance_Implementation() const override;

protected:
  // ========== Components ==========

  /** Root scene component */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USceneComponent *SceneRoot;

  /** Static mesh component for seat visual representation */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UStaticMeshComponent *MeshComponent;

  /** Point where player will be positioned when seated */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USceneComponent *SeatPoint;

  // ========== Interaction Settings ==========

  /** Display name for this seat */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  FText InteractionName = FText::FromString(TEXT("Train Driver Seat"));

  /** Action text to display */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  FText InteractionAction = FText::FromString(TEXT("Sit"));

  /** Can this seat currently be interacted with? */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  bool bCanInteract = true;

  /** Maximum interaction distance (in cm) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  float MaxInteractionDistance = 200.0f;

  /** Enable debug logging */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Debug")
  bool bEnableDebugLog = false;

  // ========== Train Control Settings ==========

  /** Train that will be controlled from this seat */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train")
  ARailsTrain *ControlledTrain;

  /** Default input mapping context used for regular gameplay */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
  UInputMappingContext *DefaultInputMappingContext;

  /** Input mapping context used while controlling the train */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
  UInputMappingContext *TrainControlInputMappingContext;

  /** Priority for default IMC */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
  int32 DefaultIMCPriority = 0;

  /** Priority for train control IMC */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
  int32 TrainControlIMCPriority = 1;

  /** Whether player is currently seated and controlling the train */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Train")
  bool bIsPlayerSeated;

  /** Cached reference to the seated player */
  UPROPERTY()
  ARailsPlayerCharacter *SeatedPlayer;

  /** Should character mesh be hidden while seated? */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Visibility")
  bool bHideCharacterWhileSeated = false;

  /** Should character be attached to seat point? */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Attachment")
  bool bAttachCharacterToSeat = true;

  // ========== Animation Settings ==========

  /** Animation to play when sitting down */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Animation")
  UAnimMontage *SitDownAnimationMontage = nullptr;

  /** Animation to play when standing up */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Animation")
  UAnimMontage *StandUpAnimationMontage = nullptr;

  /** Idle sitting pose (looping animation while seated) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Animation")
  UAnimMontage *SittingIdleAnimationMontage = nullptr;

  /** Should play sit down animation? */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Animation")
  bool bPlaySitDownAnimation = true;

  /** Should play stand up animation? */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Animation")
  bool bPlayStandUpAnimation = true;

  /** Should play idle sitting animation while seated? */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Animation")
  bool bPlaySittingIdleAnimation = true;

  /** Animation play rate for sit/stand animations */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train|Animation",
            meta = (ClampMin = "0.1", ClampMax = "5.0"))
  float SeatAnimationPlayRate = 1.0f;

protected:
  // ========== Seat Control Methods ==========

  /**
   * Put player into the seat and enable train control
   * Called after sit down animation completes (or immediately if no animation)
   */
  UFUNCTION(BlueprintCallable, Category = "Train")
  void SeatPlayer(ARailsPlayerCharacter *PlayerCharacter);

  /**
   * Remove player from the seat and restore default controls
   * Called after stand up animation completes (or immediately if no animation)
   */
  UFUNCTION(BlueprintCallable, Category = "Train")
  void UnseatPlayer();

  // ========== Animation Methods ==========

  /**
   * Play sit down animation and call completion when done
   */
  UFUNCTION(BlueprintCallable, Category = "Train|Animation")
  void PlaySitDownAnimation(ARailsPlayerCharacter *PlayerCharacter);

  /**
   * Play stand up animation and call completion when done
   */
  UFUNCTION(BlueprintCallable, Category = "Train|Animation")
  void PlayStandUpAnimation(ARailsPlayerCharacter *PlayerCharacter);

  /**
   * Play looping sitting idle animation
   */
  UFUNCTION(BlueprintCallable, Category = "Train|Animation")
  void PlaySittingIdleAnimation(ARailsPlayerCharacter *PlayerCharacter);

  /**
   * Stop all seat-related animations
   */
  UFUNCTION(BlueprintCallable, Category = "Train|Animation")
  void StopSeatAnimations(ARailsPlayerCharacter *PlayerCharacter);

  /**
   * Called after sit down animation completes
   */
  UFUNCTION()
  void OnSitDownAnimationComplete();

  /**
   * Called after stand up animation completes
   */
  UFUNCTION()
  void OnStandUpAnimationComplete();

  // ========== Helper Methods ==========

  /**
   * Helper to switch input mapping contexts
   */
  void ApplyInputMappingContexts(ARailsPlayerCharacter *PlayerCharacter,
                                 bool bUseTrainControl);

  /**
   * Helper to get Enhanced Input subsystem for a player
   */
  UEnhancedInputLocalPlayerSubsystem *
  GetInputSubsystem(ARailsPlayerCharacter *PlayerCharacter) const;

private:
  /** Timer handle for sit down animation completion */
  FTimerHandle SitDownAnimationTimerHandle;

  /** Timer handle for stand up animation completion */
  FTimerHandle StandUpAnimationTimerHandle;

  /** Is currently playing sit down animation? */
  bool bIsPlayingSitDownAnimation = false;

  /** Is currently playing stand up animation? */
  bool bIsPlayingStandUpAnimation = false;
};
