#include "ResourceInventory.h"
#include "Engine/World.h"

UResourceInventory::UResourceInventory() {
  PrimaryComponentTick.bCanEverTick = false;
}

void UResourceInventory::BeginPlay() {
  Super::BeginPlay();
  InitializeInventory();
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
    return false;
  }

  // Check if adding would exceed capacity
  if (GetTotalStorageUsed() + Amount > StorageCapacity) {
    UE_LOG(LogTemp, Warning, TEXT("Storage full! Cannot add %d resources"),
           Amount);
    return false;
  }

  if (Resources.Contains(ResourceType)) {
    Resources[ResourceType] += Amount;
    OnResourceAdded.Broadcast(ResourceType, Amount);
    return true;
  }

  return false;
}

bool UResourceInventory::RemoveResource(EResourceType ResourceType,
                                        int32 Amount) {
  if (Amount <= 0) {
    return false;
  }

  if (!Resources.Contains(ResourceType)) {
    return false;
  }

  if (Resources[ResourceType] < Amount) {
    UE_LOG(LogTemp, Warning, TEXT("Not enough %d resources. Have %d, need %d"),
           static_cast<int32>(ResourceType), Resources[ResourceType], Amount);
    return false;
  }

  Resources[ResourceType] -= Amount;
  OnResourceRemoved.Broadcast(ResourceType, Amount);
  return true;
}

int32 UResourceInventory::GetResourceAmount(EResourceType ResourceType) const {
  if (Resources.Contains(ResourceType)) {
    return Resources[ResourceType];
  }
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
    UE_LOG(LogTemp, Log, TEXT("Storage capacity increased to: %d"),
           StorageCapacity);
  }
}
