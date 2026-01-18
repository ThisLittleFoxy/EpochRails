// Copyright Epic Games, Inc. All Rights Reserved.

#include "RailsPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Building/BuildingComponent.h"
#include "Character/ControllableCharacterInterface.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EpochRails.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Train/TrainActor.h"
#include "Widgets/Input/SVirtualJoystick.h"

// FIXED CONSTRUCTOR
ARailsPlayerController::ARailsPlayerController() {
  // Enable widget interaction
  bShowMouseCursor = false;
  bEnableClickEvents = true;
  bEnableTouchEvents = true;
  bEnableMouseOverEvents = false;
  bEnableTouchOverEvents = false;
}

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
                                           this,
                                           &ARailsPlayerController::HandleMove);
        UE_LOG(LogEpochRails, Log, TEXT("Bound action '%s' to HandleMove"),
               *ActionName);
        BoundActions.Add(Action);
      } else if (ActionName.Contains(TEXT("Look")) ||
                 ActionName.Contains(TEXT("IA_Look"))) {
        EnhancedInputComponent->BindAction(Action, ETriggerEvent::Triggered,
                                           this,
                                           &ARailsPlayerController::HandleLook);
        UE_LOG(LogEpochRails, Log, TEXT("Bound action '%s' to HandleLook"),
               *ActionName);
        BoundActions.Add(Action);
      } else if (ActionName.Contains(TEXT("Jump")) ||
                 ActionName.Contains(TEXT("IA_Jump"))) {
        EnhancedInputComponent->BindAction(
            Action, ETriggerEvent::Started, this,
            &ARailsPlayerController::HandleJumpStarted);
        EnhancedInputComponent->BindAction(
            Action, ETriggerEvent::Completed, this,
            &ARailsPlayerController::HandleJumpCompleted);
        UE_LOG(LogEpochRails, Log, TEXT("Bound action '%s' to HandleJump"),
               *ActionName);
        BoundActions.Add(Action);
      } else if (ActionName.Contains(TEXT("Sprint")) ||
                 ActionName.Contains(TEXT("IA_Sprint"))) {
        EnhancedInputComponent->BindAction(
            Action, ETriggerEvent::Started, this,
            &ARailsPlayerController::HandleSprintStarted);
        EnhancedInputComponent->BindAction(
            Action, ETriggerEvent::Completed, this,
            &ARailsPlayerController::HandleSprintCompleted);
        UE_LOG(LogEpochRails, Log, TEXT("Bound action '%s' to HandleSprint"),
               *ActionName);
        BoundActions.Add(Action);
      } else if (ActionName.Contains(TEXT("Interact")) ||
                 ActionName.Contains(TEXT("IA_Interact"))) {
        EnhancedInputComponent->BindAction(
            Action, ETriggerEvent::Started, this,
            &ARailsPlayerController::HandleInteract);
        UE_LOG(LogEpochRails, Log, TEXT("Bound action '%s' to HandleInteract"),
               *ActionName);
        BoundActions.Add(Action);
      } else if (ActionName.Contains(TEXT("Fire")) ||
                 ActionName.Contains(TEXT("IA_Fire"))) {
        EnhancedInputComponent->BindAction(
            Action, ETriggerEvent::Started, this,
            &ARailsPlayerController::HandleFireStarted);
        EnhancedInputComponent->BindAction(
            Action, ETriggerEvent::Completed, this,
            &ARailsPlayerController::HandleFireCompleted);
        UE_LOG(LogEpochRails, Log, TEXT("Bound action '%s' to HandleFire"),
               *ActionName);
        BoundActions.Add(Action);
      }
      // ========== Train Control ==========
      else if (ActionName.Contains(TEXT("TrainThrottle")) ||
               ActionName.Contains(TEXT("IA_TrainThrottle"))) {
        EnhancedInputComponent->BindAction(
            Action, ETriggerEvent::Triggered, this,
            &ARailsPlayerController::HandleTrainThrottle);
        UE_LOG(LogEpochRails, Log, TEXT("Bound action '%s' to HandleTrainThrottle"),
               *ActionName);
        BoundActions.Add(Action);
      } else if (ActionName.Contains(TEXT("TrainBrake")) ||
                 ActionName.Contains(TEXT("IA_TrainBrake"))) {
        EnhancedInputComponent->BindAction(
            Action, ETriggerEvent::Triggered, this,
            &ARailsPlayerController::HandleTrainBrake);
        UE_LOG(LogEpochRails, Log, TEXT("Bound action '%s' to HandleTrainBrake"),
               *ActionName);
        BoundActions.Add(Action);
      } else if (ActionName.Contains(TEXT("TrainReverse")) ||
                 ActionName.Contains(TEXT("IA_TrainReverse"))) {
        EnhancedInputComponent->BindAction(
            Action, ETriggerEvent::Started, this,
            &ARailsPlayerController::HandleTrainReverse);
        UE_LOG(LogEpochRails, Log, TEXT("Bound action '%s' to HandleTrainReverse"),
               *ActionName);
        BoundActions.Add(Action);
      } else if (ActionName.Contains(TEXT("TrainEmergency")) ||
                 ActionName.Contains(TEXT("IA_TrainEmergencyBrake"))) {
        EnhancedInputComponent->BindAction(
            Action, ETriggerEvent::Started, this,
            &ARailsPlayerController::HandleTrainEmergencyBrake);
        UE_LOG(LogEpochRails, Log, TEXT("Bound action '%s' to HandleTrainEmergencyBrake"),
               *ActionName);
        BoundActions.Add(Action);
      }
      // ========== Building ==========
      else if (ActionName.Contains(TEXT("ToggleBuildMode")) ||
               ActionName.Contains(TEXT("IA_ToggleBuildMode"))) {
        EnhancedInputComponent->BindAction(
            Action, ETriggerEvent::Started, this,
            &ARailsPlayerController::HandleToggleBuildMode);
        UE_LOG(LogEpochRails, Log, TEXT("Bound action '%s' to HandleToggleBuildMode"),
               *ActionName);
        BoundActions.Add(Action);
      } else if (ActionName.Contains(TEXT("BuildPlace")) ||
                 ActionName.Contains(TEXT("IA_BuildPlace"))) {
        EnhancedInputComponent->BindAction(
            Action, ETriggerEvent::Started, this,
            &ARailsPlayerController::HandleBuildPlace);
        UE_LOG(LogEpochRails, Log, TEXT("Bound action '%s' to HandleBuildPlace"),
               *ActionName);
        BoundActions.Add(Action);
      } else if (ActionName.Contains(TEXT("BuildRotate")) ||
                 ActionName.Contains(TEXT("IA_BuildRotate"))) {
        EnhancedInputComponent->BindAction(
            Action, ETriggerEvent::Triggered, this,
            &ARailsPlayerController::HandleBuildRotate);
        UE_LOG(LogEpochRails, Log, TEXT("Bound action '%s' to HandleBuildRotate"),
               *ActionName);
        BoundActions.Add(Action);
      } else if (ActionName.Contains(TEXT("BuildToggleSnap")) ||
                 ActionName.Contains(TEXT("IA_BuildToggleSnap"))) {
        EnhancedInputComponent->BindAction(
            Action, ETriggerEvent::Started, this,
            &ARailsPlayerController::HandleBuildToggleSnap);
        UE_LOG(LogEpochRails, Log, TEXT("Bound action '%s' to HandleBuildToggleSnap"),
               *ActionName);
        BoundActions.Add(Action);
      } else if (ActionName.Contains(TEXT("BuildCancel")) ||
                 ActionName.Contains(TEXT("IA_BuildCancel"))) {
        EnhancedInputComponent->BindAction(
            Action, ETriggerEvent::Started, this,
            &ARailsPlayerController::HandleBuildCancel);
        UE_LOG(LogEpochRails, Log, TEXT("Bound action '%s' to HandleBuildCancel"),
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

  UE_LOG(LogEpochRails, Verbose,
         TEXT("RailsPlayerController::OnPossess - Pawn: %s"),
         InPawn ? *InPawn->GetName() : TEXT("NULL"));

  if (InPawn) {
    ACharacter *PossessedCharacter = Cast<ACharacter>(InPawn);
    if (PossessedCharacter) {
      UE_LOG(LogEpochRails, Verbose, TEXT("Possessed Character: %s"),
             *PossessedCharacter->GetName());
      if (UCharacterMovementComponent *MovementComp =
              PossessedCharacter->GetCharacterMovement()) {
        UE_LOG(LogEpochRails, Verbose,
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

// ========== Movement Handlers ==========

void ARailsPlayerController::HandleMove(const FInputActionValue &Value) {
  const FVector2D MovementVector = Value.Get<FVector2D>();

  APawn *ControlledPawn = GetPawn();
  if (!ControlledPawn) {
    return;
  }

  // Calculate direction based on controller rotation (FPS style)
  const FRotator Rotation = GetControlRotation();
  const FRotator YawRotation(0, Rotation.Yaw, 0);

  const FVector ForwardDirection =
      FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
  const FVector RightDirection =
      FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

  // Apply movement input
  ControlledPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
  ControlledPawn->AddMovementInput(RightDirection, MovementVector.X);
}

void ARailsPlayerController::HandleLook(const FInputActionValue &Value) {
  const FVector2D LookAxisVector = Value.Get<FVector2D>();

  // Add yaw and pitch input to controller
  AddYawInput(LookAxisVector.X);
  AddPitchInput(LookAxisVector.Y);
}

// ========== Jump Handlers ==========

void ARailsPlayerController::HandleJumpStarted(const FInputActionValue &Value) {
  if (ACharacter *ControlledCharacter = Cast<ACharacter>(GetPawn())) {
    ControlledCharacter->Jump();
  }
}

void ARailsPlayerController::HandleJumpCompleted(
    const FInputActionValue &Value) {
  if (ACharacter *ControlledCharacter = Cast<ACharacter>(GetPawn())) {
    ControlledCharacter->StopJumping();
  }
}

// ========== Sprint Handlers ==========

void ARailsPlayerController::HandleSprintStarted(
    const FInputActionValue &Value) {
  APawn *ControlledPawn = GetPawn();
  if (!ControlledPawn) {
    return;
  }

  // Call interface method if pawn implements it
  if (ControlledPawn->Implements<UControllableCharacterInterface>()) {
    IControllableCharacterInterface::Execute_StartSprint(ControlledPawn);
    UE_LOG(LogEpochRails, Log, TEXT("Sprint started via interface"));
  }
}

void ARailsPlayerController::HandleSprintCompleted(
    const FInputActionValue &Value) {
  APawn *ControlledPawn = GetPawn();
  if (!ControlledPawn) {
    return;
  }

  // Call interface method if pawn implements it
  if (ControlledPawn->Implements<UControllableCharacterInterface>()) {
    IControllableCharacterInterface::Execute_StopSprint(ControlledPawn);
    UE_LOG(LogEpochRails, Log, TEXT("Sprint stopped via interface"));
  }
}

// ========== Interact Handler ==========

void ARailsPlayerController::HandleInteract(const FInputActionValue &Value) {
  APawn *ControlledPawn = GetPawn();
  if (!ControlledPawn) {
    return;
  }

  // Call interface method if pawn implements it
  if (ControlledPawn->Implements<UControllableCharacterInterface>()) {
    IControllableCharacterInterface::Execute_DoInteract(ControlledPawn);
    UE_LOG(LogEpochRails, Log, TEXT("Interact triggered via interface"));
  }
}

// ========== Fire Handlers ==========

void ARailsPlayerController::HandleFireStarted(const FInputActionValue &Value) {
  APawn *ControlledPawn = GetPawn();
  if (!ControlledPawn) {
    return;
  }

  // Call interface method if pawn implements it
  if (ControlledPawn->Implements<UControllableCharacterInterface>()) {
    IControllableCharacterInterface::Execute_StartFire(ControlledPawn);
    UE_LOG(LogEpochRails, Log, TEXT("Fire started via interface"));
  }
}

void ARailsPlayerController::HandleFireCompleted(
    const FInputActionValue &Value) {
  APawn *ControlledPawn = GetPawn();
  if (!ControlledPawn) {
    return;
  }

  // Call interface method if pawn implements it
  if (ControlledPawn->Implements<UControllableCharacterInterface>()) {
    IControllableCharacterInterface::Execute_StopFire(ControlledPawn);
    UE_LOG(LogEpochRails, Log, TEXT("Fire stopped via interface"));
  }
}

void ARailsPlayerController::SetMouseCursorVisible(bool bVisible) {
  bShowMouseCursor = bVisible;
  bIsInteractingWithUI = bVisible;

  // Set input mode based on cursor visibility
  if (bVisible) {
    // Allow both game and UI input
    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);

    UE_LOG(LogEpochRails, Log, TEXT("Mouse cursor enabled for UI interaction"));
  } else {
    // Game input only
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);

    UE_LOG(LogEpochRails, Log, TEXT("Mouse cursor disabled, game mode active"));
  }
}

// ========== Train Control Handlers ==========

void ARailsPlayerController::HandleTrainThrottle(const FInputActionValue &Value) {
  if (!ControlledTrain) {
    return;
  }

  float ThrottleValue = Value.Get<float>();
  ControlledTrain->SetThrottle(ThrottleValue);
}

void ARailsPlayerController::HandleTrainBrake(const FInputActionValue &Value) {
  if (!ControlledTrain) {
    return;
  }

  float BrakeValue = Value.Get<float>();
  ControlledTrain->SetBrake(BrakeValue);
}

void ARailsPlayerController::HandleTrainReverse(const FInputActionValue &Value) {
  if (!ControlledTrain) {
    return;
  }

  ControlledTrain->ToggleReverse();
  UE_LOG(LogEpochRails, Log, TEXT("Train reverse toggled"));
}

void ARailsPlayerController::HandleTrainEmergencyBrake(const FInputActionValue &Value) {
  if (!ControlledTrain) {
    return;
  }

  // Toggle emergency brake
  if (ControlledTrain->MovementComponent &&
      ControlledTrain->MovementComponent->IsEmergencyBrakeActive()) {
    ControlledTrain->ReleaseEmergencyBrake();
    UE_LOG(LogEpochRails, Log, TEXT("Emergency brake released"));
  } else {
    ControlledTrain->EmergencyBrake();
    UE_LOG(LogEpochRails, Log, TEXT("Emergency brake activated"));
  }
}

void ARailsPlayerController::SetControlledTrain(ATrainActor* Train) {
  ControlledTrain = Train;
  UE_LOG(LogEpochRails, Log, TEXT("Controlled train set to: %s"),
         Train ? *Train->GetName() : TEXT("None"));
}

// ========== Building Handlers ==========

UBuildingComponent* ARailsPlayerController::GetBuildingComponent() const {
  APawn* ControlledPawn = GetPawn();
  if (!ControlledPawn) {
    return nullptr;
  }

  return ControlledPawn->FindComponentByClass<UBuildingComponent>();
}

void ARailsPlayerController::HandleToggleBuildMode(const FInputActionValue &Value) {
  UBuildingComponent* BuildComp = GetBuildingComponent();
  if (!BuildComp) {
    UE_LOG(LogEpochRails, Warning, TEXT("No BuildingComponent found on pawn"));
    return;
  }

  BuildComp->ToggleBuildMode();
}

void ARailsPlayerController::HandleBuildPlace(const FInputActionValue &Value) {
  UBuildingComponent* BuildComp = GetBuildingComponent();
  if (!BuildComp || !BuildComp->IsInBuildMode()) {
    return;
  }

  if (BuildComp->ConfirmPlacement()) {
    UE_LOG(LogEpochRails, Log, TEXT("Structure placed successfully"));
  }
}

void ARailsPlayerController::HandleBuildRotate(const FInputActionValue &Value) {
  UBuildingComponent* BuildComp = GetBuildingComponent();
  if (!BuildComp || !BuildComp->IsInBuildMode()) {
    return;
  }

  float RotateValue = Value.Get<float>();
  BuildComp->RotatePreview(RotateValue * 15.0f); // 15 degrees per input tick
}

void ARailsPlayerController::HandleBuildToggleSnap(const FInputActionValue &Value) {
  UBuildingComponent* BuildComp = GetBuildingComponent();
  if (!BuildComp || !BuildComp->IsInBuildMode()) {
    return;
  }

  BuildComp->TogglePlacementMode();
  UE_LOG(LogEpochRails, Log, TEXT("Build snap mode toggled"));
}

void ARailsPlayerController::HandleBuildCancel(const FInputActionValue &Value) {
  UBuildingComponent* BuildComp = GetBuildingComponent();
  if (!BuildComp) {
    return;
  }

  BuildComp->CancelPlacement();
  UE_LOG(LogEpochRails, Log, TEXT("Build mode cancelled"));
}