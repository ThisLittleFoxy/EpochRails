// TrainActor.cpp

#include "TrainActor.h"
#include "TrainConfigDataAsset.h"
#include "RailTrainMovementComponent.h"
#include "RollingStockActor.h"

#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"

ATrainActor::ATrainActor()
{
	// Tick in PrePhysics so character updates after train
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	// Root component
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Locomotive mesh
	LocomotiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LocomotiveMesh"));
	LocomotiveMesh->SetupAttachment(Root);
	LocomotiveMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	LocomotiveMesh->SetCollisionProfileName(TEXT("BlockAll"));

	// Track spline
	TrackSpline = CreateDefaultSubobject<USplineComponent>(TEXT("TrackSpline"));
	TrackSpline->SetupAttachment(Root);
	TrackSpline->SetDrawDebug(true);
	TrackSpline->SetUnselectedSplineSegmentColor(FLinearColor(0.8f, 0.4f, 0.0f));

	// Movement component
	MovementComponent = CreateDefaultSubobject<URailTrainMovementComponent>(TEXT("MovementComponent"));

	// Rear coupler (for reference when attaching wagons)
	RearCoupler = CreateDefaultSubobject<USceneComponent>(TEXT("RearCoupler"));
	RearCoupler->SetupAttachment(Root);
	RearCoupler->SetRelativeLocation(FVector(-300.0f, 0.0f, 0.0f)); // Default, updated from config

	// Default wagon class
	WagonClass = ARollingStockActor::StaticClass();
}

void ATrainActor::BeginPlay()
{
	Super::BeginPlay();

	// Initialize movement
	InitializeMovement();

	// Spawn cars if configured
	if (bAutoSpawnWagons && Config && Config->WagonCount > 0)
	{
		SpawnCars();
	}
}

void ATrainActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up cars
	DestroyCars();

	Super::EndPlay(EndPlayReason);
}

void ATrainActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Movement component ticks separately and updates transforms
}

void ATrainActor::InitializeMovement()
{
	if (MovementComponent)
	{
		MovementComponent->SetTrackSpline(TrackSpline);

		if (Config)
		{
			MovementComponent->SetConfig(Config);

			// Update rear coupler position based on locomotive length
			if (RearCoupler)
			{
				RearCoupler->SetRelativeLocation(FVector(-Config->LocomotiveLength * 0.5f, 0.0f, 0.0f));
			}

			// Update spline debug color
			if (TrackSpline)
			{
				TrackSpline->SetUnselectedSplineSegmentColor(Config->SplineDebugColor);
				TrackSpline->SetDrawDebug(Config->bShowDebugInGame);
			}
		}
	}
}

// ===== Car Management =====

void ATrainActor::SpawnCars()
{
	if (!Config || !WagonClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ATrainActor::SpawnCars - Missing Config or WagonClass"));
		return;
	}

	// Destroy existing cars first
	DestroyCars();

	// Spawn wagons
	int32 WagonCount = Config->WagonCount;
	Cars.Reserve(WagonCount);

	for (int32 i = 0; i < WagonCount; ++i)
	{
		ARollingStockActor* NewCar = AddCar();
		if (!NewCar)
		{
			UE_LOG(LogTemp, Error, TEXT("ATrainActor::SpawnCars - Failed to spawn car %d"), i);
			break;
		}
	}

	// Update movement component with car array
	if (MovementComponent)
	{
		MovementComponent->SetCars(Cars);
	}

	UE_LOG(LogTemp, Log, TEXT("ATrainActor: Spawned %d cars"), Cars.Num());
}

void ATrainActor::DestroyCars()
{
	for (ARollingStockActor* Car : Cars)
	{
		if (Car && IsValid(Car))
		{
			Car->Destroy();
		}
	}
	Cars.Empty();

	if (MovementComponent)
	{
		MovementComponent->SetCars(Cars);
	}
}

