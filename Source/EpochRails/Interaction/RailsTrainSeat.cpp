// Fill out your copyright notice in the Description page of Project Settings.


// Copyright Epic Games, Inc. All Rights Reserved.

#include "RailsTrainSeat.h"

#include "Character/RailsPlayerCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputMappingContext.h"
#include "Train/RailsTrain.h" // путь скорректируйте под ваш проект, если отличается

ARailsTrainSeat::ARailsTrainSeat() {
  PrimaryActorTick.bCanEverTick = false;
  // Create seat point component
  SeatPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SeatPoint"));
  SeatPoint->SetupAttachment(RootComponent);

  InteractionName = FText::FromString(TEXT("Train Driver Seat"));
  InteractionAction = FText::FromString(TEXT("Sit"));
  bCanInteract = true;
  bIsPlayerSeated = false;
  SeatedPlayer = nullptr;
}

void ARailsTrainSeat::BeginPlay() { Super::BeginPlay(); }

bool ARailsTrainSeat::OnInteract_Implementation(
    ARailsPlayerCharacter *PlayerCharacter) {
  if (!PlayerCharacter) {
    return false;
  }

  // Toggle seat state on interaction
  if (!bIsPlayerSeated) {
    SeatPlayer(PlayerCharacter);
  } else {
    // If another player is already seated, only allow the same player to get up
    if (SeatedPlayer == PlayerCharacter) {
      UnseatPlayer();
    } else {
      return false;
    }
  }

  return true;
}

FText ARailsTrainSeat::GetInteractionName_Implementation() const {
  return InteractionName;
}

FText ARailsTrainSeat::GetInteractionAction_Implementation() const {
  // Change prompt depending on state
  if (bIsPlayerSeated) {
    return FText::FromString(TEXT("Stand up"));
  }

  return InteractionAction;
}

bool ARailsTrainSeat::CanInteract_Implementation(
    ARailsPlayerCharacter *PlayerCharacter) const {
  if (!Super::CanInteract_Implementation(PlayerCharacter)) {
    return false;
  }

  // Require a valid train and train control IMC to be assigned
  if (!ControlledTrain || !TrainControlInputMappingContext) {
    return false;
  }

  // If someone else is already seated, block interaction
  if (bIsPlayerSeated && SeatedPlayer && SeatedPlayer != PlayerCharacter) {
    return false;
  }

  return true;
}

void ARailsTrainSeat::SeatPlayer(ARailsPlayerCharacter *PlayerCharacter) {
  if (!PlayerCharacter || !ControlledTrain) {
    return;
  }

  SeatedPlayer = PlayerCharacter;
  bIsPlayerSeated = true;

  // Simply teleport player to seat point
  if (SeatPoint) {
    FVector SeatLocation = SeatPoint->GetComponentLocation();
    FRotator SeatRotation = SeatPoint->GetComponentRotation();

    PlayerCharacter->SetActorLocation(SeatLocation);
    PlayerCharacter->SetActorRotation(SeatRotation);
  }

  // Switch input mapping to train control
  ApplyInputMappingContexts(PlayerCharacter, true);

  // Set controlled train reference
  if (PlayerCharacter) {
    PlayerCharacter->SetControlledTrain(ControlledTrain);
    PlayerCharacter->SetCurrentSeat(this);
  }
}


void ARailsTrainSeat::UnseatPlayer() {
  if (!SeatedPlayer) {
    bIsPlayerSeated = false;
    return;
  }

  ARailsPlayerCharacter *PlayerCharacter = SeatedPlayer;
  SeatedPlayer = nullptr;
  bIsPlayerSeated = false;

  // Restore default input mapping
  ApplyInputMappingContexts(PlayerCharacter, false);

  // Clear controlled train and seat
  if (PlayerCharacter) {
    PlayerCharacter->SetControlledTrain(nullptr);
    PlayerCharacter->SetCurrentSeat(nullptr); // Добавить эту строку
  }
}


void ARailsTrainSeat::ApplyInputMappingContexts(
    ARailsPlayerCharacter *PlayerCharacter, bool bUseTrainControl) {
  if (!PlayerCharacter) {
    return;
  }

  UEnhancedInputLocalPlayerSubsystem *InputSubsystem =
      GetInputSubsystem(PlayerCharacter);
  if (!InputSubsystem) {
    return;
  }

  if (bUseTrainControl) {
    // Remove default IMC if assigned
    if (DefaultInputMappingContext) {
      InputSubsystem->RemoveMappingContext(DefaultInputMappingContext);
    }

    // Add train control IMC
    if (TrainControlInputMappingContext) {
      InputSubsystem->AddMappingContext(TrainControlInputMappingContext,
                                        TrainControlIMCPriority);
    }
  } else {
    // Remove train control IMC
    if (TrainControlInputMappingContext) {
      InputSubsystem->RemoveMappingContext(TrainControlInputMappingContext);
    }

    // Restore default IMC
    if (DefaultInputMappingContext) {
      InputSubsystem->AddMappingContext(DefaultInputMappingContext,
                                        DefaultIMCPriority);
    }
  }
}

UEnhancedInputLocalPlayerSubsystem *ARailsTrainSeat::GetInputSubsystem(
    ARailsPlayerCharacter *PlayerCharacter) const {
  if (!PlayerCharacter) {
    return nullptr;
  }

  APlayerController *PC =
      Cast<APlayerController>(PlayerCharacter->GetController());
  if (!PC) {
    return nullptr;
  }

  ULocalPlayer *LocalPlayer = PC->GetLocalPlayer();
  if (!LocalPlayer) {
    return nullptr;
  }

  return LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
}
