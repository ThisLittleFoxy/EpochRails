// InteractableComponent.cpp
// Implementation of base interactable component

#include "Interaction/InteractableComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EpochRails.h"
#include "GameFramework/Character.h"
#include "Train/RailsTrain.h"

UInteractableComponent::UInteractableComponent() {
  PrimaryComponentTick.bCanEverTick = false;

  // Create interaction trigger sphere
  InteractionTrigger =
      CreateDefaultSubobject<USphereComponent>(TEXT("InteractionTrigger"));
  InteractionTrigger->SetSphereRadius(Settings.InteractionRadius);
  InteractionTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  InteractionTrigger->SetCollisionObjectType(ECC_WorldDynamic);
  InteractionTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
  InteractionTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
  InteractionTrigger->SetGenerateOverlapEvents(true);

  // Set default interaction prompt based on type
  Settings.InteractionPrompt = FText::FromString("Press E to interact");
}

void UInteractableComponent::BeginPlay() {
  Super::BeginPlay();

  // Attach trigger to owner's root
  if (AActor *Owner = GetOwner()) {
    InteractionTrigger->AttachToComponent(
        Owner->GetRootComponent(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    InteractionTrigger->SetSphereRadius(Settings.InteractionRadius);

    // Bind overlap events
    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(
        this, &UInteractableComponent::OnTriggerBeginOverlap);
    InteractionTrigger->OnComponentEndOverlap.AddDynamic(
        this, &UInteractableComponent::OnTriggerEndOverlap);

    // Auto-detect highlight component if not set
    if (!HighlightComponent) {
      AutoDetectHighlightComponent();
    }

    UE_LOG(LogEpochRails, Log,
           TEXT("InteractableComponent initialized on %s (Type: %d)"),
           *Owner->GetName(), (int32)InteractionType);
  }
}

void UInteractableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  // Clean up highlight
  SetHighlighted(false);

  // End any active interaction
  if (bIsInteracting && InteractingCharacter) {
    EndInteraction(InteractingCharacter);
  }

  Super::EndPlay(EndPlayReason);
}

// ========== TRIGGER EVENTS ==========

void UInteractableComponent::OnTriggerBeginOverlap(
    UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult &SweepResult) {

  ACharacter *Character = Cast<ACharacter>(OtherActor);
  if (!Character)
    return;

  UE_LOG(LogEpochRails, Verbose,
         TEXT("Character %s entered interaction trigger of %s"),
         *Character->GetName(), *GetOwner()->GetName());

  // InteractionManager on character will handle this via overlap queries
}

void UInteractableComponent::OnTriggerEndOverlap(
    UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex) {

  ACharacter *Character = Cast<ACharacter>(OtherActor);
  if (!Character)
    return;

  UE_LOG(LogEpochRails, Verbose,
         TEXT("Character %s left interaction trigger of %s"),
         *Character->GetName(), *GetOwner()->GetName());

  // If this was the focused interactable, lose focus
  if (bIsFocused) {
    SetFocused(false, Character);
  }
}

// ========== INTERACTION API ==========

void UInteractableComponent::Interact(ACharacter *Character) {
  if (!Character) {
    UE_LOG(LogEpochRails, Warning, TEXT("Interact called with null character"));
    return;
  }

  // Check if can interact
  if (!CanInteract(Character)) {
    UE_LOG(LogEpochRails, Warning, TEXT("Cannot interact with %s"),
           *GetOwner()->GetName());
    return;
  }

  UE_LOG(LogEpochRails, Log,
         TEXT("Character %s interacting with %s (Type: %d)"),
         *Character->GetName(), *GetOwner()->GetName(), (int32)InteractionType);

  // Start or end interaction (toggle)
  if (bIsInteracting && InteractingCharacter == Character) {
    EndInteraction(Character);
  } else {
    StartInteraction(Character);
  }

  // Call Blueprint event
  OnInteractBP(Character);
}

bool UInteractableComponent::CanInteract(ACharacter *Character) const {
  if (!bIsEnabled)
    return false;
  if (!Character)
    return false;

  // Check if train is moving (if restriction enabled)
  if (!Settings.bCanInteractDuringTrainMovement) {
    if (IsCharacterOnMovingTrain(Character)) {
      return false;
    }
  }

  // Check Blueprint condition
  if (!CanInteractBP(Character)) {
    return false;
  }

  return true;
}

// ========== FOCUS MANAGEMENT ==========

void UInteractableComponent::SetFocused(bool bFocused, ACharacter *Character) {
  if (bIsFocused == bFocused)
    return;

  bIsFocused = bFocused;

  if (bFocused) {
    UE_LOG(LogEpochRails, Verbose, TEXT("Interactable %s gained focus"),
           *GetOwner()->GetName());
    SetHighlighted(true);
    OnFocusGained.Broadcast(Character);
  } else {
    UE_LOG(LogEpochRails, Verbose, TEXT("Interactable %s lost focus"),
           *GetOwner()->GetName());
    SetHighlighted(false);
    OnFocusLost.Broadcast(Character);
  }
}

void UInteractableComponent::SetHighlighted(bool bHighlight) {
  if (!HighlightComponent)
    return;

  if (bHighlight) {
    // Enable custom depth for outline effect
    HighlightComponent->SetRenderCustomDepth(true);
    HighlightComponent->SetCustomDepthStencilValue(
        Settings.CustomDepthStencilValue);

    UE_LOG(LogEpochRails, Verbose, TEXT("Highlight enabled on %s"),
           *HighlightComponent->GetName());
  } else {
    // Disable custom depth
    HighlightComponent->SetRenderCustomDepth(false);

    UE_LOG(LogEpochRails, Verbose, TEXT("Highlight disabled on %s"),
           *HighlightComponent->GetName());
  }
}

// ========== UTILITY ==========

bool UInteractableComponent::IsCharacterOnMovingTrain(
    ACharacter *Character) const {
  if (!Character)
    return false;

  // Trace downward to check if standing on train
  FHitResult HitResult;
  FVector StartLocation = Character->GetActorLocation();
  FVector EndLocation = StartLocation - FVector(0, 0, 200.0f);

  FCollisionQueryParams QueryParams;
  QueryParams.AddIgnoredActor(Character);

  if (GetWorld()->LineTraceSingleByChannel(
          HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams)) {
    ARailsTrain *Train = Cast<ARailsTrain>(HitResult.GetActor());
    if (Train) {
      // Check if train is moving
      float CurrentSpeed = Train->GetCurrentSpeed();
      return FMath::Abs(CurrentSpeed) > 1.0f; // Moving if speed > 1 cm/s
    }
  }

  return false;
}

void UInteractableComponent::AutoDetectHighlightComponent() {
  AActor *Owner = GetOwner();
  if (!Owner)
    return;

  // Try to find StaticMeshComponent first
  UStaticMeshComponent *StaticMesh =
      Owner->FindComponentByClass<UStaticMeshComponent>();
  if (StaticMesh) {
    HighlightComponent = StaticMesh;
    UE_LOG(LogEpochRails, Log,
           TEXT("Auto-detected StaticMeshComponent for highlight: %s"),
           *StaticMesh->GetName());
    return;
  }

  // Try SkeletalMeshComponent
  USkeletalMeshComponent *SkeletalMesh =
      Owner->FindComponentByClass<USkeletalMeshComponent>();
  if (SkeletalMesh) {
    HighlightComponent = SkeletalMesh;
    UE_LOG(LogEpochRails, Log,
           TEXT("Auto-detected SkeletalMeshComponent for highlight: %s"),
           *SkeletalMesh->GetName());
    return;
  }

  // Fallback to any PrimitiveComponent
  UPrimitiveComponent *PrimitiveComp =
      Owner->FindComponentByClass<UPrimitiveComponent>();
  if (PrimitiveComp) {
    HighlightComponent = PrimitiveComp;
    UE_LOG(LogEpochRails, Log,
           TEXT("Auto-detected PrimitiveComponent for highlight: %s"),
           *PrimitiveComp->GetName());
    return;
  }

  UE_LOG(LogEpochRails, Warning,
         TEXT("Could not auto-detect highlight component for %s. Please set "
              "manually."),
         *Owner->GetName());
}

// ========== PROTECTED FUNCTIONS ==========

void UInteractableComponent::StartInteraction(ACharacter *Character) {
  if (bIsInteracting) {
    UE_LOG(LogEpochRails, Warning,
           TEXT("Interactable %s is already being interacted with"),
           *GetOwner()->GetName());
    return;
  }

  bIsInteracting = true;
  InteractingCharacter = Character;

  UE_LOG(LogEpochRails, Log, TEXT("Interaction started: %s with %s"),
         *Character->GetName(), *GetOwner()->GetName());

  OnInteractionStarted.Broadcast(Character);
}

void UInteractableComponent::EndInteraction(ACharacter *Character) {
  if (!bIsInteracting) {
    UE_LOG(LogEpochRails, Warning,
           TEXT("Interactable %s is not being interacted with"),
           *GetOwner()->GetName());
    return;
  }

  bIsInteracting = false;
  ACharacter *PreviousCharacter = InteractingCharacter;
  InteractingCharacter = nullptr;

  UE_LOG(LogEpochRails, Log, TEXT("Interaction ended: %s with %s"),
         *Character->GetName(), *GetOwner()->GetName());

  OnInteractionEnded.Broadcast(PreviousCharacter);
}
