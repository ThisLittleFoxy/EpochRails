// BuildingComponent.h
// Component for building system - handles placement preview and structure spawning

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuildingComponent.generated.h"

class ABuildPreviewActor;
class ARollingStockActor;
class UInputMappingContext;
class UInputAction;
class APlayerController;

/**
 * Placement mode for building
 */
UENUM(BlueprintType)
enum class EBuildPlacementMode : uint8
{
	GridSnap    UMETA(DisplayName = "Grid Snap"),
	FreePlacement UMETA(DisplayName = "Free Placement")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBuildModeEntered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBuildModeExited);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStructurePlaced, AActor*, PlacedStructure);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlacementModeChanged, EBuildPlacementMode, NewMode);

/**
 * Building component - add to player character to enable building.
 * Manages build mode state, preview ghost, and structure placement.
 *
 * Supports two placement modes:
 * - Grid Snap: Snaps to grid in BuildRoot local coordinates
 * - Free Placement: No snap, place anywhere within bounds
 */
UCLASS(ClassGroup = (Building), meta = (BlueprintSpawnableComponent))
class EPOCHRAILS_API UBuildingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuildingComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===== Build Mode Control =====

	/** Enter build mode with a specific structure class */
	UFUNCTION(BlueprintCallable, Category = "Building")
	void EnterBuildMode(TSubclassOf<AActor> StructureClass);

	/** Exit build mode and destroy preview */
	UFUNCTION(BlueprintCallable, Category = "Building")
	void ExitBuildMode();

	/** Toggle build mode (if in mode, exit; if not, do nothing - needs structure class) */
	UFUNCTION(BlueprintCallable, Category = "Building")
	void ToggleBuildMode();

	/** Check if currently in build mode */
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool IsInBuildMode() const { return bInBuildMode; }

	// ===== Placement Control =====

	/** Set placement mode (grid snap or free) */
	UFUNCTION(BlueprintCallable, Category = "Building")
	void SetPlacementMode(EBuildPlacementMode NewMode);

	/** Toggle between grid snap and free placement */
	UFUNCTION(BlueprintCallable, Category = "Building")
	void TogglePlacementMode();

	/** Get current placement mode */
	UFUNCTION(BlueprintCallable, Category = "Building")
	EBuildPlacementMode GetPlacementMode() const { return PlacementMode; }

	/** Rotate preview structure by angle */
	UFUNCTION(BlueprintCallable, Category = "Building")
	void RotatePreview(float AngleDegrees);

	/** Confirm placement - spawn actual structure */
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool ConfirmPlacement();

	/** Cancel placement (same as exit build mode) */
	UFUNCTION(BlueprintCallable, Category = "Building")
	void CancelPlacement();

	// ===== State Queries =====

	/** Check if current preview position is valid */
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool IsPlacementValid() const { return bPlacementValid; }

	/** Get the wagon we're currently building on (if any) */
	UFUNCTION(BlueprintCallable, Category = "Building")
	ARollingStockActor* GetTargetWagon() const { return TargetWagon; }

	/** Get current preview world location */
	UFUNCTION(BlueprintCallable, Category = "Building")
	FVector GetPreviewLocation() const;

	/** Get current preview rotation */
	UFUNCTION(BlueprintCallable, Category = "Building")
	FRotator GetPreviewRotation() const;

	// ===== Configuration =====

	/** Preview actor class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Config")
	TSubclassOf<ABuildPreviewActor> PreviewActorClass;

	/** Input mapping context for build mode */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Input")
	TObjectPtr<UInputMappingContext> BuildMappingContext;

	/** Priority for build input context */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Input")
	int32 BuildInputPriority = 2;

	/** Maximum distance to look for wagon surfaces */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Config")
	float MaxBuildDistance = 500.0f;

	/** Grid size for snap mode (0 = use wagon config) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Config")
	float GridSize = 0.0f;

	/** Rotation snap angle for grid mode */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Config")
	float RotationSnapAngle = 15.0f;

	/** Trace channel for finding build surfaces */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Config")
	TEnumAsByte<ECollisionChannel> BuildTraceChannel = ECC_Visibility;

	// ===== Events =====

	UPROPERTY(BlueprintAssignable, Category = "Building|Events")
	FOnBuildModeEntered OnBuildModeEntered;

	UPROPERTY(BlueprintAssignable, Category = "Building|Events")
	FOnBuildModeExited OnBuildModeExited;

	UPROPERTY(BlueprintAssignable, Category = "Building|Events")
	FOnStructurePlaced OnStructurePlaced;

	UPROPERTY(BlueprintAssignable, Category = "Building|Events")
	FOnPlacementModeChanged OnPlacementModeChanged;

private:
	/** Update preview position based on player view */
	void UpdatePreview();

	/** Perform trace to find build surface */
	bool TraceBuildSurface(FHitResult& OutHit) const;

	/** Find wagon from hit result */
	ARollingStockActor* FindWagonFromHit(const FHitResult& Hit) const;

	/** Calculate snapped position */
	FVector CalculateSnappedPosition(const FVector& WorldPosition, ARollingStockActor* Wagon) const;

	/** Validate placement at current position */
	bool ValidatePlacement() const;

	/** Spawn preview actor */
	void SpawnPreview();

	/** Destroy preview actor */
	void DestroyPreview();

	/** Add/remove build input context */
	void AddBuildInputContext();
	void RemoveBuildInputContext();

	// ===== State =====

	/** Currently in build mode */
	bool bInBuildMode = false;

	/** Current placement is valid */
	bool bPlacementValid = false;

	/** Current placement mode */
	UPROPERTY()
	EBuildPlacementMode PlacementMode = EBuildPlacementMode::GridSnap;

	/** Structure class being placed */
	UPROPERTY()
	TSubclassOf<AActor> CurrentStructureClass;

	/** Preview actor instance */
	UPROPERTY()
	TObjectPtr<ABuildPreviewActor> PreviewActor;

	/** Target wagon for placement */
	UPROPERTY()
	TObjectPtr<ARollingStockActor> TargetWagon;

	/** Current preview position (world) */
	FVector CurrentPreviewPosition = FVector::ZeroVector;

	/** Current preview rotation */
	FRotator CurrentPreviewRotation = FRotator::ZeroRotator;

	/** User rotation offset */
	float UserRotationOffset = 0.0f;

	/** Cached structure extent for validation */
	FVector StructureExtent = FVector(50.0f);

	/** Cached player controller */
	UPROPERTY()
	TObjectPtr<APlayerController> CachedController;
};
