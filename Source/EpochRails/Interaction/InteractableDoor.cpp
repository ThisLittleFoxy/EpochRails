// InteractableDoor.cpp
// Implementation of door interactable component

#include "Interaction/InteractableDoor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "EpochRails.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UInteractableDoor::UInteractableDoor() {
  // Enable tick for animation
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled =
      false; // Only tick when animating

  // Set door-specific defaults
  InteractionType = EInteractionType::Door;
  Settings.InteractionPrompt = FText::FromString("Press E to open");
}

void UInteractableDoor::BeginPlay() {
  Super::BeginPlay();

  // Find door component
  FindDoorComponent();

  // Store initial transform
  if (DoorComponent) {
    InitialTransform = DoorComponent->GetRelativeTransform();
    UE_LOG(LogEpochRails, Log, TEXT("Door component found: %s"),
           *DoorComponent->GetName());
  } else {
    UE_LOG(LogEpochRails, Warning, TEXT("No door component found on %s"),
           *GetOwner()->GetName());
  }
}

void UInteractableDoor::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  if (!bIsAnimating || !DoorComponent)
    return;

  // Update animation time
  if (bAnimatingForward) {
    CurrentAnimationTime += DeltaTime;
  } else {
    CurrentAnimationTime -= DeltaTime;
  }

  // Clamp to duration
  CurrentAnimationTime =
      FMath::Clamp(CurrentAnimationTime, 0.0f, AnimationDuration);

  // Calculate progress
  float Progress = AnimationDuration > 0.0f
                       ? CurrentAnimationTime / AnimationDuration
                       : 1.0f;

  // Apply curve if set
  if (AnimationCurve) {
    Progress = AnimationCurve->GetFloatValue(Progress);
  }

  AnimationProgress = Progress;

  // Update door transform
  UpdateDoorTransform(Progress);

  // Check if animation finished
  if ((bAnimatingForward && CurrentAnimationTime >= AnimationDuration) ||
      (!bAnimatingForward && CurrentAnimationTime <= 0.0f)) {

    // Animation complete
    bIsAnimating = false;
    SetComponentTickEnabled(false);

    bool NewOpenState = (CurrentAnimationTime >= AnimationDuration);
    bIsOpen = NewOpenState;

    UE_LOG(LogEpochRails, Log, TEXT("Door animation complete: %s"),
           bIsOpen ? TEXT("OPEN") : TEXT("CLOSED"));

    // Update prompt
    Settings.InteractionPrompt = bIsOpen ? FText::FromString("Press E to close")
                                         : FText::FromString("Press E to open");

    // Broadcast events
    OnDoorStateChanged.Broadcast(bIsOpen);
    if (bIsOpen) {
      OnDoorOpened.Broadcast(true);
      // Start auto-close timer
      if (AutoCloseDelay > 0.0f) {
        StartAutoCloseTimer();
      }
    } else {
      OnDoorClosed.Broadcast(false);
    }
  }
}

// ========== DOOR LOGIC ==========

void UInteractableDoor::Open() {
  if (bIsOpen && !bIsAnimating) {
    UE_LOG(LogEpochRails, Verbose, TEXT("Door already open"));
    return;
  }

  if (!DoorComponent) {
    UE_LOG(LogEpochRails, Warning, TEXT("Cannot open door: no door component"));
    return;
  }

  UE_LOG(LogEpochRails, Log, TEXT("Opening door: %s"), *GetOwner()->GetName());

  // Clear auto-close timer
  ClearAutoCloseTimer();

  // Start opening animation
  StartAnimation(true);

  // Play sound
  if (OpenSound) {
    UGameplayStatics::PlaySoundAtLocation(this, OpenSound,
                                          GetOwner()->GetActorLocation());
  }
}

void UInteractableDoor::Close() {
  if (!bIsOpen && !bIsAnimating) {
    UE_LOG(LogEpochRails, Verbose, TEXT("Door already closed"));
    return;
  }

  if (!DoorComponent) {
    UE_LOG(LogEpochRails, Warning,
           TEXT("Cannot close door: no door component"));
    return;
  }

  UE_LOG(LogEpochRails, Log, TEXT("Closing door: %s"), *GetOwner()->GetName());

  // Clear auto-close timer
  ClearAutoCloseTimer();

  // Start closing animation
  StartAnimation(false);

  // Play sound
  if (CloseSound) {
    UGameplayStatics::PlaySoundAtLocation(this, CloseSound,
                                          GetOwner()->GetActorLocation());
  }
}

void UInteractableDoor::Toggle() {
  if (bIsOpen || bIsAnimating) {
    Close();
  } else {
    Open();
  }
}

void UInteractableDoor::FindDoorComponent() {
  if (!GetOwner())
    return;

  // Try to find StaticMeshComponent first
  UStaticMeshComponent *StaticMesh =
      GetOwner()->FindComponentByClass<UStaticMeshComponent>();
  if (StaticMesh) {
    DoorComponent = StaticMesh;
    UE_LOG(LogEpochRails, Log, TEXT("Found door StaticMeshComponent"));
    return;
  }

  // Try SkeletalMeshComponent
  USkeletalMeshComponent *SkeletalMesh =
      GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
  if (SkeletalMesh) {
    DoorComponent = SkeletalMesh;
    UE_LOG(LogEpochRails, Log, TEXT("Found door SkeletalMeshComponent"));
    return;
  }

  // Use root component as fallback
  DoorComponent = GetOwner()->GetRootComponent();
  UE_LOG(LogEpochRails, Warning,
         TEXT("No mesh component found, using root component"));
}

