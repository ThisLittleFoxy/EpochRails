// TrainConsole.cpp

#include "TrainConsole.h"
#include "TrainActor.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Controllers/RailsPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"

ATrainConsole::ATrainConsole()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Console mesh
	ConsoleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ConsoleMesh"));
	ConsoleMesh->SetupAttachment(Root);
	ConsoleMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ConsoleMesh->SetCollisionProfileName(TEXT("BlockAll"));

	// Interaction zone (for blocking other interactions when in use)
	InteractionZone = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionZone"));
	InteractionZone->SetupAttachment(Root);
	InteractionZone->SetBoxExtent(FVector(50.0f, 100.0f, 75.0f));
	InteractionZone->SetRelativeLocation(FVector(50.0f, 0.0f, 50.0f));
	InteractionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionZone->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void ATrainConsole::BeginPlay()
{
	Super::BeginPlay();

	// Try to find train if not set
	if (!ControlledTrain)
	{
		// Look for parent train actor
		AActor* Parent = GetAttachParentActor();
		if (ATrainActor* ParentTrain = Cast<ATrainActor>(Parent))
		{
			ControlledTrain = ParentTrain;
		}
	}
}

// ===== IInteractableInterface =====

void ATrainConsole::OnInteractionFocusBegin_Implementation(ACharacter* PlayerCharacter)
{
	BP_OnFocusBegin(PlayerCharacter);
}

void ATrainConsole::OnInteractionFocusEnd_Implementation(ACharacter* PlayerCharacter)
{
	BP_OnFocusEnd(PlayerCharacter);
}

bool ATrainConsole::OnInteract_Implementation(ACharacter* PlayerCharacter)
{
	if (!PlayerCharacter)
	{
		return false;
	}

	// Toggle operation
	if (CurrentOperator == PlayerCharacter)
	{
		// Already operating - stop
		StopOperation();
		return true;
	}
	else if (CurrentOperator == nullptr)
	{
		// Not in use - start operation
		StartOperation(PlayerCharacter);
		return true;
	}

	// Someone else is using it
	return false;
}

FText ATrainConsole::GetInteractionName_Implementation() const
{
	return InteractionName;
}

FText ATrainConsole::GetInteractionAction_Implementation() const
{
	if (CurrentOperator)
	{
		return StopOperatingText;
	}
	return InteractionActionText;
}

bool ATrainConsole::CanInteract_Implementation(ACharacter* PlayerCharacter) const
{
	// Can interact if no one is using it, or if the player is already the operator
	return (CurrentOperator == nullptr || CurrentOperator == PlayerCharacter);
}

float ATrainConsole::GetInteractionDistance_Implementation() const
{
	return MaxInteractionDistance;
}

// ===== Operation Control =====

void ATrainConsole::StartOperation(ACharacter* Operator)
{
	if (!Operator)
	{
		return;
	}

	CurrentOperator = Operator;

	// Get player controller
	APlayerController* PC = Cast<APlayerController>(Operator->GetController());
	if (PC)
	{
		CachedController = PC;
		AddInputContext(PC);

		// Set controlled train in player controller
		if (ARailsPlayerController* RailsPC = Cast<ARailsPlayerController>(PC))
		{
			RailsPC->SetControlledTrain(ControlledTrain);
		}
	}

	// Broadcast events
	OnConsoleActivated.Broadcast(Operator);
	BP_OnOperationStarted(Operator);

	UE_LOG(LogTemp, Log, TEXT("Train console: Operation started by %s"), *Operator->GetName());
}

void ATrainConsole::StopOperation()
{
	if (!CurrentOperator)
	{
		return;
	}

	// Remove input context and clear controlled train
	if (CachedController)
	{
		RemoveInputContext(CachedController);

		// Clear controlled train in player controller
		if (ARailsPlayerController* RailsPC = Cast<ARailsPlayerController>(CachedController))
		{
			RailsPC->SetControlledTrain(nullptr);
		}

		CachedController = nullptr;
	}

	CurrentOperator = nullptr;

	// Broadcast events
	OnConsoleDeactivated.Broadcast();
	BP_OnOperationStopped();

	UE_LOG(LogTemp, Log, TEXT("Train console: Operation stopped"));
}

void ATrainConsole::ForceStopOperation()
{
	StopOperation();
}

void ATrainConsole::AddInputContext(APlayerController* PC)
{
	if (!PC || !TrainControlMappingContext)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(TrainControlMappingContext, InputContextPriority);
		UE_LOG(LogTemp, Log, TEXT("Train console: Added train control input context"));
	}
}

void ATrainConsole::RemoveInputContext(APlayerController* PC)
{
	if (!PC || !TrainControlMappingContext)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->RemoveMappingContext(TrainControlMappingContext);
		UE_LOG(LogTemp, Log, TEXT("Train console: Removed train control input context"));
	}
}
