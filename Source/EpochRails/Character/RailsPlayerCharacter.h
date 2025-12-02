// RailsPlayerCharacter.h
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "RailsPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
class UInteractionManagerComponent;  // ADD THIS
enum class EInteractionType : uint8; // ADD THIS

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 * A simple player-controllable third person character
 * Implements a controllable orbiting camera and interaction system
 */
UCLASS(abstract)
class ARailsPlayerCharacter : public ACharacter {
  GENERATED_BODY()

  /** Camera boom positioning the camera behind the character */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
            meta = (AllowPrivateAccess = "true"))
  USpringArmComponent *CameraBoom;

  /** Follow camera */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
            meta = (AllowPrivateAccess = "true"))
  UCameraComponent *FollowCamera;

  /** Interaction manager component (ADD THIS) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
            meta = (AllowPrivateAccess = "true"))
  UInteractionManagerComponent *InteractionManager;

protected:
  // ========== Camera Settings ==========

  /** Socket name to attach camera boom to. Leave empty to attach to root
   * component */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
  FName CameraSocketName;

  /** If true, camera boom will attach to mesh socket instead of root component
   */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
  bool bAttachCameraToSocket = false;

  /** Custom relative location offset for camera boom when attached to socket */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
  FVector CameraRelativeLocationOffset = FVector::ZeroVector;

  /** Custom relative rotation offset for camera boom when attached to socket */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
  FRotator CameraRelativeRotationOffset = FRotator::ZeroRotator;

  /** If true, camera boom ignores socket rotation and uses world/pawn rotation
   * instead */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
  bool bIgnoreSocketRotation = true;

  // ========== Movement Settings ==========

  /** Normal walking speed */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sprint")
  float WalkSpeed = 500.0f;

  /** Sprint speed */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sprint")
  float SprintSpeed = 800.0f;

  // ========== Input Actions ==========

  /** Jump Input Action */
  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction *JumpAction;

  /** Move Input Action */
  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction *MoveAction;

  /** Look Input Action */
  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction *LookAction;

  /** Mouse Look Input Action */
  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction *MouseLookAction;

  /** Sprint Input Action */
  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction *SprintAction;

  /** Interact Input Action (ADD THIS) */
  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction *InteractAction;

public:
  // ========== Animation Variables (PUBLIC for AnimBP access) ==========

  /** Is the character currently sprinting? (for Animation Blueprint) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Animation")
  bool bIsSprinting = false;

  /** Current speed of the character (for Animation Blueprint) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Animation")
  float CurrentSpeed = 0.0f;

  /** Current movement direction (for Animation Blueprint) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Animation")
  float MovementDirection = 0.0f;

  /** Is the character in the air? (for Animation Blueprint) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Animation")
  bool bIsInAir = false;

  // ========== Interaction Animation Variables (ADD ALL OF THESE) ==========

  /** Is character currently sitting (for Animation Blueprint) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
            Category = "Interaction|Animation")
  bool bIsSitting = false;

  /** Is character currently interacting with something (for Animation
   * Blueprint) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
            Category = "Interaction|Animation")
  bool bIsInteracting = false;

  /** Is character currently controlling train (for Animation Blueprint) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
            Category = "Interaction|Animation")
  bool bIsControllingTrain = false;

  /** Current interaction type (for Animation Blueprint) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
            Category = "Interaction|Animation")
  EInteractionType CurrentInteractionType;

  /** Actor currently being interacted with (for Animation Blueprint) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
            Category = "Interaction|Animation")
  AActor *CurrentInteractedActor = nullptr;

public:
  /** Constructor */
  ARailsPlayerCharacter();

protected:
  /** Called when the game starts or when spawned */
  virtual void BeginPlay() override;

  /** Called every frame */
  virtual void Tick(float DeltaTime) override;

  /** Initialize input action bindings */
  virtual void SetupPlayerInputComponent(
      class UInputComponent *PlayerInputComponent) override;

  /** Setup camera attachment based on settings */
  virtual void SetupCameraAttachment();

  /** Update animation variables */
  virtual void UpdateAnimationVariables();

protected:
  /** Called for movement input */
  void Move(const FInputActionValue &Value);

  /** Called for looking input */
  void Look(const FInputActionValue &Value);

  /** Called when sprint is started */
  void StartSprint(const FInputActionValue &Value);

  /** Called when sprint is stopped */
  void StopSprint(const FInputActionValue &Value);

  /** Called when interact key is pressed (ADD THIS) */
  void Interact(const FInputActionValue &Value);

public:
  /** Dynamically change camera socket at runtime */
  UFUNCTION(BlueprintCallable, Category = "Camera")
  void SetCameraSocket(FName NewSocketName, bool bIgnoreRotation = true);

  /** Reset camera to default attachment (root component) */
  UFUNCTION(BlueprintCallable, Category = "Camera")
  void ResetCameraToDefault();

  /** Start sprinting */
  UFUNCTION(BlueprintCallable, Category = "Movement")
  void DoStartSprint();

  /** Stop sprinting */
  UFUNCTION(BlueprintCallable, Category = "Movement")
  void DoStopSprint();

  /** Handles move inputs from either controls or UI interfaces */
  UFUNCTION(BlueprintCallable, Category = "Input")
  virtual void DoMove(float Right, float Forward);

  /** Handles look inputs from either controls or UI interfaces */
  UFUNCTION(BlueprintCallable, Category = "Input")
  virtual void DoLook(float Yaw, float Pitch);

  /** Handles jump pressed inputs from either controls or UI interfaces */
  UFUNCTION(BlueprintCallable, Category = "Input")
  virtual void DoJumpStart();

  /** Handles jump pressed inputs from either controls or UI interfaces */
  UFUNCTION(BlueprintCallable, Category = "Input")
  virtual void DoJumpEnd();

  /** Handle interact input (ADD THIS) */
  UFUNCTION(BlueprintCallable, Category = "Interaction")
  virtual void DoInteract();

public:
  /** Returns CameraBoom subobject **/
  FORCEINLINE class USpringArmComponent *GetCameraBoom() const {
    return CameraBoom;
  }

  /** Returns FollowCamera subobject **/
  FORCEINLINE class UCameraComponent *GetFollowCamera() const {
    return FollowCamera;
  }

  /** Returns InteractionManager subobject (ADD THIS) **/
  FORCEINLINE class UInteractionManagerComponent *
  GetInteractionManager() const {
    return InteractionManager;
  }

  /** Returns whether character is sprinting */
  FORCEINLINE bool IsSprinting() const { return bIsSprinting; }

  /** Returns current speed */
  FORCEINLINE float GetCurrentSpeed() const { return CurrentSpeed; }

  /** Returns movement direction */
  FORCEINLINE float GetMovementDirection() const { return MovementDirection; }

  /** Returns whether character is in air */
  FORCEINLINE bool IsInAir() const { return bIsInAir; }

  /** Returns whether character is sitting (ADD THIS) */
  FORCEINLINE bool IsSitting() const { return bIsSitting; }

  /** Returns whether character is interacting (ADD THIS) */
  FORCEINLINE bool IsInteracting() const { return bIsInteracting; }

  /** Returns whether character is controlling train (ADD THIS) */
  FORCEINLINE bool IsControllingTrain() const { return bIsControllingTrain; }
};
