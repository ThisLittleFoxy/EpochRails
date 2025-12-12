// RailsPlayerCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "RailsPlayerCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class ARailsTrain;

UCLASS(config = Game)
class EPOCHRAILS_API ARailsPlayerCharacter : public ACharacter {
  GENERATED_BODY()

public:
  ARailsPlayerCharacter();

  // ========== Components ==========

  /** Camera boom positioning the camera behind character */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera,
            meta = (AllowPrivateAccess = "true"))
  class USpringArmComponent *CameraBoom;

  /** Follow camera */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera,
            meta = (AllowPrivateAccess = "true"))
  UCameraComponent *FollowCamera;

  /** Interaction component for interacting with objects */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
  class UInteractionComponent *InteractionComponent;

  // ========== Input Actions ==========

  /** Move Input Action */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input,
            meta = (AllowPrivateAccess = "true"))
  UInputAction *MoveAction;

  /** Look Input Action */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input,
            meta = (AllowPrivateAccess = "true"))
  UInputAction *LookAction;

  /** Jump Input Action */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input,
            meta = (AllowPrivateAccess = "true"))
  UInputAction *JumpAction;

  /** Interact Input Action */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input,
            meta = (AllowPrivateAccess = "true"))
  UInputAction *InteractAction;

protected:
  // ========== Movement Input Handlers ==========

  /** Called for movement input */
  void Move(const FInputActionValue &Value);

  /** Called for looking input */
  void Look(const FInputActionValue &Value);

  /** Called for interact input */
  void Interact(const FInputActionValue &Value);

  // ========== APawn Interface ==========
protected:
  virtual void SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) override;

  virtual void BeginPlay() override;

  // ========== Train System ==========
protected:
  /** Current train the player is on (null if not on train) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Train")
  ARailsTrain *CurrentTrain;

public:
  /**
   * Get the train the player is currently on
   * @return Current train reference, or nullptr if not on train
   */
  UFUNCTION(BlueprintPure, Category = "Train")
  ARailsTrain *GetCurrentTrain() const { return CurrentTrain; }

  /**
   * Set the current train (called by train when player enters/exits)
   * @param Train - Train reference, or nullptr to clear
   */
  UFUNCTION(BlueprintCallable, Category = "Train")
  void SetCurrentTrain(ARailsTrain *Train);

  /**
   * Check if player is currently on a train
   * @return true if on train, false otherwise
   */
  UFUNCTION(BlueprintPure, Category = "Train")
  bool IsOnTrain() const { return CurrentTrain != nullptr; }
};
