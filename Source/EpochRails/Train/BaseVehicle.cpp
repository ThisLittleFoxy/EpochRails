#include "Train/BaseVehicle.h"

#include "Gameplay/ResourceInventory.h"
#include "Train/LocomotionComponent.h"
#include "Train/RailSplineActor.h"

#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ABaseVehicle::ABaseVehicle() {
  PrimaryActorTick.bCanEverTick = true;

  // Создаем визуальные компоненты
  CreateVisualComponents();

  // Существующие компоненты
  ResourceInventory =
      CreateDefaultSubobject<UResourceInventory>(TEXT("ResourceInventory"));
  LocomotionComponent =
      CreateDefaultSubobject<ULocomotionComponent>(TEXT("LocomotionComponent"));
}

void ABaseVehicle::CreateVisualComponents() {
  // Vehicle Mesh (основной меш)
  VehicleMesh =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleMesh"));
  RootComponent = VehicleMesh;

  // Cabin Mesh (кабина, только для локомотива)
  CabinMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CabinMesh"));
  CabinMesh->SetupAttachment(VehicleMesh);
  CabinMesh->SetRelativeLocation(FVector(100.0f, 0.0f, 150.0f));

  // Coupler Points (точки сцепки)
  FrontCouplerPoint =
      CreateDefaultSubobject<USceneComponent>(TEXT("FrontCouplerPoint"));
  FrontCouplerPoint->SetupAttachment(RootComponent);
  FrontCouplerPoint->SetRelativeLocation(FVector(300.0f, 0.0f, 0.0f));

  RearCouplerPoint =
      CreateDefaultSubobject<USceneComponent>(TEXT("RearCouplerPoint"));
  RearCouplerPoint->SetupAttachment(RootComponent);
  RearCouplerPoint->SetRelativeLocation(FVector(-300.0f, 0.0f, 0.0f));

  // Headlights (фары)
  HeadlightLeft =
      CreateDefaultSubobject<USpotLightComponent>(TEXT("HeadlightLeft"));
  HeadlightLeft->SetupAttachment(VehicleMesh);
  HeadlightLeft->SetRelativeLocation(FVector(300.0f, -80.0f, 50.0f));
  HeadlightLeft->SetIntensity(10000.0f);
  HeadlightLeft->SetAttenuationRadius(3000.0f);
  HeadlightLeft->SetOuterConeAngle(45.0f);
  HeadlightLeft->SetVisibility(false);

  HeadlightRight =
      CreateDefaultSubobject<USpotLightComponent>(TEXT("HeadlightRight"));
  HeadlightRight->SetupAttachment(VehicleMesh);
  HeadlightRight->SetRelativeLocation(FVector(300.0f, 80.0f, 50.0f));
  HeadlightRight->SetIntensity(10000.0f);
  HeadlightRight->SetAttenuationRadius(3000.0f);
  HeadlightRight->SetOuterConeAngle(45.0f);
  HeadlightRight->SetVisibility(false);

  // Загружаем базовый меш (куб)
  static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(
      TEXT("/Engine/BasicShapes/Cube"));

  if (CubeMeshAsset.Succeeded()) {
    VehicleMesh->SetStaticMesh(CubeMeshAsset.Object);
    CabinMesh->SetStaticMesh(CubeMeshAsset.Object);
  } else {
    UE_LOG(LogTemp, Error, TEXT("BaseVehicle: Failed to load cube mesh"));
  }
}

void ABaseVehicle::BeginPlay() {
  Super::BeginPlay();

  // Инициализация здоровья
  CurrentHealth = MaxHealth;
  UE_LOG(LogTemp, Log, TEXT("BaseVehicle '%s' spawned with health: %f"),
         *GetName(), CurrentHealth);

  // Проверка компонентов
  if (!ResourceInventory) {
    UE_LOG(LogTemp, Error, TEXT("BaseVehicle '%s': ResourceInventory is null!"),
           *GetName());
  } else {
    ResourceInventory->InitializeInventory();
  }

  if (!LocomotionComponent) {
    UE_LOG(LogTemp, Error,
           TEXT("BaseVehicle '%s': LocomotionComponent is null!"), *GetName());
  }

  // Настройка визуализации
  SetupMeshForVehicleType();

  // Инициализация сетки строительства
  if (bAllowBuilding) {
    InitializeBuildGrid();
  }

  // Инициализация двигателя
  CurrentFuel = MaxFuel;
  bEngineRunning = bIsLocomotive;

  // Отключаем фары, если это не локомотив
  if (!bIsLocomotive || !bHasHeadlights) {
    if (HeadlightLeft)
      HeadlightLeft->SetVisibility(false);
    if (HeadlightRight)
      HeadlightRight->SetVisibility(false);
  }

  // Инициализация рельсов
  InitializeRailSpline();
}

