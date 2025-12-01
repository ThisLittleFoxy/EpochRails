// Copyright Epic Games, Inc. All Rights Reserved.

#include "RailsPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EpochRails.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Widgets/Input/SVirtualJoystick.h"

void ARailsPlayerController::BeginPlay() {
  Super::BeginPlay();

  UE_LOG(LogEpochRails, Log,
         TEXT("RailsPlayerController::BeginPlay - Controller: %s"), *GetName());
  UE_LOG(LogEpochRails, Log, TEXT("IsLocalPlayerController: %s"),
         IsLocalPlayerController() ? TEXT("true") : TEXT("false"));
  UE_LOG(LogEpochRails, Log, TEXT("ShouldUseTouchControls: %s"),
         ShouldUseTouchControls() ? TEXT("true") : TEXT("false"));

  // only spawn touch controls on local player controllers
  if (ShouldUseTouchControls() && IsLocalPlayerController()) {
    // spawn the mobile controls widget
    MobileControlsWidget =
        CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);
    if (MobileControlsWidget) {
      // add the controls to the player screen
      MobileControlsWidget->AddToPlayerScreen(0);
      UE_LOG(LogEpochRails, Log,
             TEXT("Mobile controls widget created and added to screen"));
    } else {
      UE_LOG(LogEpochRails, Error,
             TEXT("Could not spawn mobile controls widget."));
    }
  }
}

void ARailsPlayerController::SetupInputComponent() {
  Super::SetupInputComponent();

  UE_LOG(LogEpochRails, Log,
         TEXT("RailsPlayerController::SetupInputComponent - Controller: %s"),
         *GetName());
  UE_LOG(LogEpochRails, Log, TEXT("IsLocalPlayerController: %s"),
         IsLocalPlayerController() ? TEXT("true") : TEXT("false"));
  UE_LOG(LogEpochRails, Log, TEXT("InputComponent valid: %s"),
         InputComponent ? TEXT("true") : TEXT("false"));

  // only add IMCs for local player controllers
  if (IsLocalPlayerController()) {
    // Add Input Mapping Contexts
    if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
                GetLocalPlayer())) {
      UE_LOG(LogEpochRails, Log, TEXT("Enhanced Input Subsystem found"));
      UE_LOG(LogEpochRails, Log, TEXT("DefaultMappingContexts count: %d"),
             DefaultMappingContexts.Num());

      for (int32 i = 0; i < DefaultMappingContexts.Num(); ++i) {
        UInputMappingContext *CurrentContext = DefaultMappingContexts[i];
        if (CurrentContext) {
          Subsystem->AddMappingContext(CurrentContext, 0);
          UE_LOG(LogEpochRails, Log,
                 TEXT("Added DefaultMappingContext [%d]: %s"), i,
                 *CurrentContext->GetName());
        } else {
          UE_LOG(LogEpochRails, Warning,
                 TEXT("DefaultMappingContext [%d] is NULL"), i);
        }
      }

      // only add these IMCs if we're not using mobile touch input
      if (!ShouldUseTouchControls()) {
        UE_LOG(LogEpochRails, Log,
               TEXT("MobileExcludedMappingContexts count: %d"),
               MobileExcludedMappingContexts.Num());

        for (int32 i = 0; i < MobileExcludedMappingContexts.Num(); ++i) {
          UInputMappingContext *CurrentContext =
              MobileExcludedMappingContexts[i];
          if (CurrentContext) {
            Subsystem->AddMappingContext(CurrentContext, 0);
            UE_LOG(LogEpochRails, Log,
                   TEXT("Added MobileExcludedMappingContext [%d]: %s"), i,
                   *CurrentContext->GetName());
          } else {
            UE_LOG(LogEpochRails, Warning,
                   TEXT("MobileExcludedMappingContext [%d] is NULL"), i);
          }
        }
      }
    } else {
      UE_LOG(LogEpochRails, Error,
             TEXT("Failed to get Enhanced Input Subsystem!"));
    }

    // Automatically bind all input actions from mapping contexts
    BindInputActions();
  } else {
    UE_LOG(LogEpochRails, Warning,
           TEXT("Not a local player controller, skipping input setup"));
  }
}

