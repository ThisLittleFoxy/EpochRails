// InteractableMasterActor.cpp

#include "Interaction/InteractableMasterActor.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EpochRails.h"
#include "Interaction/InteractableComponent.h"
#include "Interaction/InteractableDoor.h"
#include "Interaction/InteractableDriverSeat.h"
#include "Interaction/InteractableSeat.h"
#include "Train/RailsTrain.h"

AInteractableMasterActor::AInteractableMasterActor() {
  PrimaryActorTick.bCanEverTick = false;

  // Create root
  SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
  RootComponent = SceneRoot;

  // Create mesh component
  InteractableMesh =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InteractableMesh"));
  InteractableMesh->SetupAttachment(SceneRoot);
  InteractableMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  InteractableMesh->SetCollisionResponseToAllChannels(ECR_Block);

  // Create interaction trigger as SphereComponent
  InteractionTrigger =
      CreateDefaultSubobject<USphereComponent>(TEXT("InteractionTrigger"));
  InteractionTrigger->SetupAttachment(SceneRoot);
  InteractionTrigger->SetSphereRadius(200.0f);
  InteractionTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  InteractionTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
  InteractionTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
  InteractionTrigger->SetGenerateOverlapEvents(true);

  // Interactable component will be created in OnConstruction
}

void AInteractableMasterActor::OnConstruction(const FTransform &Transform) {
  Super::OnConstruction(Transform);

  // Recreate interactable component based on type
  CreateInteractableComponent();
  ConfigureInteractableComponent();
}

void AInteractableMasterActor::BeginPlay() {
  Super::BeginPlay();

  // Ensure component exists at runtime
  if (!InteractableComponent) {
    CreateInteractableComponent();
    ConfigureInteractableComponent();
  }

  UE_LOG(LogEpochRails, Log,
         TEXT("InteractableMasterActor '%s' initialized with type: %d"),
         *GetName(), (int32)InteractionType);
}

#if WITH_EDITOR
void AInteractableMasterActor::PostEditChangeProperty(
    FPropertyChangedEvent &PropertyChangedEvent) {
  Super::PostEditChangeProperty(PropertyChangedEvent);

  FName PropertyName = PropertyChangedEvent.GetPropertyName();

  // Recreate component if interaction type changed
  if (PropertyName ==
      GET_MEMBER_NAME_CHECKED(AInteractableMasterActor, InteractionType)) {
    CreateInteractableComponent();
    ConfigureInteractableComponent();
  }
  // Update configuration if other properties changed
  else if (PropertyName == GET_MEMBER_NAME_CHECKED(AInteractableMasterActor,
                                                   CustomInteractionPrompt) ||
           PropertyName == GET_MEMBER_NAME_CHECKED(AInteractableMasterActor,
                                                   bAutoFindParentTrain) ||
           PropertyName == GET_MEMBER_NAME_CHECKED(AInteractableMasterActor,
                                                   AssignedTrain) ||
           PropertyName == GET_MEMBER_NAME_CHECKED(AInteractableMasterActor,
                                                   DoorAnimationDuration) ||
           PropertyName == GET_MEMBER_NAME_CHECKED(AInteractableMasterActor,
                                                   DoorOpenOffset)) {
    ConfigureInteractableComponent();
  }
}
#endif

void AInteractableMasterActor::CleanupInteractableComponent() {
  if (InteractableComponent) {
    InteractableComponent->DestroyComponent();
    InteractableComponent = nullptr;
  }
}

void AInteractableMasterActor::CreateInteractableComponent() {
  // Clean up old component
  CleanupInteractableComponent();

  // Create new component based on type
  switch (InteractionType) {
  case EInteractionType::DriverSeat: {
    UInteractableDriverSeat *DriverSeat = NewObject<UInteractableDriverSeat>(
        this, UInteractableDriverSeat::StaticClass(),
        TEXT("InteractableDriverSeat"));
    if (DriverSeat) {
      DriverSeat->RegisterComponent();
      InteractableComponent = DriverSeat;
      UE_LOG(LogEpochRails, Log,
             TEXT("Created InteractableDriverSeat component"));
    }
    break;
  }

  case EInteractionType::Seat: {
    UInteractableSeat *Seat = NewObject<UInteractableSeat>(
        this, UInteractableSeat::StaticClass(), TEXT("InteractableSeat"));
    if (Seat) {
      Seat->RegisterComponent();
      InteractableComponent = Seat;
      UE_LOG(LogEpochRails, Log, TEXT("Created InteractableSeat component"));
    }
    break;
  }

  case EInteractionType::Door: {
    UInteractableDoor *Door = NewObject<UInteractableDoor>(
        this, UInteractableDoor::StaticClass(), TEXT("InteractableDoor"));
    if (Door) {
      Door->RegisterComponent();
      InteractableComponent = Door;
      UE_LOG(LogEpochRails, Log, TEXT("Created InteractableDoor component"));
    }
    break;
  }

  case EInteractionType::None:
  default: {
    UInteractableComponent *BaseComponent = NewObject<UInteractableComponent>(
        this, UInteractableComponent::StaticClass(),
        TEXT("InteractableComponent"));
    if (BaseComponent) {
      BaseComponent->RegisterComponent();
      InteractableComponent = BaseComponent;
      UE_LOG(LogEpochRails, Log, TEXT("Created base InteractableComponent"));
    }
    break;
  }
  }
}

void AInteractableMasterActor::ConfigureInteractableComponent() {
  if (!InteractableComponent) {
    return;
  }

  // Setup external trigger
  if (InteractionTrigger) {
    InteractableComponent->SetupExternalTrigger(InteractionTrigger);
  }

  // Configure base settings
  FInteractionSettings Settings = InteractableComponent->GetSettings();
  if (!CustomInteractionPrompt.IsEmpty()) {
    Settings.InteractionPrompt = CustomInteractionPrompt;
  }
  InteractableComponent->SetSettings(Settings);

  // Configure type-specific settings
  if (InteractionType == EInteractionType::DriverSeat) {
    UInteractableDriverSeat *DriverSeat =
        Cast<UInteractableDriverSeat>(InteractableComponent);
    if (DriverSeat) {
      // Configure driver seat settings
      DriverSeat->bAutoFindParentTrain = bAutoFindParentTrain;
      if (!bAutoFindParentTrain && AssignedTrain) {
        DriverSeat->SetControlledTrain(AssignedTrain);
      }
      UE_LOG(LogEpochRails, Log, TEXT("Configured DriverSeat settings"));
    }
  } else if (InteractionType == EInteractionType::Seat) {
    UInteractableSeat *Seat = Cast<UInteractableSeat>(InteractableComponent);
    if (Seat) {
      // Configure seat-specific settings here if needed
      UE_LOG(LogEpochRails, Log, TEXT("Configured Seat settings"));
    }
  } else if (InteractionType == EInteractionType::Door) {
    UInteractableDoor *Door = Cast<UInteractableDoor>(InteractableComponent);
    if (Door) {
      // Configure door-specific settings
      // Door->AnimationDuration = DoorAnimationDuration;
      UE_LOG(LogEpochRails, Log, TEXT("Configured Door settings"));
    }
  }
}
