#include "Gameplay/ResourceInventory.h"
#include "Engine/World.h"

UResourceInventory::UResourceInventory() {
  PrimaryComponentTick.bCanEverTick = false;
}

void UResourceInventory::BeginPlay() {
  Super::BeginPlay();
  // ИСПРАВЛЕНИЕ: Убрали дублирование вызова
  if (Resources.Num() == 0) {
    InitializeInventory();
  }
}

void UResourceInventory::InitializeInventory() {
  InitializeResourceMap();
  UE_LOG(LogTemp, Log, TEXT("ResourceInventory initialized with capacity: %d"),
         StorageCapacity);
}

void UResourceInventory::InitializeResourceMap() {
  Resources.Empty();
  Resources.Add(EResourceType::Wood, 0);
  Resources.Add(EResourceType::Metal, 0);
  Resources.Add(EResourceType::Oxygen, 100); // Start with some oxygen
  Resources.Add(EResourceType::RareCrystals, 0);
  Resources.Add(EResourceType::Batteries, 0);
  Resources.Add(EResourceType::FoodSupplies, 50); // Start with some food
  Resources.Add(EResourceType::Microchips, 0);
}

bool UResourceInventory::AddResource(EResourceType ResourceType, int32 Amount) {
  if (Amount <= 0) {
    UE_LOG(LogTemp, Warning,
           TEXT("ResourceInventory: Attempted to add invalid amount: %d"),
           Amount);
    return false;
  }

  // Check if adding would exceed capacity
  if (GetTotalStorageUsed() + Amount > StorageCapacity) {
    const int32 AvailableSpace = StorageCapacity - GetTotalStorageUsed();
    UE_LOG(LogTemp, Warning,
           TEXT("ResourceInventory: Storage full! Cannot add %d resources. "
                "Available space: %d"),
           Amount, AvailableSpace);
    return false;
  }

  if (Resources.Contains(ResourceType)) {
    Resources[ResourceType] += Amount;
    OnResourceAdded.Broadcast(ResourceType, Amount);

    UE_LOG(
        LogTemp, Log,
        TEXT("ResourceInventory: Added %d of resource type %d. New total: %d"),
        Amount, static_cast<int32>(ResourceType), Resources[ResourceType]);
    return true;
  }

  UE_LOG(LogTemp, Error,
         TEXT("ResourceInventory: Resource type %d not found in inventory"),
         static_cast<int32>(ResourceType));
  return false;
}

bool UResourceInventory::RemoveResource(EResourceType ResourceType,
                                        int32 Amount) {
  if (Amount <= 0) {
    UE_LOG(LogTemp, Warning,
           TEXT("ResourceInventory: Attempted to remove invalid amount: %d"),
           Amount);
    return false;
  }

  if (!Resources.Contains(ResourceType)) {
    UE_LOG(LogTemp, Error,
           TEXT("ResourceInventory: Resource type %d not found in inventory"),
           static_cast<int32>(ResourceType));
    return false;
  }

  if (Resources[ResourceType] < Amount) {
    UE_LOG(LogTemp, Warning,
           TEXT("ResourceInventory: Not enough resources of type %d. Have %d, "
                "need %d"),
           static_cast<int32>(ResourceType), Resources[ResourceType], Amount);
    return false;
  }

  Resources[ResourceType] -= Amount;
  OnResourceRemoved.Broadcast(ResourceType, Amount);

  UE_LOG(
      LogTemp, Log,
      TEXT("ResourceInventory: Removed %d of resource type %d. Remaining: %d"),
      Amount, static_cast<int32>(ResourceType), Resources[ResourceType]);
  return true;
}

int32 UResourceInventory::GetResourceAmount(EResourceType ResourceType) const {
  if (Resources.Contains(ResourceType)) {
    return Resources[ResourceType];
  }

  UE_LOG(LogTemp, Warning,
         TEXT("ResourceInventory: Queried non-existent resource type %d"),
         static_cast<int32>(ResourceType));
  return 0;
}

bool UResourceInventory::HasEnoughResources(EResourceType ResourceType,
                                            int32 Amount) const {
  return GetResourceAmount(ResourceType) >= Amount;
}

int32 UResourceInventory::GetTotalStorageUsed() const {
  int32 Total = 0;
  for (const auto &Pair : Resources) {
    Total += Pair.Value;
  }
  return Total;
}

void UResourceInventory::IncreaseCapacity(int32 Amount) {
  if (Amount > 0) {
    StorageCapacity += Amount;
    UE_LOG(LogTemp, Log,
           TEXT("ResourceInventory: Storage capacity increased by %d to: %d"),
           Amount, StorageCapacity);
  } else {
    UE_LOG(LogTemp, Warning,
           TEXT("ResourceInventory: Attempted to increase capacity by invalid "
                "amount: %d"),
           Amount);
  }
}