void ABaseVehicle::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  // Обновление текущей скорости
  if (LocomotionComponent) {
    CurrentSpeed = LocomotionComponent->GetCurrentSpeed();
  }

  // Обновления для локомотива
  if (bIsLocomotive) {
    UpdateThrottle(DeltaTime);
    UpdateFuelConsumption(DeltaTime);
    UpdateWagonPositions();
  }
}

void ABaseVehicle::SetupPlayerInputComponent(
    UInputComponent *PlayerInputComponent) {
  Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// ========== Настройка визуализации ==========

void ABaseVehicle::SetupMeshForVehicleType() {
  if (!VehicleMesh)
    return;

  UMaterialInterface *BaseMaterial = LoadObject<UMaterialInterface>(
      nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));

  if (!BaseMaterial)
    return;

  switch (VehicleType) {
  case EVehicleType::Locomotive:
    VehicleMesh->SetWorldScale3D(FVector(6.0f, 3.0f, 3.0f));

    if (CabinMesh) {
      CabinMesh->SetWorldScale3D(FVector(2.0f, 2.8f, 1.5f));
      CabinMesh->SetVisibility(true);

      UMaterialInstanceDynamic *CabinMat =
          UMaterialInstanceDynamic::Create(BaseMaterial, this);
      CabinMat->SetVectorParameterValue(FName("Color"),
                                        FLinearColor(0.8f, 0.8f, 0.2f));
      CabinMesh->SetMaterial(0, CabinMat);
    }

    {
      UMaterialInstanceDynamic *VehicleMat =
          UMaterialInstanceDynamic::Create(BaseMaterial, this);
      VehicleMat->SetVectorParameterValue(FName("Color"),
                                          FLinearColor(0.2f, 0.2f, 0.8f));
      VehicleMesh->SetMaterial(0, VehicleMat);
    }
    break;

  case EVehicleType::CargoWagon:
    VehicleMesh->SetWorldScale3D(FVector(5.0f, 3.0f, 3.0f));

    if (CabinMesh)
      CabinMesh->SetVisibility(false);

    {
      UMaterialInstanceDynamic *VehicleMat =
          UMaterialInstanceDynamic::Create(BaseMaterial, this);
      VehicleMat->SetVectorParameterValue(FName("Color"),
                                          FLinearColor(0.6f, 0.3f, 0.1f));
      VehicleMesh->SetMaterial(0, VehicleMat);
    }
    break;

  case EVehicleType::PassengerWagon:
    VehicleMesh->SetWorldScale3D(FVector(5.0f, 3.0f, 3.0f));

    if (CabinMesh)
      CabinMesh->SetVisibility(false);

    {
      UMaterialInstanceDynamic *VehicleMat =
          UMaterialInstanceDynamic::Create(BaseMaterial, this);
      VehicleMat->SetVectorParameterValue(FName("Color"),
                                          FLinearColor(0.2f, 0.8f, 0.2f));
      VehicleMesh->SetMaterial(0, VehicleMat);
    }
    break;

  default:
    VehicleMesh->SetWorldScale3D(FVector(5.0f, 3.0f, 3.0f));
    if (CabinMesh)
      CabinMesh->SetVisibility(false);
    break;
  }

  UE_LOG(LogTemp, Log, TEXT("BaseVehicle '%s': Mesh setup for type %d"),
         *GetName(), static_cast<int32>(VehicleType));
}

// ========== Health System ==========

