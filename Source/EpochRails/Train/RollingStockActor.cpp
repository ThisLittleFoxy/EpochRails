// RollingStockActor.cpp

#include "RollingStockActor.h"
#include "TrainConfigDataAsset.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

ARollingStockActor::ARollingStockActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root component
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Platform mesh - the wagon floor/body
	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	PlatformMesh->SetupAttachment(Root);
	PlatformMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Collision box - separate for precise control
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(Root);
	CollisionBox->SetBoxExtent(FVector(200.0f, 100.0f, 10.0f));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionProfileName(TEXT("BlockAll"));
	// Enable character to walk on this
	CollisionBox->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Default, 0.0f));
	CollisionBox->CanCharacterStepUpOn = ECB_Yes;

	// Build root - coordinate system for building structures
	BuildRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BuildRoot"));
	BuildRoot->SetupAttachment(Root);
	// Origin at center of wagon floor surface
	BuildRoot->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));

	// Front coupler
	FrontCoupler = CreateDefaultSubobject<USceneComponent>(TEXT("FrontCoupler"));
	FrontCoupler->SetupAttachment(Root);
	FrontCoupler->SetRelativeLocation(FVector(200.0f, 0.0f, 0.0f));

	// Rear coupler
	RearCoupler = CreateDefaultSubobject<USceneComponent>(TEXT("RearCoupler"));
	RearCoupler->SetupAttachment(Root);
	RearCoupler->SetRelativeLocation(FVector(-200.0f, 0.0f, 0.0f));

	// Buildable zone visualization (hidden in game)
	BuildableZoneVisual = CreateDefaultSubobject<UBoxComponent>(TEXT("BuildableZoneVisual"));
	BuildableZoneVisual->SetupAttachment(BuildRoot);
	BuildableZoneVisual->SetBoxExtent(FVector(200.0f, 100.0f, 150.0f));
	BuildableZoneVisual->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
	BuildableZoneVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BuildableZoneVisual->SetHiddenInGame(true);
	BuildableZoneVisual->ShapeColor = FColor::Green;

	// Default cached values
	WagonHalfExtent = FVector(200.0f, 100.0f, 10.0f);
	MaxBuildHeight = 300.0f;
	GridSize = 50.0f;
}

void ARollingStockActor::BeginPlay()
{
	Super::BeginPlay();
}

void ARollingStockActor::Initialize(int32 InCarIndex, float InDistanceOffset, const UTrainConfigDataAsset* InConfig)
{
	CarIndex = InCarIndex;
	DistanceOffset = InDistanceOffset;
	Config = InConfig;

	if (Config)
	{
		UpdateFromConfig();
	}
}

void ARollingStockActor::UpdateFromConfig()
{
	if (!Config)
	{
		return;
	}

	// Cache values from config
	WagonHalfExtent = FVector(Config->WagonLength * 0.5f, Config->WagonWidth * 0.5f, 10.0f);
	MaxBuildHeight = Config->MaxBuildHeight;
	GridSize = Config->BuildGridSize;

	// Update collision box
	if (CollisionBox)
	{
		CollisionBox->SetBoxExtent(WagonHalfExtent);
	}

	// Update coupler positions
	if (FrontCoupler)
	{
		FrontCoupler->SetRelativeLocation(FVector(WagonHalfExtent.X, 0.0f, 0.0f));
	}
	if (RearCoupler)
	{
		RearCoupler->SetRelativeLocation(FVector(-WagonHalfExtent.X, 0.0f, 0.0f));
	}

	// Update buildable zone visual
	if (BuildableZoneVisual)
	{
		BuildableZoneVisual->SetBoxExtent(FVector(WagonHalfExtent.X, WagonHalfExtent.Y, MaxBuildHeight * 0.5f));
		BuildableZoneVisual->SetRelativeLocation(FVector(0.0f, 0.0f, MaxBuildHeight * 0.5f));
	}
}

// ===== Building API =====

