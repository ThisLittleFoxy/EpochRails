// BuildingComponent.cpp

#include "BuildingComponent.h"
#include "BuildPreviewActor.h"
#include "Train/RollingStockActor.h"
#include "Train/TrainConfigDataAsset.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"

UBuildingComponent::UBuildingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // Only tick in build mode
}

void UBuildingComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache player controller
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		CachedController = Cast<APlayerController>(OwnerPawn->GetController());
	}
}

void UBuildingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up
	if (bInBuildMode)
	{
		ExitBuildMode();
	}

	Super::EndPlay(EndPlayReason);
}

void UBuildingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bInBuildMode)
	{
		UpdatePreview();
	}
}

// ===== Build Mode Control =====

void UBuildingComponent::EnterBuildMode(TSubclassOf<AActor> StructureClass)
{
	if (!StructureClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildingComponent: Cannot enter build mode without structure class"));
		return;
	}

	if (bInBuildMode)
	{
		// Already in build mode - switch structure
		CurrentStructureClass = StructureClass;
		DestroyPreview();
		SpawnPreview();
		return;
	}

	CurrentStructureClass = StructureClass;
	bInBuildMode = true;
	UserRotationOffset = 0.0f;

	// Get structure extent from CDO
	if (AActor* CDO = StructureClass->GetDefaultObject<AActor>())
	{
		FVector Origin, BoxExtent;
		CDO->GetActorBounds(true, Origin, BoxExtent);
		StructureExtent = BoxExtent;
	}

	// Enable tick
	SetComponentTickEnabled(true);

	// Add input context
	AddBuildInputContext();

	// Spawn preview
	SpawnPreview();

	// Broadcast
	OnBuildModeEntered.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("BuildingComponent: Entered build mode with %s"), *StructureClass->GetName());
}

void UBuildingComponent::ExitBuildMode()
{
	if (!bInBuildMode)
	{
		return;
	}

	bInBuildMode = false;
	bPlacementValid = false;
	CurrentStructureClass = nullptr;
	TargetWagon = nullptr;

	// Disable tick
	SetComponentTickEnabled(false);

	// Remove input context
	RemoveBuildInputContext();

	// Destroy preview
	DestroyPreview();

	// Broadcast
	OnBuildModeExited.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("BuildingComponent: Exited build mode"));
}

void UBuildingComponent::ToggleBuildMode()
{
	if (bInBuildMode)
	{
		ExitBuildMode();
	}
	// Cannot enter without structure class - use EnterBuildMode directly
}

// ===== Placement Control =====

void UBuildingComponent::SetPlacementMode(EBuildPlacementMode NewMode)
{
	if (PlacementMode != NewMode)
	{
		PlacementMode = NewMode;
		OnPlacementModeChanged.Broadcast(NewMode);

		UE_LOG(LogTemp, Log, TEXT("BuildingComponent: Placement mode changed to %s"),
			NewMode == EBuildPlacementMode::GridSnap ? TEXT("Grid Snap") : TEXT("Free"));
	}
}

void UBuildingComponent::TogglePlacementMode()
{
	SetPlacementMode(PlacementMode == EBuildPlacementMode::GridSnap
		? EBuildPlacementMode::FreePlacement
		: EBuildPlacementMode::GridSnap);
}

void UBuildingComponent::RotatePreview(float AngleDegrees)
{
	if (PlacementMode == EBuildPlacementMode::GridSnap && RotationSnapAngle > 0.0f)
	{
		// Snap rotation
		UserRotationOffset += FMath::Sign(AngleDegrees) * RotationSnapAngle;
	}
	else
	{
		UserRotationOffset += AngleDegrees;
	}

	// Normalize
	UserRotationOffset = FMath::Fmod(UserRotationOffset + 360.0f, 360.0f);
}

bool UBuildingComponent::ConfirmPlacement()
{
	if (!bInBuildMode || !bPlacementValid || !CurrentStructureClass || !TargetWagon)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildingComponent: Cannot confirm placement - invalid state"));
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Spawn the actual structure
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FTransform SpawnTransform(CurrentPreviewRotation, CurrentPreviewPosition);

	AActor* NewStructure = World->SpawnActor<AActor>(CurrentStructureClass, SpawnTransform, SpawnParams);
	if (!NewStructure)
	{
		UE_LOG(LogTemp, Error, TEXT("BuildingComponent: Failed to spawn structure"));
		return false;
	}

	// Attach to wagon
	if (!TargetWagon->AttachStructure(NewStructure))
	{
		UE_LOG(LogTemp, Error, TEXT("BuildingComponent: Failed to attach structure to wagon"));
		NewStructure->Destroy();
		return false;
	}

	// Broadcast
	OnStructurePlaced.Broadcast(NewStructure);

	UE_LOG(LogTemp, Log, TEXT("BuildingComponent: Placed %s on wagon %d"),
		*NewStructure->GetName(), TargetWagon->CarIndex);

	// Stay in build mode for continued building
	return true;
}

void UBuildingComponent::CancelPlacement()
{
	ExitBuildMode();
}

// ===== State Queries =====

FVector UBuildingComponent::GetPreviewLocation() const
{
	return CurrentPreviewPosition;
}

FRotator UBuildingComponent::GetPreviewRotation() const
{
	return CurrentPreviewRotation;
}

// ===== Internal =====

