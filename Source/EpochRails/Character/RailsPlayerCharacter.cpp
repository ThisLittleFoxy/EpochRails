// RailsPlayerCharacter.cpp
#include "RailsPlayerCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Interaction/InteractionComponent.h"
#include "Train/RailsTrain.h"

ARailsPlayerCharacter::ARailsPlayerCharacter() {
  // Set size for collision capsule
  GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

  // Don't rotate when the controller rotates. Let that just affect the camera.
  bUseControllerRotationPitch = false;
  bUseControllerRotationYaw = false;
  bUseControllerRotationRoll = false;

  // Configure character movement
  GetCharacterMovement()->bOrientRotationToMovement = true;
  GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

  GetCharacterMovement()->JumpZVelocity = 700.f;
  GetCharacterMovement()->AirControl = 0.35f;
  GetCharacterMovement()->MaxWalkSpeed = 500.f;
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
  InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));

  // Initialize train reference
  CurrentTrain = nullptr;
}

void ARailsPlayerCharacter::BeginPlay() {
  Super::BeginPlay();
}

void ARailsPlayerCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) {
  // Add Input Mapping Context
  if (APlayerController *PlayerController = Cast<APlayerController>(GetController())) {
    if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
                PlayerController->GetLocalPlayer())) {
      // IMC will be managed by train system, so we don't add it here
    }
  }

  // Set up action bindings
  if (UEnhancedInputComponent *EnhancedInputComponent =
          Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
    // Moving
    if (MoveAction) {
      EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this,
                                         &ARailsPlayerCharacter::Move);
    }

    // Looking
    if (LookAction) {
      EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this,
                                         &ARailsPlayerCharacter::Look);
    }

    // Jumping
    if (JumpAction) {
      EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this,
                                         &ACharacter::Jump);
      EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this,
                                         &ACharacter::StopJumping);
    }

    // Interacting
    if (InteractAction) {
      EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this,
                                         &ARailsPlayerCharacter::Interact);
    }
  }
}

void ARailsPlayerCharacter::Move(const FInputActionValue &Value) {
  // Input is a Vector2D
  FVector2D MovementVector = Value.Get<FVector2D>();

  if (Controller != nullptr) {
    // Find out which way is forward
    const FRotator Rotation = Controller->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);

    // Get forward vector
    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

    // Get right vector
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    // Add movement
    AddMovementInput(ForwardDirection, MovementVector.Y);
    AddMovementInput(RightDirection, MovementVector.X);
  }
}

void ARailsPlayerCharacter::Look(const FInputActionValue &Value) {
  // Input is a Vector2D
  FVector2D LookAxisVector = Value.Get<FVector2D>();

  if (Controller != nullptr) {
    // Add yaw and pitch input to controller
    AddControllerYawInput(LookAxisVector.X);
    AddControllerPitchInput(LookAxisVector.Y);
  }
}

void ARailsPlayerCharacter::Interact(const FInputActionValue &Value) {
  if (InteractionComponent) {
    InteractionComponent->TryInteract();
  }
}

void ARailsPlayerCharacter::SetCurrentTrain(ARailsTrain *Train) {
  CurrentTrain = Train;

  if (Train) {
    UE_LOG(LogTemp, Log, TEXT("Player %s now on train: %s"), *GetName(),
           *Train->GetName());
  } else {
    UE_LOG(LogTemp, Log, TEXT("Player %s no longer on train"), *GetName());
  }
}