float ABaseVehicle::TakeDamage(float DamageAmount,
                               FDamageEvent const &DamageEvent,
                               AController *EventInstigator,
                               AActor *DamageCauser) {
  if (bIsDestroyed || CurrentHealth <= 0.0f) {
    return 0.0f;
  }

  const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent,
                                               EventInstigator, DamageCauser);

  if (ActualDamage <= 0.0f) {
    return 0.0f;
  }

  CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);
  UE_LOG(LogTemp, Log,
         TEXT("BaseVehicle '%s' took damage: %f, current health: %f"),
         *GetName(), ActualDamage, CurrentHealth);

  if (CurrentHealth <= 0.0f && !bIsDestroyed) {
    bIsDestroyed = true;
    UE_LOG(LogTemp, Warning, TEXT("BaseVehicle '%s' destroyed"), *GetName());
    HandleDestruction();
  }

  return ActualDamage;
}

void ABaseVehicle::HandleDestruction_Implementation() {
  if (LocomotionComponent) {
    LocomotionComponent->SetThrottle(0.0f);
    LocomotionComponent->ApplyBrakes(1000.0f);
  }

  OnVehicleDestroyed.Broadcast(this);

  FTimerHandle DestroyTimer;
  GetWorldTimerManager().SetTimer(
      DestroyTimer,
      [this]() {
        if (IsValid(this)) {
          Destroy();
        }
      },
      5.0f, false);
}

