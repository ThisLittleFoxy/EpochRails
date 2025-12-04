// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/RailsPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EpochRails.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Train/RailsTrain.h"
#include "Interaction/InteractionComponent.h"
#include "Interaction/RailsTrainSeat.h"  // путь может быть другим
#include "Interaction/InteractableInterface.h"

ARailsPlayerCharacter::ARailsPlayerCharacter() {
  // Set this character to call Tick() every frame
  PrimaryActorTick.bCanEverTick = true;

  // Set size for collision capsule
  GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

  // Don't rotate when the controller rotates. Let that just affect the camera.
  bUseControllerRotationPitch = false;
  bUseControllerRotationYaw = false;
  bUseControllerRotationRoll = false;

  // Configure character movement
  GetCharacterMovement()->bOrientRotationToMovement = true;
  GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
  GetCharacterMovement()->JumpZVelocity = 500.f;
  GetCharacterMovement()->AirControl = 0.35f;
  GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
  GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
  GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
  GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

  // Create a camera boom (pulls in towards the player if there is a collision)
  CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
  CameraBoom->SetupAttachment(RootComponent);
  CameraBoom->TargetArmLength = 400.0f;
  CameraBoom->bUsePawnControlRotation = true;

  // Create a follow camera
  FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
  FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
  FollowCamera->bUsePawnControlRotation = false;

  // Create interaction component
  InteractionComponent =
      CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
}

void ARailsPlayerCharacter::BeginPlay() {
  Super::BeginPlay();

  // Setup camera attachment based on configuration
  SetupCameraAttachment();

  // Initialize movement speed
  if (UCharacterMovementComponent *MovementComp = GetCharacterMovement()) {
    MovementComp->MaxWalkSpeed = WalkSpeed;
  }
}

void ARailsPlayerCharacter::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  // Update animation variables every frame
  UpdateAnimationVariables();
}

void ARailsPlayerCharacter::UpdateAnimationVariables() {
  if (UCharacterMovementComponent *MovementComp = GetCharacterMovement()) {
    // Calculate current speed
    FVector Velocity = GetVelocity();
    Velocity.Z = 0.0f;
    CurrentSpeed = Velocity.Size();

    // Check if in air
    bIsInAir = MovementComp->IsFalling();

    // Calculate movement direction relative to actor rotation
    if (CurrentSpeed > 0.0f) {
      FRotator VelocityRotation = Velocity.Rotation();
      FRotator ActorRotation = GetActorRotation();
      float DeltaAngle = VelocityRotation.Yaw - ActorRotation.Yaw;

      // Normalize angle to -180 to 180 range
      MovementDirection = FMath::UnwindDegrees(DeltaAngle);
    } else {
      MovementDirection = 0.0f;
    }
  }
}

void ARailsPlayerCharacter::SetupCameraAttachment() {
  if (!CameraBoom || !GetMesh()) {
    return;
  }

  // If we should attach to a socket
  if (bAttachCameraToSocket && !CameraSocketName.IsNone()) {
    UE_LOG(LogEpochRails, Log, TEXT("Attaching camera boom to socket: %s"),
           *CameraSocketName.ToString());

    // Define attachment rules
    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget,
                                          bIgnoreSocketRotation
                                              ? EAttachmentRule::KeepWorld
                                              : EAttachmentRule::SnapToTarget,
                                          EAttachmentRule::KeepWorld, false);

    // Attach to mesh socket
    CameraBoom->AttachToComponent(GetMesh(), AttachRules, CameraSocketName);

    // Apply custom offsets
    CameraBoom->SetRelativeLocation(CameraRelativeLocationOffset);
    CameraBoom->SetRelativeRotation(CameraRelativeRotationOffset);

    UE_LOG(LogEpochRails, Log,
           TEXT("Camera boom attached to socket with offset: %s, rotation: %s"),
           *CameraRelativeLocationOffset.ToString(),
           *CameraRelativeRotationOffset.ToString());
  } else {
    UE_LOG(LogEpochRails, Log,
           TEXT("Camera boom attached to root component (default)"));
  }
}

