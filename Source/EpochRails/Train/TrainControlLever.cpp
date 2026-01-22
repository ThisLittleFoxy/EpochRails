// TrainControlLever.cpp

#include "TrainControlLever.h"
#include "TrainActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"

ATrainControlLever::ATrainControlLever()
{
	PrimaryActorTick.bCanEverTick = true;

	// Root
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Base mesh (static)
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(Root);
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BaseMesh->SetCollisionProfileName(TEXT("BlockAll"));

	// Pivot point
	LeverPivot = CreateDefaultSubobject<USceneComponent>(TEXT("LeverPivot"));
	LeverPivot->SetupAttachment(Root);
	LeverPivot->SetRelativeLocation(FVector(0.0f, 0.0f, 20.0f));

	// Handle mesh (rotates)
	HandleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandleMesh"));
	HandleMesh->SetupAttachment(LeverPivot);
	HandleMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HandleMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	HandleMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void ATrainControlLever::BeginPlay()
{
	Super::BeginPlay();

	// Try to find train if not set
	if (!ControlledTrain)
	{
		AActor* Parent = GetAttachParentActor();
		while (Parent)
		{
			if (ATrainActor* Train = Cast<ATrainActor>(Parent))
			{
				ControlledTrain = Train;
				break;
			}
			Parent = Parent->GetAttachParentActor();
		}
	}
}

void ATrainControlLever::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Animate lever
	UpdateLeverRotation(DeltaTime);
}

// ===== IInteractableInterface =====

void ATrainControlLever::OnInteractionFocusBegin_Implementation(ACharacter* PlayerCharacter)
{
	BP_OnFocusBegin(PlayerCharacter);
}

void ATrainControlLever::OnInteractionFocusEnd_Implementation(ACharacter* PlayerCharacter)
{
	BP_OnFocusEnd(PlayerCharacter);
}

bool ATrainControlLever::OnInteract_Implementation(ACharacter* PlayerCharacter)
{
	switch (LeverType)
	{
	case ETrainLeverType::Throttle:
		// Increase throttle by step
		SetValue(FMath::Clamp(CurrentValue + ValueStep, 0.0f, 1.0f));
		// If we hit max, wrap to 0
		if (CurrentValue >= 1.0f)
		{
			SetValue(0.0f);
		}
		break;

	case ETrainLeverType::Brake:
		// Toggle between 0 and full brake
		SetValue(CurrentValue > 0.5f ? 0.0f : 1.0f);
		break;

	case ETrainLeverType::Reverse:
		// Toggle reverse
		bReverseEngaged = !bReverseEngaged;
		if (ControlledTrain)
		{
			ControlledTrain->SetReverse(bReverseEngaged);
		}
		TargetRotation = bReverseEngaged ? MaxRotationAngle : 0.0f;
		OnValueChanged.Broadcast(LeverType, bReverseEngaged ? 1.0f : 0.0f);
		break;

	case ETrainLeverType::EmergencyBrake:
		// Toggle emergency brake
		bEmergencyEngaged = !bEmergencyEngaged;
		if (ControlledTrain)
		{
			if (bEmergencyEngaged)
			{
				ControlledTrain->EmergencyBrake();
			}
			else
			{
				ControlledTrain->ReleaseEmergencyBrake();
			}
		}
		TargetRotation = bEmergencyEngaged ? MaxRotationAngle : 0.0f;
		OnValueChanged.Broadcast(LeverType, bEmergencyEngaged ? 1.0f : 0.0f);
		break;
	}

	BP_OnLeverActivated();

	UE_LOG(LogTemp, Log, TEXT("Lever %s activated, value: %.2f"),
		*UEnum::GetValueAsString(LeverType), CurrentValue);

	return true;
}

FText ATrainControlLever::GetInteractionName_Implementation() const
{
	switch (LeverType)
	{
	case ETrainLeverType::Throttle:
		return NSLOCTEXT("Train", "LeverThrottle", "Throttle");
	case ETrainLeverType::Brake:
		return NSLOCTEXT("Train", "LeverBrake", "Brake");
	case ETrainLeverType::Reverse:
		return NSLOCTEXT("Train", "LeverReverse", "Reverse");
	case ETrainLeverType::EmergencyBrake:
		return NSLOCTEXT("Train", "LeverEmergency", "Emergency Brake");
	default:
		return NSLOCTEXT("Train", "LeverUnknown", "Control Lever");
	}
}

FText ATrainControlLever::GetInteractionAction_Implementation() const
{
	switch (LeverType)
	{
	case ETrainLeverType::Throttle:
		return FText::Format(NSLOCTEXT("Train", "ThrottleAction", "Press E to adjust ({0}%)"),
			FText::AsNumber(FMath::RoundToInt(CurrentValue * 100.0f)));
	case ETrainLeverType::Brake:
		return CurrentValue > 0.5f
			? NSLOCTEXT("Train", "BrakeRelease", "Press E to release brake")
			: NSLOCTEXT("Train", "BrakeApply", "Press E to apply brake");
	case ETrainLeverType::Reverse:
		return bReverseEngaged
			? NSLOCTEXT("Train", "ReverseOff", "Press E for forward")
			: NSLOCTEXT("Train", "ReverseOn", "Press E for reverse");
	case ETrainLeverType::EmergencyBrake:
		return bEmergencyEngaged
			? NSLOCTEXT("Train", "EmergencyRelease", "Press E to release")
			: NSLOCTEXT("Train", "EmergencyPull", "Press E to pull");
	default:
		return NSLOCTEXT("Train", "LeverUse", "Press E to use");
	}
}

bool ATrainControlLever::CanInteract_Implementation(ACharacter* PlayerCharacter) const
{
	return true;
}

float ATrainControlLever::GetInteractionDistance_Implementation() const
{
	return MaxInteractionDistance;
}

// ===== Value Control =====

void ATrainControlLever::SetValue(float NewValue)
{
	CurrentValue = FMath::Clamp(NewValue, 0.0f, 1.0f);

	// Update target rotation for animation
	TargetRotation = CurrentValue * MaxRotationAngle;

	// Apply to train
	ApplyValueToTrain();

	// Broadcast change
	OnValueChanged.Broadcast(LeverType, CurrentValue);
}

void ATrainControlLever::ResetToNeutral()
{
	SetValue(0.0f);
	bReverseEngaged = false;
	bEmergencyEngaged = false;
}

void ATrainControlLever::ApplyValueToTrain()
{
	if (!ControlledTrain)
	{
		return;
	}

	switch (LeverType)
	{
	case ETrainLeverType::Throttle:
		ControlledTrain->SetThrottle(CurrentValue);
		break;
	case ETrainLeverType::Brake:
		ControlledTrain->SetBrake(CurrentValue);
		break;
	default:
		// Reverse and Emergency are handled in OnInteract
		break;
	}
}

void ATrainControlLever::UpdateLeverRotation(float DeltaTime)
{
	if (!LeverPivot)
	{
		return;
	}

	// Smoothly interpolate to target rotation
	CurrentRotation = FMath::FInterpTo(CurrentRotation, TargetRotation, DeltaTime, AnimationSpeed);

	// Apply rotation to pivot
	FRotator NewRotation = FRotator::ZeroRotator;
	NewRotation += FRotator(
		RotationAxis.Y * CurrentRotation,  // Pitch
		RotationAxis.Z * CurrentRotation,  // Yaw
		RotationAxis.X * CurrentRotation   // Roll
	);

	LeverPivot->SetRelativeRotation(NewRotation);
}