float ABaseVehicle::GetHealthPercent() const {
  return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

// ========== Movement Control ==========

void ABaseVehicle::SetThrottle(float Value) {
  if (LocomotionComponent && !bIsDestroyed) {
    if (bIsLocomotive && bEngineRunning) {
      LocomotionComponent->SetThrottle(Value);
    } else if (!bIsLocomotive) {
      // Прицепные вагоны не контролируют throttle напрямую
    }
  }
}

void ABaseVehicle::SetRailSpline(ARailSplineActor *NewRailSpline) {
  if (!NewRailSpline) {
    UE_LOG(LogTemp, Warning,
           TEXT("BaseVehicle '%s': Attempted to assign null rail spline"),
           *GetName());
    return;
  }

  AssignedRailSpline = NewRailSpline;
  InitializeRailSpline();
}

// ========== Engine Control ==========

void ABaseVehicle::StartEngine() {
  if (!bIsLocomotive) {
    UE_LOG(LogTemp, Warning,
           TEXT("Cannot start engine: '%s' is not a locomotive"), *GetName());
    return;
  }

  if (bRequiresFuel && CurrentFuel <= 0.0f) {
    UE_LOG(LogTemp, Warning, TEXT("Cannot start engine on '%s': No fuel"),
           *GetName());
    return;
  }

  bEngineRunning = true;
  UE_LOG(LogTemp, Log, TEXT("Engine started on '%s'"), *GetName());
}

void ABaseVehicle::StopEngine() {
  bEngineRunning = false;
  TargetThrottle = 0.0f;
  SetThrottle(0.0f);
  UE_LOG(LogTemp, Log, TEXT("Engine stopped on '%s'"), *GetName());
}

void ABaseVehicle::ToggleEngine() {
  if (bEngineRunning) {
    StopEngine();
  } else {
    StartEngine();
  }
}

void ABaseVehicle::SetTargetThrottle(float Value) {
  if (!bIsLocomotive || !bEngineRunning) {
    TargetThrottle = 0.0f;
    return;
  }

  if (bRequiresFuel && CurrentFuel <= 0.0f) {
    TargetThrottle = 0.0f;
    return;
  }

  TargetThrottle = FMath::Clamp(Value, -1.0f, 1.0f);
}

void ABaseVehicle::IncreaseThrottle(float Amount) {
  SetTargetThrottle(TargetThrottle + Amount);
}

void ABaseVehicle::DecreaseThrottle(float Amount) {
  SetTargetThrottle(TargetThrottle - Amount);
}

void ABaseVehicle::EmergencyBrake() {
  TargetThrottle = 0.0f;
  SetThrottle(0.0f);

  if (LocomotionComponent) {
    LocomotionComponent->ApplyBrakes(5000.0f);
  }

  UE_LOG(LogTemp, Warning, TEXT("EMERGENCY BRAKE on '%s'"), *GetName());
}

void ABaseVehicle::UpdateThrottle(float DeltaTime) {
  if (!bIsLocomotive || !bEngineRunning) {
    return;
  }

  const float CurrentThrottle =
      LocomotionComponent ? LocomotionComponent->GetCurrentSpeed() / MaxSpeed
                          : 0.0f;

  const float NewThrottle = FMath::FInterpTo(CurrentThrottle, TargetThrottle,
                                             DeltaTime, ThrottleChangeRate);

  const float TrainMass = GetTotalTrainMass();
  const float MassPenalty =
      FMath::Clamp(1.0f - (TrainMass * 0.0001f), 0.5f, 1.0f);
  const float EffectiveThrottle = NewThrottle * EngineEfficiency * MassPenalty;

  SetThrottle(EffectiveThrottle);

  OnThrottleChanged.Broadcast(EffectiveThrottle, 1.0f);
}

void ABaseVehicle::UpdateFuelConsumption(float DeltaTime) {
  if (!bRequiresFuel || !bEngineRunning || !bIsLocomotive) {
    return;
  }

  const float AbsThrottle = FMath::Abs(TargetThrottle);
  const float FuelUsed = FuelConsumptionRate * AbsThrottle * DeltaTime;

  CurrentFuel = FMath::Max(0.0f, CurrentFuel - FuelUsed);

  if (CurrentFuel <= 0.0f && bEngineRunning) {
    StopEngine();
    UE_LOG(LogTemp, Warning, TEXT("Engine stopped on '%s': Out of fuel"),
           *GetName());
  }
}

// ========== Train Composition ==========

bool ABaseVehicle::AttachWagon(ABaseVehicle *Wagon) {
  if (!Wagon || !bIsLocomotive) {
    return false;
  }

  if (AttachedWagons.Num() >= MaxAttachedWagons) {
    UE_LOG(LogTemp, Warning,
           TEXT("Cannot attach wagon to '%s': Max capacity %d reached"),
           *GetName(), MaxAttachedWagons);
    return false;
  }

  AttachedWagons.Add(Wagon);
  Wagon->SetLeadingVehicle(this);
  Wagon->bIsLocomotive = false;

  UE_LOG(LogTemp, Log, TEXT("Wagon '%s' attached to '%s'. Total: %d"),
         *Wagon->GetName(), *GetName(), AttachedWagons.Num());

  return true;
}

bool ABaseVehicle::DetachWagon(ABaseVehicle *Wagon) {
  if (!Wagon) {
    return false;
  }

  const int32 Removed = AttachedWagons.Remove(Wagon);
  if (Removed > 0) {
    Wagon->SetLeadingVehicle(nullptr);
    UE_LOG(LogTemp, Log, TEXT("Wagon '%s' detached from '%s'"),
           *Wagon->GetName(), *GetName());
    return true;
  }

  return false;
}

void ABaseVehicle::DetachAllWagons() {
  for (ABaseVehicle *Wagon : AttachedWagons) {
    if (Wagon) {
      Wagon->SetLeadingVehicle(nullptr);
    }
  }

  AttachedWagons.Empty();
  UE_LOG(LogTemp, Log, TEXT("All wagons detached from '%s'"), *GetName());
}

float ABaseVehicle::GetTotalTrainMass() const {
  float TotalMass = 1000.0f;

  for (const ABaseVehicle *Wagon : AttachedWagons) {
    if (Wagon) {
      TotalMass += 500.0f;
      TotalMass += Wagon->PlacedObjects.Num() * 50.0f;
    }
  }

  return TotalMass;
}

void ABaseVehicle::SetLeadingVehicle(ABaseVehicle *Locomotive) {
  LeadingVehicle = Locomotive;
}

void ABaseVehicle::UpdateWagonPositions() {
  if (!bIsLocomotive || AttachedWagons.Num() == 0 || !LocomotionComponent) {
    return;
  }

  USplineComponent *RailSpline =
      AssignedRailSpline ? AssignedRailSpline->RailSpline : nullptr;

  if (!RailSpline) {
    return;
  }

  const float LocomotiveDistance =
      LocomotionComponent->GetDistanceAlongSpline();
  float CurrentDistance = LocomotiveDistance;

  for (ABaseVehicle *Wagon : AttachedWagons) {
    if (!Wagon)
      continue;

    CurrentDistance -= (Wagon->WagonLength + CouplerGap);
    CurrentDistance = FMath::Max(0.0f, CurrentDistance);

    const FVector WagonLocation = RailSpline->GetLocationAtDistanceAlongSpline(
        CurrentDistance, ESplineCoordinateSpace::World);

    const FRotator WagonRotation = RailSpline->GetRotationAtDistanceAlongSpline(
        CurrentDistance, ESplineCoordinateSpace::World);

    Wagon->SetActorLocationAndRotation(WagonLocation, WagonRotation);
  }
}

// ========== Lights ==========

void ABaseVehicle::ToggleHeadlights() { SetHeadlights(!bHeadlightsEnabled); }

void ABaseVehicle::SetHeadlights(bool bEnabled) {
  if (!bHasHeadlights)
    return;

  bHeadlightsEnabled = bEnabled;

  if (HeadlightLeft)
    HeadlightLeft->SetVisibility(bEnabled);
  if (HeadlightRight)
    HeadlightRight->SetVisibility(bEnabled);

  UE_LOG(LogTemp, Log, TEXT("Headlights on '%s': %s"), *GetName(),
         bEnabled ? TEXT("ON") : TEXT("OFF"));
}

// ========== Building System ==========

void ABaseVehicle::InitializeBuildGrid() {
  BuildGrid.Empty();

  for (int32 Y = 0; Y < GridSizeY; Y++) {
    for (int32 X = 0; X < GridSizeX; X++) {
      BuildGrid.Add(FBuildGridCell(X, Y));
    }
  }

  UE_LOG(LogTemp, Log, TEXT("Build grid initialized on '%s': %dx%d = %d cells"),
         *GetName(), GridSizeX, GridSizeY, BuildGrid.Num());
}

FVector ABaseVehicle::GetGridCellWorldLocation(int32 X, int32 Y) const {
  if (!IsGridCellValid(X, Y) || !VehicleMesh) {
    return FVector::ZeroVector;
  }

  const FVector VehicleCenter = VehicleMesh->GetComponentLocation();

  const float OffsetX = (X - GridSizeX / 2.0f) * CellSize;
  const float OffsetY = (Y - GridSizeY / 2.0f) * CellSize;

  const FVector LocalOffset(OffsetX, OffsetY, 200.0f);
  const FVector WorldOffset = GetActorRotation().RotateVector(LocalOffset);

  return VehicleCenter + WorldOffset;
}

bool ABaseVehicle::IsGridCellValid(int32 X, int32 Y) const {
  return X >= 0 && X < GridSizeX && Y >= 0 && Y < GridSizeY;
}

bool ABaseVehicle::IsGridCellOccupied(int32 X, int32 Y) const {
  const int32 Index = GetGridIndex(X, Y);
  return Index >= 0 && Index < BuildGrid.Num() && BuildGrid[Index].bOccupied;
}

bool ABaseVehicle::PlaceObjectAtCell(int32 X, int32 Y,
                                     TSubclassOf<AActor> ObjectClass) {
  if (!bAllowBuilding || !IsGridCellValid(X, Y) || IsGridCellOccupied(X, Y) ||
      !ObjectClass) {
    return false;
  }

  const FVector SpawnLocation = GetGridCellWorldLocation(X, Y);
  const FRotator SpawnRotation = GetActorRotation();

  AActor *NewObject =
      GetWorld()->SpawnActor<AActor>(ObjectClass, SpawnLocation, SpawnRotation);

  if (NewObject) {
    const int32 Index = GetGridIndex(X, Y);
    BuildGrid[Index].bOccupied = true;
    BuildGrid[Index].PlacedObject = NewObject;
    PlacedObjects.Add(NewObject);

    NewObject->AttachToActor(this,
                             FAttachmentTransformRules::KeepWorldTransform);

    UE_LOG(LogTemp, Log, TEXT("Object placed at (%d,%d) on '%s'"), X, Y,
           *GetName());
    return true;
  }

  return false;
}

bool ABaseVehicle::RemoveObjectAtCell(int32 X, int32 Y) {
  const int32 Index = GetGridIndex(X, Y);
  if (Index < 0 || Index >= BuildGrid.Num() || !BuildGrid[Index].bOccupied) {
    return false;
  }

  AActor *Object = BuildGrid[Index].PlacedObject;
  if (Object) {
    PlacedObjects.Remove(Object);
    Object->Destroy();
  }

  BuildGrid[Index].bOccupied = false;
  BuildGrid[Index].PlacedObject = nullptr;

  UE_LOG(LogTemp, Log, TEXT("Object removed from (%d,%d) on '%s'"), X, Y,
         *GetName());
  return true;
}

void ABaseVehicle::ClearAllObjects() {
  for (AActor *Object : PlacedObjects) {
    if (Object) {
      Object->Destroy();
    }
  }

  PlacedObjects.Empty();

  for (FBuildGridCell &Cell : BuildGrid) {
    Cell.bOccupied = false;
    Cell.PlacedObject = nullptr;
  }

  UE_LOG(LogTemp, Log, TEXT("All objects cleared from '%s'"), *GetName());
}

int32 ABaseVehicle::GetGridIndex(int32 X, int32 Y) const {
  if (!IsGridCellValid(X, Y)) {
    return -1;
  }
  return Y * GridSizeX + X;
}

// ========== Rail System ==========

void ABaseVehicle::InitializeRailSpline() {
  if (!LocomotionComponent) {
    UE_LOG(LogTemp, Error,
           TEXT("BaseVehicle '%s': LocomotionComponent is null, cannot "
                "initialize rail spline"),
           *GetName());
    return;
  }

  ARailSplineActor *RailToUse = nullptr;

  if (AssignedRailSpline && AssignedRailSpline->RailSpline) {
    RailToUse = AssignedRailSpline;
    UE_LOG(LogTemp, Log,
           TEXT("BaseVehicle '%s': Using manually assigned rail '%s'"),
           *GetName(), *RailToUse->GetName());
  } else if (bAutoFindNearestRail) {
    RailToUse = FindNearestRail();
    if (RailToUse) {
      UE_LOG(LogTemp, Log,
             TEXT("BaseVehicle '%s': Auto-found nearest rail '%s'"), *GetName(),
             *RailToUse->GetName());
    }
  }

  if (RailToUse && RailToUse->RailSpline) {
    LocomotionComponent->SetRailSpline(RailToUse->RailSpline);

    UE_LOG(LogTemp, Log,
           TEXT("BaseVehicle '%s': Rail spline successfully assigned and "
                "positioned on '%s'"),
           *GetName(), *RailToUse->GetName());
  } else {
    UE_LOG(LogTemp, Error,
           TEXT("BaseVehicle '%s': Failed to initialize rail spline! "
                "AssignedRailSpline=%s, AutoFind=%s"),
           *GetName(), AssignedRailSpline ? TEXT("Set") : TEXT("None"),
           bAutoFindNearestRail ? TEXT("Enabled") : TEXT("Disabled"));
  }
}

ARailSplineActor *ABaseVehicle::FindNearestRail() {
  UWorld *World = GetWorld();
  if (!World) {
    return nullptr;
  }

  const FVector VehicleLocation = GetActorLocation();
  ARailSplineActor *NearestRail = nullptr;
  float MinDistance = RailSearchRadius;

  TArray<AActor *> FoundRails;
  UGameplayStatics::GetAllActorsOfClass(World, ARailSplineActor::StaticClass(),
                                        FoundRails);

  for (AActor *Actor : FoundRails) {
    ARailSplineActor *Rail = Cast<ARailSplineActor>(Actor);
    if (!Rail || !Rail->RailSpline) {
      continue;
    }

    const float InputKey =
        Rail->RailSpline->FindInputKeyClosestToWorldLocation(VehicleLocation);
    const FVector ClosestPoint = Rail->RailSpline->GetLocationAtSplineInputKey(
        InputKey, ESplineCoordinateSpace::World);
    const float Distance = FVector::Dist(VehicleLocation, ClosestPoint);

    if (Distance < MinDistance) {
      MinDistance = Distance;
      NearestRail = Rail;
    }

#if !UE_BUILD_SHIPPING
    DrawDebugLine(World, VehicleLocation, ClosestPoint,
                  Distance < RailSearchRadius ? FColor::Green : FColor::Red,
                  false, 2.0f, 0, 2.0f);
#endif
  }

  if (NearestRail) {
    UE_LOG(LogTemp, Log,
           TEXT("BaseVehicle '%s': Found nearest rail '%s' at distance %f"),
           *GetName(), *NearestRail->GetName(), MinDistance);
  } else {
    UE_LOG(LogTemp, Warning,
           TEXT("BaseVehicle '%s': No rail found within search radius %f"),
           *GetName(), RailSearchRadius);
  }

  return NearestRail;
}
