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
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    /** Follow camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

protected:
    /** Socket name to attach camera boom to. Leave empty to attach to root component */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FName CameraSocketName;

    /** If true, camera boom will attach to mesh socket instead of root component */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    bool bAttachCameraToSocket = false;

    /** Custom relative location offset for camera boom when attached to socket */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FVector CameraRelativeLocationOffset = FVector::ZeroVector;

    /** Custom relative rotation offset for camera boom when attached to socket */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FRotator CameraRelativeRotationOffset = FRotator::ZeroRotator;

    /** If true, camera boom ignores socket rotation and uses world/pawn rotation instead */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    bool bIgnoreSocketRotation = true;

    /** Jump Input Action */
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* JumpAction;

    /** Move Input Action */
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* MoveAction;

    /** Look Input Action */
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* LookAction;

    /** Mouse Look Input Action */
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* MouseLookAction;

public:
    /** Constructor */
    ARailsPlayerCharacter();

protected:
    /** Called when the game starts or when spawned */
    virtual void BeginPlay() override;

    /** Initialize input action bindings */
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    /** Setup camera attachment based on settings */
    virtual void SetupCameraAttachment();

protected:
    /** Called for movement input */
    void Move(const FInputActionValue& Value);

    /** Called for looking input */
    void Look(const FInputActionValue& Value);

public:
    /** Dynamically change camera socket at runtime */
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraSocket(FName NewSocketName, bool bIgnoreRotation = true);

    /** Reset camera to default attachment (root component) */
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void ResetCameraToDefault();

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
    FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    
    /** Returns FollowCamera subobject **/
    FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