bool ARollingStockActor::CanPlaceStructure(const FVector& LocalLocation, const FVector& StructureExtent) const
{
	// Check X bounds (length)
	if (FMath::Abs(LocalLocation.X) + StructureExtent.X > WagonHalfExtent.X)
	{
		return false;
	}

	// Check Y bounds (width)
	if (FMath::Abs(LocalLocation.Y) + StructureExtent.Y > WagonHalfExtent.Y)
	{
		return false;
	}

	// Check Z bounds (must be above floor, below max height)
	if (LocalLocation.Z < 0.0f || LocalLocation.Z + StructureExtent.Z > MaxBuildHeight)
	{
		return false;
	}

	return true;
}

bool ARollingStockActor::AttachStructure(AActor* Structure)
{
	if (!Structure || !BuildRoot)
	{
		return false;
	}

	// Attach to BuildRoot - structure will move with the wagon
	Structure->AttachToComponent(BuildRoot, FAttachmentTransformRules::KeepWorldTransform);

	// Track for queries
	AttachedStructures.Add(Structure);

	UE_LOG(LogTemp, Log, TEXT("Structure %s attached to wagon %d"), *Structure->GetName(), CarIndex);
	return true;
}

bool ARollingStockActor::DetachStructure(AActor* Structure)
{
	if (!Structure)
	{
		return false;
	}

	// Find in array
	int32 Index = AttachedStructures.IndexOfByPredicate([Structure](const TWeakObjectPtr<AActor>& WeakActor)
	{
		return WeakActor.IsValid() && WeakActor.Get() == Structure;
	});

	if (Index == INDEX_NONE)
	{
		return false;
	}

	// Detach from wagon
	Structure->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// Remove from tracking
	AttachedStructures.RemoveAt(Index);

	UE_LOG(LogTemp, Log, TEXT("Structure %s detached from wagon %d"), *Structure->GetName(), CarIndex);
	return true;
}

TArray<AActor*> ARollingStockActor::GetAttachedStructures() const
{
	TArray<AActor*> Result;
	Result.Reserve(AttachedStructures.Num());

	for (const TWeakObjectPtr<AActor>& WeakActor : AttachedStructures)
	{
		if (WeakActor.IsValid())
		{
			Result.Add(WeakActor.Get());
		}
	}

	return Result;
}

FBox ARollingStockActor::GetBuildableZoneBounds() const
{
	FVector Min(-WagonHalfExtent.X, -WagonHalfExtent.Y, 0.0f);
	FVector Max(WagonHalfExtent.X, WagonHalfExtent.Y, MaxBuildHeight);
	return FBox(Min, Max);
}

FVector ARollingStockActor::WorldToBuildLocal(const FVector& WorldLocation) const
{
	if (!BuildRoot)
	{
		return FVector::ZeroVector;
	}
	return BuildRoot->GetComponentTransform().InverseTransformPosition(WorldLocation);
}

FVector ARollingStockActor::BuildLocalToWorld(const FVector& LocalLocation) const
{
	if (!BuildRoot)
	{
		return LocalLocation;
	}
	return BuildRoot->GetComponentTransform().TransformPosition(LocalLocation);
}

FVector ARollingStockActor::SnapToGrid(const FVector& LocalLocation, float InGridSize) const
{
	float SnapSize = (InGridSize > 0.0f) ? InGridSize : GridSize;

	if (SnapSize <= 0.0f)
	{
		return LocalLocation;
	}

	return FVector(
		FMath::RoundToFloat(LocalLocation.X / SnapSize) * SnapSize,
		FMath::RoundToFloat(LocalLocation.Y / SnapSize) * SnapSize,
		FMath::RoundToFloat(LocalLocation.Z / SnapSize) * SnapSize
	);
}

FVector ARollingStockActor::GetFrontCouplerLocation() const
{
	return FrontCoupler ? FrontCoupler->GetComponentLocation() : GetActorLocation();
}

FVector ARollingStockActor::GetRearCouplerLocation() const
{
	return RearCoupler ? RearCoupler->GetComponentLocation() : GetActorLocation();
}
