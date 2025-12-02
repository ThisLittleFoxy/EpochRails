// InteractableSeat.h
// Seat interactable - allows character to sit down

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableComponent.h"
#include "InteractableSeat.generated.h"

// Forward declarations
class USceneComponent;
class ACharacter;

/**
 * Delegate for seat events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSeatEvent, ACharacter *,
                                            Character);

/**
 * Interactable component for seats
 * Allows characters to sit down and attach to seat socket
 */
UCLASS(ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class EPOCHRAILS_API UInteractableSeat : public UInteractableComponent {
  GENERATED_BODY()

public:
  UInteractableSeat();

protected:
  // ========== SEAT SETTINGS ==========

  /** Socket name on owner mesh for character positioning */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seat")
  FName SeatSocketName = TEXT("SeatSocket");

  /** If no socket found, use this offset from actor location */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seat")
  FVector SeatOffset = FVector(0.0f, 0.0f, 50.0f);

  /** If no socket found, use this rotation offset */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seat")
  FRotator SeatRotation = FRotator::ZeroRotator;

  /** Disable character movement when sitting */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seat")
  bool bDisableMovementWhenSitting = true;

  /** Disable character collision when sitting */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seat")
  bool bDisableCollisionWhenSitting = false;

  /** Allow standing up while moving (train/vehicle) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seat")
  bool bCanStandWhileMoving = true;

  // ========== STATE ==========

  /** Is this seat currently occupied */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Seat|State")
  bool bIsOccupied = false;

  /** Character currently sitting in this seat */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Seat|State")
  ACharacter *CurrentOccupant = nullptr;

  /** Cached scene component for socket attachment */
  UPROPERTY()
  USceneComponent *SeatComponent = nullptr;

  /** Previous movement mode before sitting */
  uint8 PreviousMovementMode = 0;

  // ========== LIFECYCLE ==========

  virtual void BeginPlay() override;

  // ========== SEAT LOGIC ==========

  /**
   * Make character sit down in this seat
   */
  UFUNCTION(BlueprintCallable, Category = "Seat")
  virtual void SitDown(ACharacter *Character);

  /**
   * Make character stand up from this seat
   */
  UFUNCTION(BlueprintCallable, Category = "Seat")
  virtual void StandUp(ACharacter *Character);

  /**
   * Get socket transform for character positioning
   */
  UFUNCTION(BlueprintPure, Category = "Seat")
  FTransform GetSeatTransform() const;

  /**
   * Find scene component with socket
   */
  UFUNCTION(BlueprintCallable, Category = "Seat")
  void FindSeatComponent();

public:
  // ========== EVENTS ==========

  /** Called when character sits down */
  UPROPERTY(BlueprintAssignable, Category = "Seat|Events")
  FOnSeatEvent OnCharacterSatDown;

  /** Called when character stands up */
  UPROPERTY(BlueprintAssignable, Category = "Seat|Events")
  FOnSeatEvent OnCharacterStoodUp;

  // ========== OVERRIDES ==========

  virtual void Interact(ACharacter *Character) override;
  virtual bool CanInteract(ACharacter *Character) const override;

  // ========== QUERY API ==========

  /** Is this seat occupied */
  UFUNCTION(BlueprintPure, Category = "Seat")
  bool IsOccupied() const { return bIsOccupied; }

  /** Get current occupant */
  UFUNCTION(BlueprintPure, Category = "Seat")
  ACharacter *GetOccupant() const { return CurrentOccupant; }

  /** Check if character is sitting in this seat */
  UFUNCTION(BlueprintPure, Category = "Seat")
  bool IsCharacterSitting(ACharacter *Character) const;

protected:
  /**
   * Setup character for sitting
   */
  virtual void SetupSittingCharacter(ACharacter *Character);

  /**
   * Restore character after standing up
   */
  virtual void RestoreStandingCharacter(ACharacter *Character);

  /**
   * Update character animation variables
   */
  virtual void UpdateCharacterAnimationState(ACharacter *Character,
                                             bool bSitting);
};