void UInteractableDoor::StartAnimation(bool bOpenDirection) {
  bIsAnimating = true;
  bAnimatingForward = bOpenDirection;
  SetComponentTickEnabled(true);

  UE_LOG(LogEpochRails, Verbose, TEXT("Door animation started: %s"),
         bOpenDirection ? TEXT("OPENING") : TEXT("CLOSING"));
}

void UInteractableDoor::UpdateDoorTransform(float Progress) {
  if (!DoorComponent)
    return;

  FTransform NewTransform;

  switch (AnimationType) {
  case EDoorAnimationType::Hinge:
    NewTransform = CalculateHingeTransform(Progress);
    break;

  case EDoorAnimationType::Slide:
    NewTransform = CalculateSlideTransform(Progress);
    break;

  default:
    NewTransform = InitialTransform;
    break;
  }

  DoorComponent->SetRelativeTransform(NewTransform);
}

FTransform UInteractableDoor::CalculateHingeTransform(float Progress) const {
  // Start from initial transform
  FTransform NewTransform = InitialTransform;

  // Calculate rotation
  float CurrentAngle = HingeRotationAngle * Progress;
  FQuat RotationQuat = FQuat(HingeRotationAxis.GetSafeNormal(),
                             FMath::DegreesToRadians(CurrentAngle));

  // Apply rotation to initial rotation
  FQuat FinalRotation = InitialTransform.GetRotation() * RotationQuat;
  NewTransform.SetRotation(FinalRotation);

  return NewTransform;
}

FTransform UInteractableDoor::CalculateSlideTransform(float Progress) const {
  // Start from initial transform
  FTransform NewTransform = InitialTransform;

  // Calculate offset
  FVector CurrentOffset = SlideOffset * Progress;

  // Apply offset to initial location
  FVector FinalLocation = InitialTransform.GetLocation() + CurrentOffset;
  NewTransform.SetLocation(FinalLocation);

  return NewTransform;
}

void UInteractableDoor::StartAutoCloseTimer() {
  if (AutoCloseDelay <= 0.0f)
    return;

  ClearAutoCloseTimer();

  if (UWorld *World = GetWorld()) {
    World->GetTimerManager().SetTimer(AutoCloseTimer, this,
                                      &UInteractableDoor::OnAutoClose,
                                      AutoCloseDelay, false);

    UE_LOG(LogEpochRails, Verbose,
           TEXT("Auto-close timer started: %.1f seconds"), AutoCloseDelay);
  }
}

void UInteractableDoor::ClearAutoCloseTimer() {
  if (UWorld *World = GetWorld()) {
    if (AutoCloseTimer.IsValid()) {
      World->GetTimerManager().ClearTimer(AutoCloseTimer);
      AutoCloseTimer.Invalidate();
      UE_LOG(LogEpochRails, Verbose, TEXT("Auto-close timer cleared"));
    }
  }
}

void UInteractableDoor::OnAutoClose() {
  UE_LOG(LogEpochRails, Log, TEXT("Auto-closing door"));
  Close();
}

// ========== OVERRIDES ==========

void UInteractableDoor::Interact(ACharacter *Character) {
  if (!Character)
    return;

  // Toggle door
  Toggle();

  // Call parent for Blueprint event
  Super::Interact(Character);
}

bool UInteractableDoor::CanInteract(ACharacter *Character) const {
  // Check parent conditions
  if (!Super::CanInteract(Character)) {
    return false;
  }

  // Check if locked during train movement
  if (bLockDuringTrainMovement) {
    if (IsCharacterOnMovingTrain(Character)) {
      UE_LOG(LogEpochRails, Verbose, TEXT("Door locked: train is moving"));
      return false;
    }
  }

  // Can't interact while animating
  if (bIsAnimating) {
    return false;
  }

  return true;
}

// ========== QUERY API ==========

void UInteractableDoor::SetDoorStateImmediate(bool bOpen) {
  if (!DoorComponent) {
    UE_LOG(LogEpochRails, Warning,
           TEXT("SetDoorStateImmediate: no door component"));
    return;
  }

  // Stop any animation
  bIsAnimating = false;
  SetComponentTickEnabled(false);
  ClearAutoCloseTimer();

  // Set state
  bIsOpen = bOpen;
  AnimationProgress = bOpen ? 1.0f : 0.0f;
  CurrentAnimationTime = bOpen ? AnimationDuration : 0.0f;

  // Update transform immediately
  UpdateDoorTransform(AnimationProgress);

  // Update prompt
  Settings.InteractionPrompt = bIsOpen ? FText::FromString("Press E to close")
                                       : FText::FromString("Press E to open");

  UE_LOG(LogEpochRails, Log, TEXT("Door state set immediately: %s"),
         bIsOpen ? TEXT("OPEN") : TEXT("CLOSED"));

  // Broadcast event
  OnDoorStateChanged.Broadcast(bIsOpen);
}