ARollingStockActor* ATrainActor::AddCar()
{
	if (!WagonClass)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Calculate index and offset for new car
	int32 NewIndex = Cars.Num();
	float Offset = CalculateCarOffset(NewIndex);

	// Spawn at train's initial position (will be updated by movement)
	FTransform SpawnTransform = GetActorTransform();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ARollingStockActor* NewCar = World->SpawnActor<ARollingStockActor>(WagonClass, SpawnTransform, SpawnParams);
	if (NewCar)
	{
		NewCar->Initialize(NewIndex, Offset, Config);
		Cars.Add(NewCar);

		// Update movement component
		if (MovementComponent)
		{
			MovementComponent->SetCars(Cars);
		}

		UE_LOG(LogTemp, Log, TEXT("Added car %d at offset %.1f"), NewIndex, Offset);
	}

	return NewCar;
}

void ATrainActor::RemoveLastCar()
{
	if (Cars.Num() == 0)
	{
		return;
	}

	ARollingStockActor* LastCar = Cars.Pop();
	if (LastCar && IsValid(LastCar))
	{
		LastCar->Destroy();
	}

	// Update movement component
	if (MovementComponent)
	{
		MovementComponent->SetCars(Cars);
	}
}

ARollingStockActor* ATrainActor::GetCar(int32 Index) const
{
	if (Cars.IsValidIndex(Index))
	{
		return Cars[Index];
	}
	return nullptr;
}

float ATrainActor::CalculateCarOffset(int32 CarIndex) const
{
	if (!Config)
	{
		return 500.0f * (CarIndex + 1);
	}

	// Offset = locomotive half-length + coupling gap + (car index * (wagon length + coupling gap)) + wagon half-length
	float LocoHalfLength = Config->LocomotiveLength * 0.5f;
	float WagonHalfLength = Config->WagonLength * 0.5f;
	float Gap = Config->CouplingGap;

	// First car offset
	float BaseOffset = LocoHalfLength + Gap + WagonHalfLength;

	// Additional offset for subsequent cars
	float PerCarOffset = Config->WagonLength + Gap;

	return BaseOffset + (CarIndex * PerCarOffset);
}

// ===== Movement Control =====

void ATrainActor::SetThrottle(float ThrottleValue)
{
	if (MovementComponent)
	{
		MovementComponent->SetThrottle(ThrottleValue);
	}
}

void ATrainActor::SetBrake(float BrakeValue)
{
	if (MovementComponent)
	{
		MovementComponent->SetBrake(BrakeValue);
	}
}

void ATrainActor::ToggleReverse()
{
	if (MovementComponent)
	{
		MovementComponent->ToggleReverse();
	}
}

void ATrainActor::SetReverse(bool bReverse)
{
	if (MovementComponent)
	{
		MovementComponent->SetReverse(bReverse);
	}
}

void ATrainActor::EmergencyBrake()
{
	if (MovementComponent)
	{
		MovementComponent->EmergencyBrake();
	}
}

void ATrainActor::ReleaseEmergencyBrake()
{
	if (MovementComponent)
	{
		MovementComponent->ReleaseEmergencyBrake();
	}
}

// ===== State Queries =====

float ATrainActor::GetSpeed() const
{
	return MovementComponent ? MovementComponent->GetSpeed() : 0.0f;
}

float ATrainActor::GetSpeedKmh() const
{
	return MovementComponent ? MovementComponent->GetSpeedKmh() : 0.0f;
}

bool ATrainActor::IsReversing() const
{
	return MovementComponent ? MovementComponent->IsReversing() : false;
}

bool ATrainActor::IsMoving() const
{
	return MovementComponent ? MovementComponent->IsMoving() : false;
}

float ATrainActor::GetTrainDistance() const
{
	return MovementComponent ? MovementComponent->GetTrainDistance() : 0.0f;
}

float ATrainActor::GetTrackLength() const
{
	return TrackSpline ? TrackSpline->GetSplineLength() : 0.0f;
}
