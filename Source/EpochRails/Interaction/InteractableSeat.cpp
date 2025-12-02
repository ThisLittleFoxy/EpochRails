// InteractableSeat.cpp
// Implementation of seat interactable component

#include "Interaction/InteractableSeat.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EpochRails.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/RailsPlayerCharacter.h"  // ADD THIS LINE

UInteractableSeat::UInteractableSeat() {
  // Set seat-specific defaults
  InteractionType = EInteractionType::Seat;
  Settings.InteractionPrompt = FText::FromString("Press E to sit");
}

void UInteractableSeat::BeginPlay() {
  Super::BeginPlay();

  // Find seat component with socket
  FindSeatComponent();

  UE_LOG(LogEpochRails, Log,
         TEXT("InteractableSeat initialized: %s (Socket: %s)"),
         *GetOwner()->GetName(), *SeatSocketName.ToString());
}

// ========== SEAT LOGIC ==========

void UInteractableSeat::SitDown(ACharacter *Character) {
  if (!Character) {
    UE_LOG(LogEpochRails, Warning, TEXT("SitDown called with null character"));
    return;
  }

  if (bIsOccupied) {
    UE_LOG(LogEpochRails, Warning, TEXT("Seat %s is already occupied"),
           *GetOwner()->GetName());
    return;
  }

  UE_LOG(LogEpochRails, Log, TEXT("Character %s sitting down in %s"),
         *Character->GetName(), *GetOwner()->GetName());

  // Mark as occupied
  bIsOccupied = true;
  CurrentOccupant = Character;

  // Setup character for sitting
  SetupSittingCharacter(Character);

  // Get seat transform
  FTransform SeatTransform = GetSeatTransform();

  // Attach character to seat
  if (SeatComponent) {
    // Attach to socket if available
    Character->AttachToComponent(
        SeatComponent, FAttachmentTransformRules::SnapToTargetIncludingScale,
        SeatSocketName);
    UE_LOG(LogEpochRails, Verbose, TEXT("Character attached to socket: %s"),
           *SeatSocketName.ToString());
  } else {
    // Fallback: attach to owner root with manual offset
    Character->AttachToComponent(GetOwner()->GetRootComponent(),
                                 FAttachmentTransformRules::KeepWorldTransform);
    Character->SetActorRelativeLocation(SeatOffset);
    Character->SetActorRelativeRotation(SeatRotation);
    UE_LOG(LogEpochRails, Verbose,
           TEXT("Character attached with manual offset"));
  }

  // Update animation state
  UpdateCharacterAnimationState(Character, true);

  // Broadcast event
  OnCharacterSatDown.Broadcast(Character);

  // Update interaction state
  StartInteraction(Character);
}

void UInteractableSeat::StandUp(ACharacter *Character) {
  if (!Character) {
    UE_LOG(LogEpochRails, Warning, TEXT("StandUp called with null character"));
    return;
  }

  if (!bIsOccupied || CurrentOccupant != Character) {
    UE_LOG(LogEpochRails, Warning, TEXT("Character %s is not sitting in %s"),
           *Character->GetName(), *GetOwner()->GetName());
    return;
  }

  UE_LOG(LogEpochRails, Log, TEXT("Character %s standing up from %s"),
         *Character->GetName(), *GetOwner()->GetName());

  // Detach character
  FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
  Character->DetachFromActor(DetachRules);

  // Restore character
  RestoreStandingCharacter(Character);

  // Update animation state
  UpdateCharacterAnimationState(Character, false);

  // Broadcast event
  OnCharacterStoodUp.Broadcast(Character);

  // Clear occupancy
  bIsOccupied = false;
  CurrentOccupant = nullptr;

  // Update interaction state
  EndInteraction(Character);
}

FTransform UInteractableSeat::GetSeatTransform() const {
  // Try to get socket transform
  if (SeatComponent) {
    USkeletalMeshComponent *SkeletalMesh =
        Cast<USkeletalMeshComponent>(SeatComponent);
    if (SkeletalMesh && SkeletalMesh->DoesSocketExist(SeatSocketName)) {
      return SkeletalMesh->GetSocketTransform(SeatSocketName);
    }

    UStaticMeshComponent *StaticMesh =
        Cast<UStaticMeshComponent>(SeatComponent);
    if (StaticMesh && StaticMesh->DoesSocketExist(SeatSocketName)) {
      return StaticMesh->GetSocketTransform(SeatSocketName);
    }

    // Socket exists on generic scene component
    if (SeatComponent->DoesSocketExist(SeatSocketName)) {
      return SeatComponent->GetSocketTransform(SeatSocketName);
    }
  }

  // Fallback: use owner location + offset
  if (AActor *Owner = GetOwner()) {
    FTransform OwnerTransform = Owner->GetActorTransform();
    FTransform OffsetTransform(SeatRotation, SeatOffset);
    return OffsetTransform * OwnerTransform;
  }

  return FTransform::Identity;
}

