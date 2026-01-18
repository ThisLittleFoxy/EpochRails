// RollingStockActor.h
// Base class for train cars/wagons that move along the track

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RollingStockActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class USceneComponent;
class UTrainConfigDataAsset;

/**
 * Rolling stock actor - represents a wagon/car in the train.
 * Contains collision, mesh, and BuildRoot for attaching structures.
 * Transform is updated by the owning ATrainActor's movement component.
 */
UCLASS(BlueprintType, Blueprintable)
class EPOCHRAILS_API ARollingStockActor : public AActor
{
	GENERATED_BODY()

public:
	ARollingStockActor();

protected:
	virtual void BeginPlay() override;

public:
	// ===== Components =====

	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	/** Visual mesh for the wagon platform */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PlatformMesh;

	/** Collision box for the wagon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionBox;

	/**
	 * Build root - coordinate system for building.
	 * All structures are attached to this component.
	 * Origin is at the center of the wagon floor.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> BuildRoot;

	/** Front coupler attachment point */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> FrontCoupler;

	/** Rear coupler attachment point */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> RearCoupler;

	/** Visual bounds for buildable zone (editor only) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> BuildableZoneVisual;

	// ===== Configuration =====

	/** Distance offset from the locomotive along the spline (set by ATrainActor) */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Train")
	float DistanceOffset = 0.0f;

	/** Index in the train (0 = first wagon after locomotive) */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Train")
	int32 CarIndex = 0;

	/** Reference to train config (set by ATrainActor) */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Train")
	TObjectPtr<const UTrainConfigDataAsset> Config;

	// ===== Building API =====

	/**
	 * Check if a structure can be placed at the given location.
	 * @param LocalLocation Location in BuildRoot local space
	 * @param StructureExtent Half-extents of the structure's bounding box
	 * @return True if placement is valid
	 */
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool CanPlaceStructure(const FVector& LocalLocation, const FVector& StructureExtent) const;

	/**
	 * Attach a structure to this wagon's BuildRoot.
	 * The structure will move with the wagon.
	 * @param Structure The actor to attach
	 * @return True if attachment succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool AttachStructure(AActor* Structure);

	/**
	 * Detach a structure from this wagon.
	 * @param Structure The actor to detach
	 * @return True if detachment succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool DetachStructure(AActor* Structure);

	/**
	 * Get all structures attached to this wagon.
	 */
	UFUNCTION(BlueprintCallable, Category = "Building")
	TArray<AActor*> GetAttachedStructures() const;

	/**
	 * Get the buildable zone bounds in local space.
	 */
	UFUNCTION(BlueprintCallable, Category = "Building")
	FBox GetBuildableZoneBounds() const;

	/**
	 * Convert world location to BuildRoot local space.
	 */
	UFUNCTION(BlueprintCallable, Category = "Building")
	FVector WorldToBuildLocal(const FVector& WorldLocation) const;

	/**
	 * Convert BuildRoot local location to world space.
	 */
	UFUNCTION(BlueprintCallable, Category = "Building")
	FVector BuildLocalToWorld(const FVector& LocalLocation) const;

	/**
	 * Snap a local position to the building grid.
	 * @param LocalLocation Location in BuildRoot local space
	 * @param GridSize Grid cell size (from config if 0)
	 * @return Snapped location
	 */
	UFUNCTION(BlueprintCallable, Category = "Building")
	FVector SnapToGrid(const FVector& LocalLocation, float GridSize = 0.0f) const;

	// ===== Internal =====

	/** Called by ATrainActor to initialize this car */
	void Initialize(int32 InCarIndex, float InDistanceOffset, const UTrainConfigDataAsset* InConfig);

	/** Update visual components based on config */
	void UpdateFromConfig();

	/** Get front coupler world location */
	UFUNCTION(BlueprintCallable, Category = "Train")
	FVector GetFrontCouplerLocation() const;

	/** Get rear coupler world location */
	UFUNCTION(BlueprintCallable, Category = "Train")
	FVector GetRearCouplerLocation() const;

private:
	/** Structures attached to this wagon */
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> AttachedStructures;

	/** Cached dimensions */
	FVector WagonHalfExtent;
	float MaxBuildHeight;
	float GridSize;
};
