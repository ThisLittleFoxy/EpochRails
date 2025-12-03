// Copyright Epic Games, Inc. All Rights Reserved.

#include "InteractionComponent.h"
#include "InteractableInterface.h"
#include "Character/RailsPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UInteractionComponent::UInteractionComponent() {
  // Enable ticking for this component
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled = true;

  InteractionCheckTimer = 0.0f;
}

void UInteractionComponent::BeginPlay() {
  Super::BeginPlay();

  // Cache the owning character
  OwningCharacter = Cast<ARailsPlayerCharacter>(GetOwner());
  if (!OwningCharacter) {
    UE_LOG(LogTemp, Warning,
           TEXT("UInteractionComponent: Owner is not ARailsPlayerCharacter!"));
  }
}

void UInteractionComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  // Don't check every frame for performance
  InteractionCheckTimer -= DeltaTime;
  if (InteractionCheckTimer > 0.0f) {
    return;
  }
  InteractionCheckTimer = InteractionCheckFrequency;

  // Perform interaction trace
  FHitResult HitResult;
  if (PerformInteractionTrace(HitResult)) {
    AActor *HitActor = HitResult.GetActor();
    if (HitActor && HitActor->Implements<UInteractableInterface>()) {
      // Check if we can interact with this object
      IInteractableInterface *Interactable =
          Cast<IInteractableInterface>(HitActor);
      if (Interactable &&
          Interactable->Execute_CanInteract(HitActor, OwningCharacter)) {
        // New valid interactable found
        if (HitActor != FocusedActor.Get()) {
          UpdateFocusedActor(HitActor);
        }
        return;
      }
    }
  }

  // No valid interactable found, clear focus
  if (FocusedActor.IsValid()) {
    UpdateFocusedActor(nullptr);
  }
}

bool UInteractionComponent::PerformInteractionTrace(FHitResult &OutHitResult) {
  if (!OwningCharacter) {
    return false;
  }

  // Get camera for trace start/direction
  UCameraComponent *Camera = OwningCharacter->GetFollowCamera();
  if (!Camera) {
    return false;
  }

  // Setup trace parameters
  FVector TraceStart = Camera->GetComponentLocation();
  FVector TraceDirection = Camera->GetForwardVector();
  FVector TraceEnd = TraceStart + (TraceDirection * DefaultInteractionDistance);

  // Setup collision query parameters
  FCollisionQueryParams QueryParams;
  QueryParams.AddIgnoredActor(OwningCharacter);
  QueryParams.bTraceComplex = false;

  // Perform line trace
  bool bHit = GetWorld()->LineTraceSingleByChannel(
      OutHitResult, TraceStart, TraceEnd, InteractionTraceChannel, QueryParams);

  // Debug visualization
  if (bShowDebugTrace) {
    FColor DebugColor = bHit ? FColor::Green : FColor::Red;
    DrawDebugLine(GetWorld(), TraceStart, TraceEnd, DebugColor, false,
                  DebugTraceDuration, 0, 2.0f);

    if (bHit) {
      DrawDebugPoint(GetWorld(), OutHitResult.ImpactPoint, 10.0f, DebugColor,
                     false, DebugTraceDuration);
    }
  }

  return bHit;
}

void UInteractionComponent::UpdateFocusedActor(AActor *NewFocusActor) {
  // Handle focus end on previous actor
  if (FocusedActor.IsValid() && FocusedActor.Get() != NewFocusActor) {
    IInteractableInterface *OldInteractable =
        Cast<IInteractableInterface>(FocusedActor.Get());
    if (OldInteractable) {
      OldInteractable->Execute_OnInteractionFocusEnd(FocusedActor.Get(),
                                                      OwningCharacter);
    }
  }

  // Update focused actor
  FocusedActor = NewFocusActor;

  // Handle focus begin on new actor
  if (FocusedActor.IsValid()) {
    IInteractableInterface *NewInteractable =
        Cast<IInteractableInterface>(FocusedActor.Get());
    if (NewInteractable) {
      NewInteractable->Execute_OnInteractionFocusBegin(FocusedActor.Get(),
                                                        OwningCharacter);
    }
  }
}

bool UInteractionComponent::TryInteract() {
  if (!FocusedActor.IsValid() || !OwningCharacter) {
    return false;
  }

  IInteractableInterface *Interactable =
      Cast<IInteractableInterface>(FocusedActor.Get());
  if (!Interactable) {
    return false;
  }

  // Check if we can interact
  if (!Interactable->Execute_CanInteract(FocusedActor.Get(), OwningCharacter)) {
    return false;
  }

  // Execute interaction
  return Interactable->Execute_OnInteract(FocusedActor.Get(), OwningCharacter);
}

FText UInteractionComponent::GetFocusedActorName() const {
  if (!FocusedActor.IsValid()) {
    return FText::GetEmpty();
  }

  IInteractableInterface *Interactable =
      Cast<IInteractableInterface>(FocusedActor.Get());
  if (!Interactable) {
    return FText::GetEmpty();
  }

  return Interactable->Execute_GetInteractionName(FocusedActor.Get());
}

FText UInteractionComponent::GetFocusedActorAction() const {
  if (!FocusedActor.IsValid()) {
    return FText::GetEmpty();
  }

  IInteractableInterface *Interactable =
      Cast<IInteractableInterface>(FocusedActor.Get());
  if (!Interactable) {
    return FText::GetEmpty();
  }

  return Interactable->Execute_GetInteractionAction(FocusedActor.Get());
}

bool UInteractionComponent::CanInteractWithFocusedActor() const {
  if (!FocusedActor.IsValid() || !OwningCharacter) {
    return false;
  }

  IInteractableInterface *Interactable =
      Cast<IInteractableInterface>(FocusedActor.Get());
  if (!Interactable) {
    return false;
  }

  return Interactable->Execute_CanInteract(FocusedActor.Get(), OwningCharacter);
}
