// InteractionManagerComponent.cpp
// Implementation of interaction manager component

#include "Interaction/InteractionManagerComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "EpochRails.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/InteractableComponent.h"
#include "Kismet/GameplayStatics.h"

UInteractionManagerComponent::UInteractionManagerComponent() {
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.TickInterval = 0.0f; // Every frame
}

void UInteractionManagerComponent::BeginPlay() {
  Super::BeginPlay();

  // Cache owner character
  OwnerCharacter = Cast<ACharacter>(GetOwner());
  if (!OwnerCharacter) {
    UE_LOG(LogEpochRails, Error,
           TEXT("InteractionManagerComponent must be attached to ACharacter!"));
    return;
  }

  // Cache player controller
  PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
  if (!PlayerController) {
    UE_LOG(LogEpochRails, Warning,
           TEXT("InteractionManager: No PlayerController found. Will retry "
                "later."));
  }

  // Find camera component
  CameraComponent = OwnerCharacter->FindComponentByClass<UCameraComponent>();
  if (!CameraComponent) {
    UE_LOG(LogEpochRails, Warning,
           TEXT("InteractionManager: No CameraComponent found. Raycast will "
                "use actor location."));
  }

  UE_LOG(LogEpochRails, Log,
         TEXT("InteractionManagerComponent initialized on %s"),
         *OwnerCharacter->GetName());
}

void UInteractionManagerComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  if (!OwnerCharacter)
    return;

  // Cache controller if not yet cached
  if (!PlayerController) {
    PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
  }

  // Update list of nearby interactables every frame
  UpdateNearbyInteractables();

  // Update focused interactable (with raycast throttling)
  if (bUseRaycast && RaycastUpdateInterval > 0.0f) {
    RaycastTimer += DeltaTime;
    if (RaycastTimer >= RaycastUpdateInterval) {
      RaycastTimer = 0.0f;
      UpdateFocus();
    }
  } else {
    // No throttling or raycast disabled
    UpdateFocus();
  }
}

// ========== DETECTION LOGIC ==========

void UInteractionManagerComponent::UpdateNearbyInteractables() {
  if (!OwnerCharacter)
    return;

  // Clear previous list
  TArray<UInteractableComponent *> PreviousInteractables = NearbyInteractables;
  NearbyInteractables.Empty();

  // Get all overlapping components
  TSet<UPrimitiveComponent *> OverlappingComponents;
  OwnerCharacter->GetOverlappingComponents(OverlappingComponents);

  // Filter for InteractionTriggers (SphereComponents from
  // InteractableComponent)
  for (UPrimitiveComponent *Component : OverlappingComponents) {
    USphereComponent *Sphere = Cast<USphereComponent>(Component);
    if (!Sphere)
      continue;

    // Check if this sphere belongs to an InteractableComponent
    AActor *ComponentOwner = Sphere->GetOwner();
    if (!ComponentOwner)
      continue;

    UInteractableComponent *Interactable =
        ComponentOwner->FindComponentByClass<UInteractableComponent>();

    if (Interactable && Interactable->GetTrigger() == Sphere) {
      // Valid interactable found
      if (Interactable->IsEnabled() &&
          Interactable->CanInteract(OwnerCharacter)) {
        NearbyInteractables.Add(Interactable);
      }
    }
  }

  // Debug log if list changed
  if (NearbyInteractables.Num() != PreviousInteractables.Num()) {
    UE_LOG(LogEpochRails, Verbose,
           TEXT("Nearby interactables updated: %d objects"),
           NearbyInteractables.Num());
  }
}

void UInteractionManagerComponent::UpdateFocus() {
  if (NearbyInteractables.Num() == 0) {
    // No interactables nearby, clear focus
    ClearFocus();
    return;
  }

  UInteractableComponent *NewFocus = nullptr;

  if (bUseRaycast) {
    // Use raycast for precise selection
    NewFocus = SelectBestInteractableWithRaycast();
  } else {
    // Fallback to distance-based selection
    NewFocus = SelectBestInteractableByDistance();
  }

  // Update focus if changed
  if (NewFocus != FocusedInteractable) {
    SetFocus(NewFocus);
  }
}

UInteractableComponent *
UInteractionManagerComponent::SelectBestInteractableWithRaycast() {
  FHitResult HitResult;

  // Perform raycast
  if (!PerformRaycast(HitResult)) {
    // Raycast failed, fallback to distance
    return SelectBestInteractableByDistance();
  }

  // Check if we hit an actor with InteractableComponent
  AActor *HitActor = HitResult.GetActor();
  if (!HitActor) {
    return SelectBestInteractableByDistance();
  }

  UInteractableComponent *HitInteractable =
      HitActor->FindComponentByClass<UInteractableComponent>();

  // Check if hit interactable is in our nearby list
  if (HitInteractable && NearbyInteractables.Contains(HitInteractable)) {
    return HitInteractable;
  }

  // Hit something else, use distance fallback
  return SelectBestInteractableByDistance();
}

