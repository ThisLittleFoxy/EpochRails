#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ResourceInventory.generated.h"

// Resource types enum
UENUM(BlueprintType)
enum class EResourceType : uint8 {
  Wood UMETA(DisplayName = "Wood"),
  Metal UMETA(DisplayName = "Metal"),
  Oxygen UMETA(DisplayName = "Oxygen"),
  RareCrystals UMETA(DisplayName = "Rare Crystals"),
  Batteries UMETA(DisplayName = "Batteries"),
  FoodSupplies UMETA(DisplayName = "Food Supplies"),
  Microchips UMETA(DisplayName = "Microchips")
};

// Delegate for resource changes
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnResourceChanged, EResourceType, int32);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EPOCHRAILS_API UResourceInventory : public UActorComponent {
  GENERATED_BODY()

public:
  UResourceInventory();

  virtual void BeginPlay() override;

  // Initialize inventory
  UFUNCTION(BlueprintCallable, Category = "Resources")
  void InitializeInventory();

  // Add resource to inventory
  UFUNCTION(BlueprintCallable, Category = "Resources")
  bool AddResource(EResourceType ResourceType, int32 Amount);

  // Remove resource from inventory
  UFUNCTION(BlueprintCallable, Category = "Resources")
  bool RemoveResource(EResourceType ResourceType, int32 Amount);

  // Get amount of specific resource
  UFUNCTION(BlueprintPure, Category = "Resources")
  int32 GetResourceAmount(EResourceType ResourceType) const;

  // Check if we have enough resources
  UFUNCTION(BlueprintPure, Category = "Resources")
  bool HasEnoughResources(EResourceType ResourceType, int32 Amount) const;

  // Get total storage used
  UFUNCTION(BlueprintPure, Category = "Resources")
  int32 GetTotalStorageUsed() const;

  // Get storage capacity
  UFUNCTION(BlueprintPure, Category = "Resources")
  int32 GetStorageCapacity() const { return StorageCapacity; }

  // Increase storage capacity (from upgrades)
  UFUNCTION(BlueprintCallable, Category = "Resources")
  void IncreaseCapacity(int32 Amount);

public:
  // Events
  FOnResourceChanged OnResourceAdded;
  FOnResourceChanged OnResourceRemoved;

private:
  // Resource storage - maps resource type to amount
  UPROPERTY(VisibleAnywhere, Category = "Resources")
  TMap<EResourceType, int32> Resources;

  // Storage capacity
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources",
            meta = (AllowPrivateAccess = true))
  int32 StorageCapacity = 500;

  // Initialize resource map with zeros
  void InitializeResourceMap();
};
