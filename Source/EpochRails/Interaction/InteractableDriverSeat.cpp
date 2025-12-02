// InteractableDriverSeat.cpp
// Implementation of driver seat interactable component

#include "Interaction/InteractableDriverSeat.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputSubsystems.h"
#include "EpochRails.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "Train/RailsTrain.h"
#include "Character/RailsPlayerCharacter.h"  // ADD THIS LINE

UInteractableDriverSeat::UInteractableDriverSeat() {
  // Set driver seat-specific defaults
  InteractionType = EInteractionType::DriverSeat;
  Settings.InteractionPrompt = FText::FromString("Press E to drive");
}

void UInteractableDriverSeat::BeginPlay() {
  Super::BeginPlay();

  // Find parent train if auto-find enabled
  if (bAutoFindParentTrain) {
    ControlledTrain = FindParentTrain();
    if (ControlledTrain) {
      UE_LOG(LogEpochRails, Log, TEXT("Driver seat found parent train: %s"),
             *ControlledTrain->GetName());
    } else {
      UE_LOG(LogEpochRails, Warning,
             TEXT("Driver seat could not find parent train on %s"),
             *GetOwner()->GetName());
    }
  } else if (AssignedTrain) {
    ControlledTrain = AssignedTrain;
    UE_LOG(LogEpochRails, Log, TEXT("Driver seat using assigned train: %s"),
           *ControlledTrain->GetName());
  }

  if (!ControlledTrain) {
    UE_LOG(LogEpochRails, Error,
           TEXT("Driver seat has no controlled train! Component on: %s"),
           *GetOwner()->GetName());
  }
}

void UInteractableDriverSeat::EndPlay(
    const EEndPlayReason::Type EndPlayReason) {
  // Clean up train controls
  if (bIsControllingTrain && CurrentOccupant) {
    DisableTrainControls(CurrentOccupant);
  }

  // Hide HUD
  HideTrainHUD();

  Super::EndPlay(EndPlayReason);
}

// ========== TRAIN CONTROL LOGIC ==========

void UInteractableDriverSeat::EnableTrainControls(ACharacter *Character) {
  if (!Character) {
    UE_LOG(LogEpochRails, Warning, TEXT("EnableTrainControls: null character"));
    return;
  }

  if (!ControlledTrain) {
    UE_LOG(LogEpochRails, Warning,
           TEXT("EnableTrainControls: no controlled train"));
    return;
  }

  if (bIsControllingTrain) {
    UE_LOG(LogEpochRails, Warning, TEXT("Train controls already enabled"));
    return;
  }

  // Get player controller
  CachedController = Cast<APlayerController>(Character->GetController());
  if (!CachedController) {
    UE_LOG(LogEpochRails, Warning,
           TEXT("EnableTrainControls: character has no PlayerController"));
    return;
  }

  UE_LOG(LogEpochRails, Log, TEXT("Enabling train controls for %s on train %s"),
         *Character->GetName(), *ControlledTrain->GetName());

  // Add train control IMC if specified
  if (TrainControlIMC) {
    AddTrainControlIMC(CachedController);
  }

  // Set controlling state
  bIsControllingTrain = true;

  // Show HUD if enabled
  if (bShowTrainHUD) {
    ShowTrainHUD();
  }

  // Update animation state
  UpdateCharacterAnimationState(Character, true);

  // Broadcast event
  OnStartedControllingTrain.Broadcast(Character, ControlledTrain);
}

void UInteractableDriverSeat::DisableTrainControls(ACharacter *Character) {
  if (!Character) {
    UE_LOG(LogEpochRails, Warning,
           TEXT("DisableTrainControls: null character"));
    return;
  }

  if (!bIsControllingTrain) {
    UE_LOG(LogEpochRails, Warning, TEXT("Train controls not enabled"));
    return;
  }

  UE_LOG(LogEpochRails, Log, TEXT("Disabling train controls for %s"),
         *Character->GetName());

  // Remove train control IMC
  if (TrainControlIMC && CachedController) {
    RemoveTrainControlIMC(CachedController);
  }

  // Hide HUD
  HideTrainHUD();

  // Clear controlling state
  bIsControllingTrain = false;

  // Update animation state
  UpdateCharacterAnimationState(Character, false);

  // Broadcast event
  OnStoppedControllingTrain.Broadcast(Character, ControlledTrain);

  // Clear cached controller
  CachedController = nullptr;
}

ARailsTrain *UInteractableDriverSeat::FindParentTrain() const {
  if (!GetOwner())
    return nullptr;

  // Check if owner is the train itself
  ARailsTrain *Train = Cast<ARailsTrain>(GetOwner());
  if (Train) {
    return Train;
  }

  // Search up the attachment hierarchy
  AActor *CurrentActor = GetOwner();
  while (CurrentActor) {
    Train = Cast<ARailsTrain>(CurrentActor);
    if (Train) {
      return Train;
    }

    // Move to parent actor
    CurrentActor = CurrentActor->GetAttachParentActor();
  }

  // Not found in hierarchy
  return nullptr;
}

