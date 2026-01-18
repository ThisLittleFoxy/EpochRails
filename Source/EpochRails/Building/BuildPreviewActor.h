// BuildPreviewActor.h
// Ghost preview actor for building placement

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildPreviewActor.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

/**
 * Preview/ghost actor shown during building placement.
 * Changes color based on placement validity.
 */
UCLASS(BlueprintType, Blueprintable)
class EPOCHRAILS_API ABuildPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	ABuildPreviewActor();

protected:
	virtual void BeginPlay() override;

public:
	// ===== Initialization =====

	/**
	 * Initialize preview with a structure class.
	 * Creates mesh from the structure's visual components.
	 */
	UFUNCTION(BlueprintCallable, Category = "Building")
	void InitializePreview(TSubclassOf<AActor> StructureClass);

	/**
	 * Initialize with a specific static mesh.
	 */
	UFUNCTION(BlueprintCallable, Category = "Building")
	void InitializeWithMesh(UStaticMesh* Mesh);

	// ===== Display Control =====

	/** Set whether preview is visible */
	UFUNCTION(BlueprintCallable, Category = "Building")
	void SetPreviewVisible(bool bVisible);

	/** Set placement validity (changes color) */
	UFUNCTION(BlueprintCallable, Category = "Building")
	void SetPlacementValid(bool bValid);

	/** Check if placement is valid */
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool IsPlacementValid() const { return bIsPlacementValid; }

	// ===== Configuration =====

	/** Material to use for valid placement (green ghost) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview|Materials")
	TObjectPtr<UMaterialInterface> ValidMaterial;

	/** Material to use for invalid placement (red ghost) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview|Materials")
	TObjectPtr<UMaterialInterface> InvalidMaterial;

	/** Color for valid placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview|Colors")
	FLinearColor ValidColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.5f);

	/** Color for invalid placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview|Colors")
	FLinearColor InvalidColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.5f);

	/** Parameter name for color in material */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview|Materials")
	FName ColorParameterName = TEXT("PreviewColor");

	// ===== Blueprint Events =====

	/** Called when preview is initialized */
	UFUNCTION(BlueprintImplementableEvent, Category = "Building")
	void BP_OnPreviewInitialized(TSubclassOf<AActor> StructureClass);

	/** Called when validity changes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Building")
	void BP_OnValidityChanged(bool bValid);

protected:
	/** The preview mesh component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PreviewMesh;

	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

private:
	/** Apply current material based on validity */
	void UpdateMaterial();

	/** Create dynamic material instance */
	void CreateDynamicMaterial();

	/** Current placement validity */
	bool bIsPlacementValid = false;

	/** Dynamic material instance */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;
};