void UInteractableSeat::FindSeatComponent() {
  if (!GetOwner())
    return;

  // Try skeletal mesh first
  USkeletalMeshComponent *SkeletalMesh =
      GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
  if (SkeletalMesh && SkeletalMesh->DoesSocketExist(SeatSocketName)) {
    SeatComponent = SkeletalMesh;
    UE_LOG(LogEpochRails, Log,
           TEXT("Found seat socket on SkeletalMeshComponent"));
    return;
  }

  // Try static mesh
  UStaticMeshComponent *StaticMesh =
      GetOwner()->FindComponentByClass<UStaticMeshComponent>();
  if (StaticMesh && StaticMesh->DoesSocketExist(SeatSocketName)) {
    SeatComponent = StaticMesh;
    UE_LOG(LogEpochRails, Log,
           TEXT("Found seat socket on StaticMeshComponent"));
    return;
  }

  // Try any scene component with socket
  TArray<USceneComponent *> SceneComponents;
  GetOwner()->GetComponents<USceneComponent>(SceneComponents);
  for (USceneComponent *Component : SceneComponents) {
    if (Component && Component->DoesSocketExist(SeatSocketName)) {
      SeatComponent = Component;
      UE_LOG(LogEpochRails, Log,
             TEXT("Found seat socket on SceneComponent: %s"),
             *Component->GetName());
      return;
    }
  }

  // No socket found, will use fallback offset
  UE_LOG(LogEpochRails, Warning,
         TEXT("No socket '%s' found on %s. Using fallback offset."),
         *SeatSocketName.ToString(), *GetOwner()->GetName());
  SeatComponent = nullptr;
}

// ========== OVERRIDES ==========

void UInteractableSeat::Interact(ACharacter *Character) {
  if (!Character)
    return;

  // Toggle sit/stand
  if (bIsOccupied && CurrentOccupant == Character) {
    // Standing up
    StandUp(Character);
  } else if (!bIsOccupied) {
    // Sitting down
    SitDown(Character);
  }

  // Call parent for Blueprint event
  Super::Interact(Character);
}

bool UInteractableSeat::CanInteract(ACharacter *Character) const {
  // Check parent conditions
  if (!Super::CanInteract(Character)) {
    return false;
  }

  // If occupied, only the occupant can interact (to stand up)
  if (bIsOccupied) {
    return CurrentOccupant == Character;
  }

  // Seat is free
  return true;
}

// ========== QUERY API ==========

bool UInteractableSeat::IsCharacterSitting(ACharacter *Character) const {
  return bIsOccupied && CurrentOccupant == Character;
}

// ========== PROTECTED HELPERS ==========

void UInteractableSeat::SetupSittingCharacter(ACharacter *Character) {
  if (!Character)
    return;

  UCharacterMovementComponent *MovementComp = Character->GetCharacterMovement();
  if (!MovementComp)
    return;

  // Save current movement mode
  PreviousMovementMode = MovementComp->MovementMode;

  if (bDisableMovementWhenSitting) {
    // Disable character movement
    MovementComp->DisableMovement();
    UE_LOG(LogEpochRails, Verbose, TEXT("Character movement disabled"));
  }

  if (bDisableCollisionWhenSitting) {
    // Disable character collision
    Character->SetActorEnableCollision(false);
    UE_LOG(LogEpochRails, Verbose, TEXT("Character collision disabled"));
  }
}

void UInteractableSeat::RestoreStandingCharacter(ACharacter *Character) {
  if (!Character)
    return;

  UCharacterMovementComponent *MovementComp = Character->GetCharacterMovement();
  if (!MovementComp)
    return;

  if (bDisableMovementWhenSitting) {
    // Restore movement
    MovementComp->SetMovementMode(
        static_cast<EMovementMode>(PreviousMovementMode));
    UE_LOG(LogEpochRails, Verbose, TEXT("Character movement restored"));
  }

  if (bDisableCollisionWhenSitting) {
    // Restore collision
    Character->SetActorEnableCollision(true);
    UE_LOG(LogEpochRails, Verbose, TEXT("Character collision restored"));
  }
}

void UInteractableSeat::UpdateCharacterAnimationState(ACharacter *Character,
                                                      bool bSitting) {
  if (!Character)
    return;

  // Cast to RailsPlayerCharacter to access animation variables
  ARailsPlayerCharacter *RailsCharacter =
      Cast<ARailsPlayerCharacter>(Character);
  if (RailsCharacter) {
    // Set sitting state
    RailsCharacter->bIsSitting = bSitting;
    RailsCharacter->bIsInteracting = bSitting;
    RailsCharacter->CurrentInteractionType =
        bSitting ? EInteractionType::Seat : EInteractionType::None;
    RailsCharacter->CurrentInteractedActor = bSitting ? GetOwner() : nullptr;

    UE_LOG(LogEpochRails, Log,
           TEXT("Character %s animation state updated: bIsSitting=%s"),
           *Character->GetName(), bSitting ? TEXT("true") : TEXT("false"));
  } else {
    UE_LOG(LogEpochRails, Warning,
           TEXT("Character is not ARailsPlayerCharacter, cannot update "
                "animation state"));
  }
}
