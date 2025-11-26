#include "Controllers/RailsPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

#include "Character/RailsPlayerCharacter.h"
#include "Train/BaseVehicle.h"

DEFINE_LOG_CATEGORY_STATIC(LogRailsPC, Log, All);

ARailsPlayerController::ARailsPlayerController() {
  MappingPriority = 0;

  static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMCObj(
      TEXT("/Game/Input/IMC_RailsDefault"));
  if (IMCObj.Succeeded()) {
    DefaultMappingContext = IMCObj.Object;
    UE_LOG(LogRailsPC, Log,
           TEXT("✅ Controller: Successfully loaded IMC_RailsDefault"));
  } else {
    UE_LOG(LogRailsPC, Error,
           TEXT("❌ Controller: Failed to load IMC_RailsDefault"));
  }
}

void ARailsPlayerController::BeginPlay() {
  Super::BeginPlay();

  AddDefaultIMC();

  bShowMouseCursor = false;
  SetInputMode(FInputModeGameOnly());
}

void ARailsPlayerController::Tick(float DeltaSeconds) {
  Super::Tick(DeltaSeconds);
}

void ARailsPlayerController::SetupInputComponent() {
  Super::SetupInputComponent();
}

void ARailsPlayerController::AddDefaultIMC() {
  if (!DefaultMappingContext) {
    UE_LOG(LogRailsPC, Warning, TEXT("❌ DefaultMappingContext is null"));
    return;
  }

  if (ULocalPlayer *LocalPlayer = GetLocalPlayer()) {
    if (UEnhancedInputLocalPlayerSubsystem *InputSubsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
                LocalPlayer)) {
      InputSubsystem->AddMappingContext(DefaultMappingContext, MappingPriority);
      UE_LOG(LogRailsPC, Log,
             TEXT("✅ Added Input Mapping Context to subsystem"));
    } else {
      UE_LOG(LogRailsPC, Error,
             TEXT("❌ Failed to get EnhancedInputLocalPlayerSubsystem"));
    }
  } else {
    UE_LOG(LogRailsPC, Error, TEXT("❌ Failed to get LocalPlayer"));
  }
}

void ARailsPlayerController::ShowMainHUD() {
  if (MainHUD && !MainHUD->IsInViewport()) {
    MainHUD->AddToViewport(0);
  }
}

void ARailsPlayerController::HideMainHUD() {
  if (MainHUD && MainHUD->IsInViewport()) {
    MainHUD->RemoveFromParent();
  }
}

void ARailsPlayerController::ShowTrainControlHUD() {
  if (!TrainControlHUD && TrainControlHUDClass) {
    TrainControlHUD = CreateWidget<UUserWidget>(this, TrainControlHUDClass);
  }

  if (TrainControlHUD && !TrainControlHUD->IsInViewport()) {
    TrainControlHUD->AddToViewport(1);
  }
}

void ARailsPlayerController::HideTrainControlHUD() {
  if (TrainControlHUD && TrainControlHUD->IsInViewport()) {
    TrainControlHUD->RemoveFromParent();
  }
}

void ARailsPlayerController::EnterTrainControlMode(ABaseVehicle *Train) {
  if (!Train) {
    UE_LOG(LogRailsPC, Warning,
           TEXT("Cannot enter train control: Train is null"));
    return;
  }

  bInTrainControlMode = true;

  ShowTrainControlHUD();

  bShowMouseCursor = true;
  SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));

  UE_LOG(LogRailsPC, Log, TEXT("Entered train control mode for: %s"),
         *Train->GetName());
}

void ARailsPlayerController::ExitTrainControlMode() {
  bInTrainControlMode = false;

  HideTrainControlHUD();

  bShowMouseCursor = false;
  SetInputMode(FInputModeGameOnly());

  UE_LOG(LogRailsPC, Log, TEXT("Exited train control mode"));
}

void ARailsPlayerController::EnterBuildMode() {
  bInBuildMode = true;
  UE_LOG(LogRailsPC, Log, TEXT("Entered build mode"));
}

void ARailsPlayerController::ExitBuildMode() {
  bInBuildMode = false;
  UE_LOG(LogRailsPC, Log, TEXT("Exited build mode"));
}

ARailsPlayerCharacter *ARailsPlayerController::GetRailsCharacter() const {
  return Cast<ARailsPlayerCharacter>(GetPawn());
}
