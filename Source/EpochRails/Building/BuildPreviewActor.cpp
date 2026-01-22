// BuildPreviewActor.cpp

#include "BuildPreviewActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

ABuildPreviewActor::ABuildPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Don't collide with anything
	SetActorEnableCollision(false);

	// Root
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Preview mesh
	PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(Root);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMesh->SetCastShadow(false);
	PreviewMesh->SetReceivesDecals(false);
}

void ABuildPreviewActor::BeginPlay()
{
	Super::BeginPlay();
}

void ABuildPreviewActor::InitializePreview(TSubclassOf<AActor> StructureClass)
{
	if (!StructureClass)
	{
		return;
	}

	// Get the CDO to find mesh
	AActor* CDO = StructureClass->GetDefaultObject<AActor>();
	if (!CDO)
	{
		return;
	}

	// Find first static mesh component
	TArray<UStaticMeshComponent*> MeshComponents;
	CDO->GetComponents<UStaticMeshComponent>(MeshComponents);

	if (MeshComponents.Num() > 0 && MeshComponents[0]->GetStaticMesh())
	{
		InitializeWithMesh(MeshComponents[0]->GetStaticMesh());
	}

	BP_OnPreviewInitialized(StructureClass);
}

void ABuildPreviewActor::InitializeWithMesh(UStaticMesh* Mesh)
{
	if (!PreviewMesh || !Mesh)
	{
		return;
	}

	PreviewMesh->SetStaticMesh(Mesh);
	CreateDynamicMaterial();
	UpdateMaterial();
}

void ABuildPreviewActor::SetPreviewVisible(bool bVisible)
{
	SetActorHiddenInGame(!bVisible);
}

void ABuildPreviewActor::SetPlacementValid(bool bValid)
{
	if (bIsPlacementValid != bValid)
	{
		bIsPlacementValid = bValid;
		UpdateMaterial();
		BP_OnValidityChanged(bValid);
	}
}

void ABuildPreviewActor::CreateDynamicMaterial()
{
	if (!PreviewMesh)
	{
		return;
	}

	// Use valid material as base, or create translucent material
	UMaterialInterface* BaseMaterial = ValidMaterial;
	if (!BaseMaterial)
	{
		// Try to get material from mesh
		BaseMaterial = PreviewMesh->GetMaterial(0);
	}

	if (BaseMaterial)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		if (DynamicMaterial)
		{
			// Apply to all material slots
			int32 NumMaterials = PreviewMesh->GetNumMaterials();
			for (int32 i = 0; i < NumMaterials; ++i)
			{
				PreviewMesh->SetMaterial(i, DynamicMaterial);
			}
		}
	}
}

void ABuildPreviewActor::UpdateMaterial()
{
	if (!PreviewMesh)
	{
		return;
	}

	// If we have specific valid/invalid materials, use them
	if (ValidMaterial && InvalidMaterial)
	{
		UMaterialInterface* MaterialToUse = bIsPlacementValid ? ValidMaterial : InvalidMaterial;

		int32 NumMaterials = PreviewMesh->GetNumMaterials();
		for (int32 i = 0; i < NumMaterials; ++i)
		{
			PreviewMesh->SetMaterial(i, MaterialToUse);
		}
		return;
	}

	// Otherwise, use dynamic material with color parameter
	if (DynamicMaterial)
	{
		FLinearColor Color = bIsPlacementValid ? ValidColor : InvalidColor;
		DynamicMaterial->SetVectorParameterValue(ColorParameterName, Color);
	}
}