void ARailsPlayerCharacter::SetCameraSocket(FName NewSocketName,
                                            bool bIgnoreRotation) {
  if (!CameraBoom || !GetMesh()) {
    UE_LOG(LogEpochRails, Warning,
           TEXT("Cannot change camera socket: CameraBoom or Mesh is null"));
    return;
  }

  CameraSocketName = NewSocketName;
  bIgnoreSocketRotation = bIgnoreRotation;

  if (NewSocketName.IsNone()) {
    ResetCameraToDefault();
    return;
  }

  UE_LOG(LogEpochRails, Log, TEXT("Changing camera socket to: %s"),
         *NewSocketName.ToString());

  FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget,
                                        bIgnoreRotation
                                            ? EAttachmentRule::KeepWorld
                                            : EAttachmentRule::SnapToTarget,
                                        EAttachmentRule::KeepWorld, false);

  CameraBoom->DetachFromComponent(
      FDetachmentTransformRules::KeepWorldTransform);
  CameraBoom->AttachToComponent(GetMesh(), AttachRules, NewSocketName);
  CameraBoom->SetRelativeLocation(CameraRelativeLocationOffset);
  CameraBoom->SetRelativeRotation(CameraRelativeRotationOffset);

  bAttachCameraToSocket = true;

  UE_LOG(LogEpochRails, Log, TEXT("Camera socket changed successfully"));
}

void ARailsPlayerCharacter::ResetCameraToDefault() {
  if (!CameraBoom) {
    return;
  }

  UE_LOG(LogEpochRails, Log,
         TEXT("Resetting camera to default (root component)"));

  FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget,
                                        EAttachmentRule::KeepWorld,
                                        EAttachmentRule::KeepWorld, false);

  CameraBoom->DetachFromComponent(
      FDetachmentTransformRules::KeepWorldTransform);
  CameraBoom->AttachToComponent(RootComponent, AttachRules);
  CameraBoom->SetRelativeLocation(FVector::ZeroVector);
  CameraBoom->SetRelativeRotation(FRotator::ZeroRotator);

  bAttachCameraToSocket = false;
  CameraSocketName = NAME_None;

  UE_LOG(LogEpochRails, Log, TEXT("Camera reset to default"));
}

void ARailsPlayerCharacter::SetupPlayerInputComponent(
    UInputComponent *PlayerInputComponent) {
  if (UEnhancedInputComponent *EnhancedInputComponent =
          Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
    // Jumping
    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this,
                                       &ACharacter::Jump);
    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed,
                                       this, &ACharacter::StopJumping);

    // Moving
    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered,
                                       this, &ARailsPlayerCharacter::Move);

    // Looking
    EnhancedInputComponent->BindAction(MouseLookAction,
                                       ETriggerEvent::Triggered, this,
                                       &ARailsPlayerCharacter::Look);
    EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered,
                                       this, &ARailsPlayerCharacter::Look);

    // Sprinting
    if (SprintAction) {
      EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started,
                                         this,
                                         &ARailsPlayerCharacter::StartSprint);
      EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed,
                                         this,
                                         &ARailsPlayerCharacter::StopSprint);
      UE_LOG(LogEpochRails, Log, TEXT("Sprint action bound successfully"));
    } else {
      UE_LOG(LogEpochRails, Warning,
             TEXT("SprintAction is NULL! Please assign it in Blueprint."));
    }

    // Interacting
    if (InteractAction) {
      EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started,
                                         this,
                                         &ARailsPlayerCharacter::Interact);
      UE_LOG(LogEpochRails, Log, TEXT("Interact action bound successfully"));
    } else {
      UE_LOG(LogEpochRails, Warning,
             TEXT("InteractAction is NULL! Please assign it in Blueprint."));
    }

    // Train Control - Throttle
    if (ThrottleAction) {
      EnhancedInputComponent->BindAction(
          ThrottleAction, ETriggerEvent::Triggered, this,
          &ARailsPlayerCharacter::OnThrottleInput);
      EnhancedInputComponent->BindAction(
          ThrottleAction, ETriggerEvent::Completed, this,
          &ARailsPlayerCharacter::OnThrottleInput);
      UE_LOG(LogEpochRails, Log, TEXT("Throttle action bound successfully"));
    } else {
      UE_LOG(LogEpochRails, Warning,
             TEXT("ThrottleAction is NULL! Please assign it in Blueprint."));
    }

    // Train Control - Brake
    if (BrakeAction) {
      EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered,
                                         this,
                                         &ARailsPlayerCharacter::OnBrakeInput);
      EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed,
                                         this,
                                         &ARailsPlayerCharacter::OnBrakeInput);
      UE_LOG(LogEpochRails, Log, TEXT("Brake action bound successfully"));
    } else {
      UE_LOG(LogEpochRails, Warning,
             TEXT("BrakeAction is NULL! Please assign it in Blueprint."));
    }
  } else {
    UE_LOG(LogEpochRails, Error,
           TEXT("'%s' Failed to find an Enhanced Input component!"),
           *GetNameSafe(this));
  }
}

