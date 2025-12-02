// InteractableDriverSeat.h
// Driver seat interactable - seat with train control capability

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableSeat.h"
#include "InteractableDriverSeat.generated.h"

// Forward declarations
class ARailsTrain;
class UInputMappingContext;
class APlayerController;

/**
 * Delegate for driver seat events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriverSeatEvent, ACharacter *,
                                             Character, ARailsTrain *, Train);

/**
 * Driver seat interactable component
 * Extends seat functionality with train control capability
 * Automatically enables train control IMC when character sits down
 */
UCLASS(ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class EPOCHRAILS_API UInteractableDriverSeat : public UInteractableSeat {
  GENERATED_BODY()

public:
  UInteractableDriverSeat();

protected:
  // ========== DRIVER SEAT SETTINGS ==========

  /** Auto-find parent train on BeginPlay */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Seat")
  bool bAutoFindParentTrain = true;

  /** Manually assigned train to control (if not auto-finding) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Seat",
            meta = (EditCondition = "!bAutoFindParentTrain"))
  ARailsTrain *AssignedTrain = nullptr;

  /** Input Mapping Context for train controls (optional, can be set in
   * controller) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Seat|Input")
  UInputMappingContext *TrainControlIMC = nullptr;

  /** Priority for train control IMC */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Seat|Input")
  int32 IMCPriority = 1;

  /** Automatically enable train controls when sitting */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Seat")
  bool bAutoEnableTrainControls = true;

  /** Show train HUD/UI when driving */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Seat|UI")
  bool bShowTrainHUD = false;

  /** Train HUD widget class */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver Seat|UI",
            meta = (EditCondition = "bShowTrainHUD"))
  TSubclassOf<UUserWidget> TrainHUDClass;

  // ========== STATE ==========

  /** Train controlled by this driver seat */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Driver Seat|State")
  ARailsTrain *ControlledTrain = nullptr;

  /** Is train control currently active */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Driver Seat|State")
  bool bIsControllingTrain = false;

  /** Train HUD widget instance */
  UPROPERTY()
  UUserWidget *TrainHUDWidget = nullptr;

  /** Cached player controller */
  UPROPERTY()
  APlayerController *CachedController = nullptr;

  // ========== LIFECYCLE ==========

  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  // ========== TRAIN CONTROL LOGIC ==========

  /**
   * Enable train controls for character
   */
  UFUNCTION(BlueprintCallable, Category = "Driver Seat")
  virtual void EnableTrainControls(ACharacter *Character);

  /**
   * Disable train controls for character
   */
  UFUNCTION(BlueprintCallable, Category = "Driver Seat")
  virtual void DisableTrainControls(ACharacter *Character);

  /**
   * Find parent train actor
   */
  UFUNCTION(BlueprintCallable, Category = "Driver Seat")
  ARailsTrain *FindParentTrain() const;

  /**
   * Show train HUD
   */
  UFUNCTION(BlueprintCallable, Category = "Driver Seat")
  void ShowTrainHUD();

  /**
   * Hide train HUD
   */
  UFUNCTION(BlueprintCallable, Category = "Driver Seat")
  void HideTrainHUD();

public:
  // ========== EVENTS ==========

  /** Called when character starts controlling train */
  UPROPERTY(BlueprintAssignable, Category = "Driver Seat|Events")
  FOnDriverSeatEvent OnStartedControllingTrain;

  /** Called when character stops controlling train */
  UPROPERTY(BlueprintAssignable, Category = "Driver Seat|Events")
  FOnDriverSeatEvent OnStoppedControllingTrain;

  // ========== OVERRIDES ==========

  virtual void SitDown(ACharacter *Character) override;
  virtual void StandUp(ACharacter *Character) override;
  virtual bool CanInteract(ACharacter *Character) const override;

  // ========== QUERY API ==========

  /** Is train control currently active */
  UFUNCTION(BlueprintPure, Category = "Driver Seat")
  bool IsControllingTrain() const { return bIsControllingTrain; }

  /** Get controlled train */
  UFUNCTION(BlueprintPure, Category = "Driver Seat")
  ARailsTrain *GetControlledTrain() const { return ControlledTrain; }

  /** Set controlled train manually */
  UFUNCTION(BlueprintCallable, Category = "Driver Seat")
  void SetControlledTrain(ARailsTrain *Train);

protected:
  /**
   * Add train control IMC to player input
   */
  void AddTrainControlIMC(APlayerController *Controller);

  /**
   * Remove train control IMC from player input
   */
  void RemoveTrainControlIMC(APlayerController *Controller);

  /**
   * Update character animation state for driving
   */
  virtual void UpdateCharacterAnimationState(ACharacter *Character,
                                             bool bSitting) override;
};
