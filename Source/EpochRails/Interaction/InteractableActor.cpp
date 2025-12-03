// Copyright Epic Games, Inc. All Rights Reserved.

#include "InteractableActor.h"
#include "Character/RailsPlayerCharacter.h"
#include "Components/StaticMeshComponent.h"

AInteractableActor::AInteractableActor() {
  // Enable ticking if needed
  PrimaryActorTick.bCanEverTick = false;

  // Create root component
  SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
  RootComponent = SceneRoot;

  // Create mesh component (optional)
  MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
  MeshComponent->SetupAttachment(RootComponent);
}

void AInteractableActor::BeginPlay() { Super::BeginPlay(); }

void AInteractableActor::OnInteractionFocusBegin_Implementation(
    ARailsPlayerCharacter *PlayerCharacter) {
  if (bEnableDebugLog) {
    UE_LOG(LogTemp, Log, TEXT("%s: Player started looking at this object"),
           *GetName());
  }

  // Call Blueprint event
  BP_OnInteractionFocusBegin(PlayerCharacter);
}

void AInteractableActor::OnInteractionFocusEnd_Implementation(
    ARailsPlayerCharacter *PlayerCharacter) {
  if (bEnableDebugLog) {
    UE_LOG(LogTemp, Log, TEXT("%s: Player stopped looking at this object"),
           *GetName());
  }

  // Call Blueprint event
  BP_OnInteractionFocusEnd(PlayerCharacter);
}

bool AInteractableActor::OnInteract_Implementation(
    ARailsPlayerCharacter *PlayerCharacter) {
  if (bEnableDebugLog) {
    UE_LOG(LogTemp, Log, TEXT("%s: Player interacted with this object"),
           *GetName());
  }

  // Call Blueprint event
  bool bSuccess = BP_OnInteract(PlayerCharacter);

  // Default behavior: print interaction message
  if (GEngine && bSuccess) {
    FString Message = FString::Printf(
        TEXT("Interacted with: %s"), *InteractionName.ToString());
    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, Message);
  }

  return bSuccess;
}

FText AInteractableActor::GetInteractionName_Implementation() const {
  return InteractionName;
}

FText AInteractableActor::GetInteractionAction_Implementation() const {
  return InteractionAction;
}

bool AInteractableActor::CanInteract_Implementation(
    ARailsPlayerCharacter *PlayerCharacter) const {
  return bCanInteract;
}

float AInteractableActor::GetInteractionDistance_Implementation() const {
  return MaxInteractionDistance;
}