void ARailsPlayerCharacter::StartSprint(const FInputActionValue &Value) {
  DoStartSprint();
}

void ARailsPlayerCharacter::StopSprint(const FInputActionValue &Value) {
  DoStopSprint();
}

void ARailsPlayerCharacter::Interact(const FInputActionValue &Value) {
  DoInteract();
}

void ARailsPlayerCharacter::DoStartSprint() {
  if (UCharacterMovementComponent *MovementComp = GetCharacterMovement()) {
    bIsSprinting = true;
    MovementComp->MaxWalkSpeed = SprintSpeed;
    UE_LOG(LogEpochRails, Log, TEXT("Sprint started - Speed: %f"), SprintSpeed);
  }
}

void ARailsPlayerCharacter::DoStopSprint() {
  if (UCharacterMovementComponent *MovementComp = GetCharacterMovement()) {
    bIsSprinting = false;
    MovementComp->MaxWalkSpeed = WalkSpeed;
    UE_LOG(LogEpochRails, Log, TEXT("Sprint stopped - Speed: %f"), WalkSpeed);
  }
}

void ARailsPlayerCharacter::DoInteract() {
  // Priority 1: If sitting in a seat, exit it
  if (CurrentSeat) {
    // Use Execute_ prefix for interface functions
    IInteractableInterface::Execute_OnInteract(CurrentSeat, this);
    return;
  }

  // Priority 2: Normal interaction with world objects
  if (InteractionComponent) {
    bool bSuccess = InteractionComponent->TryInteract();
    UE_LOG(LogEpochRails, Log, TEXT("Interaction attempt: %s"),
           bSuccess ? TEXT("Success") : TEXT("Failed"));
  }
}



void ARailsPlayerCharacter::Move(const FInputActionValue &Value) {
  FVector2D MovementVector = Value.Get<FVector2D>();
  DoMove(MovementVector.X, MovementVector.Y);
}

void ARailsPlayerCharacter::Look(const FInputActionValue &Value) {
  FVector2D LookAxisVector = Value.Get<FVector2D>();
  DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ARailsPlayerCharacter::DoMove(float Right, float Forward) {
  if (GetController() != nullptr) {
    const FRotator Rotation = GetController()->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);

    const FVector ForwardDirection =
        FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection =
        FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    AddMovementInput(ForwardDirection, Forward);
    AddMovementInput(RightDirection, Right);
  }
}

void ARailsPlayerCharacter::DoLook(float Yaw, float Pitch) {
  if (GetController() != nullptr) {
    AddControllerYawInput(Yaw);
    AddControllerPitchInput(Pitch);
  }
}

void ARailsPlayerCharacter::DoJumpStart() { Jump(); }

void ARailsPlayerCharacter::DoJumpEnd() { StopJumping(); }

void ARailsPlayerCharacter::OnThrottleInput(const FInputActionValue &Value) {
  if (ControlledTrain) {
    float ThrottleValue = Value.Get<float>();
    ControlledTrain->ApplyThrottle(ThrottleValue);

    UE_LOG(LogEpochRails, Verbose, TEXT("Throttle input: %f"), ThrottleValue);
  }
}

void ARailsPlayerCharacter::OnBrakeInput(const FInputActionValue &Value) {
  if (ControlledTrain) {
    float BrakeValue = Value.Get<float>();
    ControlledTrain->ApplyBrake(BrakeValue);

    UE_LOG(LogEpochRails, Verbose, TEXT("Brake input: %f"), BrakeValue);
  }
}