void UInteractableDriverSeat::ShowTrainHUD() {
  if (!TrainHUDClass) {
    UE_LOG(LogEpochRails, Warning, TEXT("ShowTrainHUD: TrainHUDClass not set"));
    return;
  }

  if (!CachedController) {
    UE_LOG(LogEpochRails, Warning, TEXT("ShowTrainHUD: no cached controller"));
    return;
  }

  // Create HUD widget if not exists
  if (!TrainHUDWidget) {
    TrainHUDWidget = CreateWidget<UUserWidget>(CachedController, TrainHUDClass);
    if (!TrainHUDWidget) {
      UE_LOG(LogEpochRails, Error, TEXT("Failed to create train HUD widget"));
      return;
    }
  }

  // Add to viewport
  TrainHUDWidget->AddToViewport(10); // High Z-order
  UE_LOG(LogEpochRails, Log, TEXT("Train HUD shown"));
}

void UInteractableDriverSeat::HideTrainHUD() {
  if (TrainHUDWidget && TrainHUDWidget->IsInViewport()) {
    TrainHUDWidget->RemoveFromParent();
    UE_LOG(LogEpochRails, Log, TEXT("Train HUD hidden"));
  }
}

// ========== OVERRIDES ==========

void UInteractableDriverSeat::SitDown(ACharacter *Character) {
  if (!Character)
    return;

  // Call parent seat logic
  Super::SitDown(Character);

  // Enable train controls if auto-enabled
  if (bAutoEnableTrainControls && ControlledTrain) {
    EnableTrainControls(Character);
  }
}

void UInteractableDriverSeat::StandUp(ACharacter *Character) {
  if (!Character)
    return;

  // Disable train controls first
  if (bIsControllingTrain) {
    DisableTrainControls(Character);
  }

  // Call parent seat logic
  Super::StandUp(Character);
}

bool UInteractableDriverSeat::CanInteract(ACharacter *Character) const {
  // Check parent seat conditions
  if (!Super::CanInteract(Character)) {
    return false;
  }

  // Check if train is valid
  if (!ControlledTrain) {
    UE_LOG(LogEpochRails, Verbose,
           TEXT("Cannot interact: no controlled train"));
    return false;
  }

  // TODO: Add multiplayer check - train not controlled by another player
  // For now, single-player only

  return true;
}

// ========== QUERY API ==========

void UInteractableDriverSeat::SetControlledTrain(ARailsTrain *Train) {
  if (bIsControllingTrain) {
    UE_LOG(LogEpochRails, Warning,
           TEXT("Cannot change controlled train while actively controlling"));
    return;
  }

  ControlledTrain = Train;
  UE_LOG(LogEpochRails, Log, TEXT("Controlled train set to: %s"),
         Train ? *Train->GetName() : TEXT("nullptr"));
}

// ========== PROTECTED HELPERS ==========

void UInteractableDriverSeat::AddTrainControlIMC(
    APlayerController *Controller) {
  if (!Controller || !TrainControlIMC)
    return;

  // Get Enhanced Input Subsystem
  UEnhancedInputLocalPlayerSubsystem *Subsystem =
      ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
          Controller->GetLocalPlayer());

  if (!Subsystem) {
    UE_LOG(LogEpochRails, Error,
           TEXT("Failed to get Enhanced Input Subsystem"));
    return;
  }

  // Add train control mapping context
  Subsystem->AddMappingContext(TrainControlIMC, IMCPriority);
  UE_LOG(LogEpochRails, Log, TEXT("Train control IMC added: %s (Priority: %d)"),
         *TrainControlIMC->GetName(), IMCPriority);
}

void UInteractableDriverSeat::RemoveTrainControlIMC(
    APlayerController *Controller) {
  if (!Controller || !TrainControlIMC)
    return;

  // Get Enhanced Input Subsystem
  UEnhancedInputLocalPlayerSubsystem *Subsystem =
      ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
          Controller->GetLocalPlayer());

  if (!Subsystem) {
    UE_LOG(LogEpochRails, Warning,
           TEXT("Failed to get Enhanced Input Subsystem for removal"));
    return;
  }

  // Remove train control mapping context
  Subsystem->RemoveMappingContext(TrainControlIMC);
  UE_LOG(LogEpochRails, Log, TEXT("Train control IMC removed: %s"),
         *TrainControlIMC->GetName());
}

void UInteractableDriverSeat::UpdateCharacterAnimationState(
    ACharacter *Character, bool bSitting) {
  // Call parent to set basic sitting state
  Super::UpdateCharacterAnimationState(Character, bSitting);

  if (!Character)
    return;

  // Cast to RailsPlayerCharacter for additional driver state
  ARailsPlayerCharacter *RailsCharacter =
      Cast<ARailsPlayerCharacter>(Character);
  if (RailsCharacter) {
    // Set driver-specific state
    RailsCharacter->bIsControllingTrain = bIsControllingTrain;
    RailsCharacter->CurrentInteractionType =
        bIsControllingTrain
            ? EInteractionType::DriverSeat
            : (bSitting ? EInteractionType::Seat : EInteractionType::None);

    UE_LOG(LogEpochRails, Verbose,
           TEXT("Character %s driver state: bSitting=%s, bControlling=%s"),
           *Character->GetName(), bSitting ? TEXT("true") : TEXT("false"),
           bIsControllingTrain ? TEXT("true") : TEXT("false"));
  } else {
    UE_LOG(LogEpochRails, Warning,
           TEXT("Character is not ARailsPlayerCharacter, cannot update driver "
                "state"));
  }
}