void ARailsPlayerController::BindInputActions() {
  UEnhancedInputComponent *EnhancedInputComponent =
      Cast<UEnhancedInputComponent>(InputComponent);
  if (!EnhancedInputComponent) {
    UE_LOG(LogEpochRails, Error,
           TEXT("Failed to cast InputComponent to EnhancedInputComponent!"));
    return;
  }

  UE_LOG(LogEpochRails, Log,
         TEXT("EnhancedInputComponent cast successful, binding actions from "
              "IMC..."));

  // Collect all mapping contexts
  TArray<UInputMappingContext *> AllContexts;
  AllContexts.Append(DefaultMappingContexts);
  if (!ShouldUseTouchControls()) {
    AllContexts.Append(MobileExcludedMappingContexts);
  }

  // Iterate through all mapping contexts and bind actions
  TSet<const UInputAction *> BoundActions;
  for (UInputMappingContext *Context : AllContexts) {
    if (!Context) {
      continue;
    }

    UE_LOG(LogEpochRails, Log, TEXT("Processing mapping context: %s"),
           *Context->GetName());

    // Get all mappings from the context
    const TArray<FEnhancedActionKeyMapping> &Mappings = Context->GetMappings();
    UE_LOG(LogEpochRails, Log, TEXT("Found %d mappings in context"),
           Mappings.Num());

    for (const FEnhancedActionKeyMapping &Mapping : Mappings) {
      const UInputAction *Action = Mapping.Action;
      if (!Action || BoundActions.Contains(Action)) {
        continue;
      }

      FString ActionName = Action->GetName();
      UE_LOG(LogEpochRails, Log, TEXT("Found Input Action: %s"), *ActionName);

      // Bind based on action name
      if (ActionName.Contains(TEXT("Move")) ||
          ActionName.Contains(TEXT("IA_Move"))) {
        EnhancedInputComponent->BindAction(Action, ETriggerEvent::Triggered,
                                           this, &ARailsPlayerController::Move);
        UE_LOG(LogEpochRails, Log, TEXT("Bound action '%s' to Move handler"),
               *ActionName);
        BoundActions.Add(Action);
      } else if (ActionName.Contains(TEXT("Look")) ||
                 ActionName.Contains(TEXT("IA_Look"))) {
        EnhancedInputComponent->BindAction(Action, ETriggerEvent::Triggered,
                                           this, &ARailsPlayerController::Look);
        UE_LOG(LogEpochRails, Log, TEXT("Bound action '%s' to Look handler"),
               *ActionName);
        BoundActions.Add(Action);
      } else if (ActionName.Contains(TEXT("Jump")) ||
                 ActionName.Contains(TEXT("IA_Jump"))) {
        EnhancedInputComponent->BindAction(Action, ETriggerEvent::Triggered,
                                           this, &ARailsPlayerController::Jump);
        UE_LOG(LogEpochRails, Log, TEXT("Bound action '%s' to Jump handler"),
               *ActionName);
        BoundActions.Add(Action);
      } else {
        UE_LOG(LogEpochRails, Warning, TEXT("No handler found for action: %s"),
               *ActionName);
      }
    }
  }

  UE_LOG(LogEpochRails, Log, TEXT("Total actions bound: %d"),
         BoundActions.Num());
}

void ARailsPlayerController::OnPossess(APawn *InPawn) {
  Super::OnPossess(InPawn);

  UE_LOG(LogEpochRails, Log,
         TEXT("RailsPlayerController::OnPossess - Pawn: %s"),
         InPawn ? *InPawn->GetName() : TEXT("NULL"));

  if (InPawn) {
    ACharacter *PossessedCharacter = Cast<ACharacter>(InPawn);
    if (PossessedCharacter) {
      UE_LOG(LogEpochRails, Log, TEXT("Possessed Character: %s"),
             *PossessedCharacter->GetName());
      if (UCharacterMovementComponent *MovementComp =
              PossessedCharacter->GetCharacterMovement()) {
        UE_LOG(LogEpochRails, Log,
               TEXT("CharacterMovementComponent found, MovementMode: %d"),
               (int32)MovementComp->MovementMode);
      } else {
        UE_LOG(LogEpochRails, Warning,
               TEXT("CharacterMovementComponent not found on Character!"));
      }
    } else {
      UE_LOG(LogEpochRails, Warning,
             TEXT("Possessed pawn is not a Character!"));
    }
  }
}

bool ARailsPlayerController::ShouldUseTouchControls() const {
  // are we on a mobile platform? Should we force touch?
  return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void ARailsPlayerController::Move(const FInputActionValue &Value) {
  UE_LOG(LogEpochRails, Log, TEXT("Move called! Value: %s, Magnitude: %f"),
         *Value.ToString(), Value.GetMagnitude());

  // Get movement vector from input
  const FVector2D MovementVector = Value.Get<FVector2D>();
  UE_LOG(LogEpochRails, Log, TEXT("MovementVector: X=%f, Y=%f"),
         MovementVector.X, MovementVector.Y);

  APawn *ControlledPawn = GetPawn();
  if (!ControlledPawn) {
    UE_LOG(LogEpochRails, Warning, TEXT("Move: No pawn controlled!"));
    return;
  }

  UE_LOG(LogEpochRails, Log, TEXT("Controlled Pawn: %s"),
         *ControlledPawn->GetName());

  // Get forward and right vectors
  const FRotator Rotation = GetControlRotation();
  const FRotator YawRotation(0, Rotation.Yaw, 0);

  const FVector ForwardDirection =
      FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
  const FVector RightDirection =
      FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

  UE_LOG(LogEpochRails, Log, TEXT("ForwardDirection: %s, RightDirection: %s"),
         *ForwardDirection.ToString(), *RightDirection.ToString());

  // Add movement input
  ControlledPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
  ControlledPawn->AddMovementInput(RightDirection, MovementVector.X);

  UE_LOG(LogEpochRails, Log, TEXT("AddMovementInput called"));
}

void ARailsPlayerController::Look(const FInputActionValue &Value) {
  UE_LOG(LogEpochRails, Log, TEXT("Look called! Value: %s, Magnitude: %f"),
         *Value.ToString(), Value.GetMagnitude());

  // Get look axis vector
  const FVector2D LookAxisVector = Value.Get<FVector2D>();
  UE_LOG(LogEpochRails, Log, TEXT("LookAxisVector: X=%f, Y=%f"),
         LookAxisVector.X, LookAxisVector.Y);

  // Add yaw and pitch input
  AddYawInput(LookAxisVector.X);
  AddPitchInput(LookAxisVector.Y);
}

void ARailsPlayerController::Jump(const FInputActionValue &Value) {
  UE_LOG(LogEpochRails, Log, TEXT("Jump called! Value: %s"), *Value.ToString());

  if (ACharacter *ControlledCharacter = Cast<ACharacter>(GetPawn())) {
    ControlledCharacter->Jump();
    UE_LOG(LogEpochRails, Log, TEXT("Jump executed on character"));
  }
}
