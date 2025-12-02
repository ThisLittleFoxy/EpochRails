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

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 * A simple player-controllable third person character
 * Implements a controllable orbiting camera
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

public:
  /** Returns CameraBoom subobject **/
  FORCEINLINE class USpringArmComponent *GetCameraBoom() const {
    return CameraBoom;
  }

  /** Returns FollowCamera subobject **/
  FORCEINLINE class UCameraComponent *GetFollowCamera() const {
    return FollowCamera;
  }

  /** Returns whether character is sprinting */
  FORCEINLINE bool IsSprinting() const { return bIsSprinting; }

  /** Returns current speed */
  FORCEINLINE float GetCurrentSpeed() const { return CurrentSpeed; }

  /** Returns movement direction */
  FORCEINLINE float GetMovementDirection() const { return MovementDirection; }

  /** Returns whether character is in air */
  FORCEINLINE bool IsInAir() const { return bIsInAir; }
};