UInteractableComponent *
UInteractionManagerComponent::SelectBestInteractableByDistance() {
  if (NearbyInteractables.Num() == 0)
    return nullptr;
  if (!OwnerCharacter)
    return nullptr;

  UInteractableComponent *Closest = nullptr;
  float ClosestDistance = FLT_MAX;

  FVector CharacterLocation = OwnerCharacter->GetActorLocation();

  for (UInteractableComponent *Interactable : NearbyInteractables) {
    if (!Interactable)
      continue;

    AActor *InteractableOwner = Interactable->GetOwner();
    if (!InteractableOwner)
      continue;

    float Distance =
        FVector::Dist(CharacterLocation, InteractableOwner->GetActorLocation());

    if (Distance < ClosestDistance) {
      ClosestDistance = Distance;
      Closest = Interactable;
    }
  }

  return Closest;
}

bool UInteractionManagerComponent::PerformRaycast(FHitResult &OutHit) {
  if (!PlayerController)
    return false;

  FVector RayStart, RayDirection;
  GetCameraRaycastData(RayStart, RayDirection);

  FVector RayEnd = RayStart + (RayDirection * RaycastDistance);

  // Setup trace parameters
  FCollisionQueryParams QueryParams;
  QueryParams.AddIgnoredActor(OwnerCharacter);

  // Perform line trace
  bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, RayStart, RayEnd,
                                                   RaycastChannel, QueryParams);

  // Draw debug line if enabled
  if (bDrawDebugRaycast) {
    DrawDebugLine(GetWorld(), RayStart, RayEnd,
                  bHit ? FColor::Green : FColor::Red, false, -1.0f, 0, 2.0f);

    if (bHit) {
      DrawDebugSphere(GetWorld(), OutHit.ImpactPoint, 10.0f, 12, FColor::Yellow,
                      false, -1.0f);
    }
  }

  return bHit;
}

void UInteractionManagerComponent::GetCameraRaycastData(FVector &OutLocation,
                                                        FVector &OutDirection) {
  if (PlayerController) {
    // Get screen center with offset
    int32 ViewportSizeX, ViewportSizeY;
    PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);

    FVector2D ScreenCenter(ViewportSizeX * 0.5f + ScreenCenterOffset.X,
                           ViewportSizeY * 0.5f + ScreenCenterOffset.Y);

    // Deproject screen position to world
    FVector WorldLocation, WorldDirection;
    if (PlayerController->DeprojectScreenPositionToWorld(
            ScreenCenter.X, ScreenCenter.Y, WorldLocation, WorldDirection)) {
      OutLocation = WorldLocation;
      OutDirection = WorldDirection;
      return;
    }
  }

  // Fallback to camera or actor location
  if (CameraComponent) {
    OutLocation = CameraComponent->GetComponentLocation();
    OutDirection = CameraComponent->GetForwardVector();
  } else if (OwnerCharacter) {
    OutLocation = OwnerCharacter->GetActorLocation();
    OutDirection = OwnerCharacter->GetActorForwardVector();
  } else {
    OutLocation = FVector::ZeroVector;
    OutDirection = FVector::ForwardVector;
  }
}

// ========== FOCUS MANAGEMENT ==========

void UInteractionManagerComponent::SetFocus(UInteractableComponent *NewFocus) {
  if (FocusedInteractable == NewFocus)
    return;

  // Clear old focus
  if (FocusedInteractable) {
    FocusedInteractable->SetFocused(false, OwnerCharacter);
    UE_LOG(LogEpochRails, Verbose, TEXT("Lost focus: %s"),
           *FocusedInteractable->GetOwner()->GetName());
  }

  // Set new focus
  FocusedInteractable = NewFocus;

  if (FocusedInteractable) {
    FocusedInteractable->SetFocused(true, OwnerCharacter);
    UE_LOG(LogEpochRails, Verbose, TEXT("Gained focus: %s (Type: %d)"),
           *FocusedInteractable->GetOwner()->GetName(),
           (int32)FocusedInteractable->GetInteractionType());
  }
}

void UInteractionManagerComponent::ClearFocus() { SetFocus(nullptr); }

// ========== PUBLIC API ==========

void UInteractionManagerComponent::HandleInteractInput() {
  if (!FocusedInteractable) {
    UE_LOG(LogEpochRails, Verbose,
           TEXT("No focused interactable to interact with"));
    return;
  }

  if (!OwnerCharacter) {
    UE_LOG(LogEpochRails, Warning, TEXT("No owner character for interaction"));
    return;
  }

  UE_LOG(LogEpochRails, Log, TEXT("Interaction input handled: %s"),
         *FocusedInteractable->GetOwner()->GetName());

  // Trigger interaction
  FocusedInteractable->Interact(OwnerCharacter);
}

void UInteractionManagerComponent::SetFocusedInteractable(
    UInteractableComponent *Interactable) {
  SetFocus(Interactable);
}

void UInteractionManagerComponent::UpdateFocusedInteractable() {
  UpdateNearbyInteractables();
  UpdateFocus();
}

FText UInteractionManagerComponent::GetInteractionPrompt() const {
  if (!FocusedInteractable) {
    return FText::GetEmpty();
  }

  return FocusedInteractable->GetInteractionPrompt();
}