void UBuildingComponent::UpdatePreview()
{
	// Trace for build surface
	FHitResult Hit;
	if (!TraceBuildSurface(Hit))
	{
		bPlacementValid = false;
		TargetWagon = nullptr;
		if (PreviewActor)
		{
			PreviewActor->SetPreviewVisible(false);
		}
		return;
	}

	// Find wagon
	ARollingStockActor* Wagon = FindWagonFromHit(Hit);
	if (!Wagon)
	{
		bPlacementValid = false;
		TargetWagon = nullptr;
		if (PreviewActor)
		{
			PreviewActor->SetPreviewVisible(false);
		}
		return;
	}

	TargetWagon = Wagon;

	// Calculate position
	FVector WorldPosition = Hit.ImpactPoint;

	// Apply snapping if in grid mode
	if (PlacementMode == EBuildPlacementMode::GridSnap)
	{
		WorldPosition = CalculateSnappedPosition(WorldPosition, Wagon);
	}

	CurrentPreviewPosition = WorldPosition;

	// Calculate rotation (relative to wagon + user offset)
	FRotator WagonRotation = Wagon->GetActorRotation();
	CurrentPreviewRotation = FRotator(0.0f, WagonRotation.Yaw + UserRotationOffset, 0.0f);

	// Validate placement
	bPlacementValid = ValidatePlacement();

	// Update preview actor
	if (PreviewActor)
	{
		PreviewActor->SetPreviewVisible(true);
		PreviewActor->SetActorLocationAndRotation(CurrentPreviewPosition, CurrentPreviewRotation);
		PreviewActor->SetPlacementValid(bPlacementValid);
	}
}

bool UBuildingComponent::TraceBuildSurface(FHitResult& OutHit) const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	// Get view point
	FVector ViewLocation;
	FRotator ViewRotation;

	if (CachedController)
	{
		CachedController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}
	else if (ACharacter* OwnerChar = Cast<ACharacter>(Owner))
	{
		ViewLocation = OwnerChar->GetPawnViewLocation();
		ViewRotation = OwnerChar->GetViewRotation();
	}
	else
	{
		ViewLocation = Owner->GetActorLocation();
		ViewRotation = Owner->GetActorRotation();
	}

	FVector TraceEnd = ViewLocation + ViewRotation.Vector() * MaxBuildDistance;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);
	if (PreviewActor)
	{
		QueryParams.AddIgnoredActor(PreviewActor);
	}

	return GetWorld()->LineTraceSingleByChannel(
		OutHit,
		ViewLocation,
		TraceEnd,
		BuildTraceChannel,
		QueryParams
	);
}

ARollingStockActor* UBuildingComponent::FindWagonFromHit(const FHitResult& Hit) const
{
	if (!Hit.GetActor())
	{
		return nullptr;
	}

	// Direct hit on wagon
	if (ARollingStockActor* Wagon = Cast<ARollingStockActor>(Hit.GetActor()))
	{
		return Wagon;
	}

	// Check if hit actor is attached to a wagon
	AActor* Parent = Hit.GetActor()->GetAttachParentActor();
	while (Parent)
	{
		if (ARollingStockActor* Wagon = Cast<ARollingStockActor>(Parent))
		{
			return Wagon;
		}
		Parent = Parent->GetAttachParentActor();
	}

	return nullptr;
}

FVector UBuildingComponent::CalculateSnappedPosition(const FVector& WorldPosition, ARollingStockActor* Wagon) const
{
	if (!Wagon)
	{
		return WorldPosition;
	}

	// Convert to local space
	FVector LocalPos = Wagon->WorldToBuildLocal(WorldPosition);

	// Get grid size
	float SnapGrid = GridSize;
	if (SnapGrid <= 0.0f && Wagon->Config)
	{
		SnapGrid = Wagon->Config->BuildGridSize;
	}
	if (SnapGrid <= 0.0f)
	{
		SnapGrid = 50.0f; // Fallback
	}

	// Snap to grid
	FVector SnappedLocal = Wagon->SnapToGrid(LocalPos, SnapGrid);

	// Keep Z at surface level (or snapped to grid if above)
	if (SnappedLocal.Z < SnapGrid)
	{
		SnappedLocal.Z = 0.0f; // On floor
	}

	// Convert back to world
	return Wagon->BuildLocalToWorld(SnappedLocal);
}

bool UBuildingComponent::ValidatePlacement() const
{
	if (!TargetWagon)
	{
		return false;
	}

	// Convert position to local
	FVector LocalPos = TargetWagon->WorldToBuildLocal(CurrentPreviewPosition);

	// Check wagon bounds
	return TargetWagon->CanPlaceStructure(LocalPos, StructureExtent);
}

void UBuildingComponent::SpawnPreview()
{
	if (!PreviewActorClass || !CurrentStructureClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PreviewActor = World->SpawnActor<ABuildPreviewActor>(PreviewActorClass, FTransform::Identity, SpawnParams);
	if (PreviewActor)
	{
		PreviewActor->InitializePreview(CurrentStructureClass);
		PreviewActor->SetPreviewVisible(false);
	}
}

void UBuildingComponent::DestroyPreview()
{
	if (PreviewActor && IsValid(PreviewActor))
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}
}

void UBuildingComponent::AddBuildInputContext()
{
	if (!CachedController || !BuildMappingContext)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(CachedController->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(BuildMappingContext, BuildInputPriority);
	}
}

void UBuildingComponent::RemoveBuildInputContext()
{
	if (!CachedController || !BuildMappingContext)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(CachedController->GetLocalPlayer()))
	{
		Subsystem->RemoveMappingContext(BuildMappingContext);
	}
}
