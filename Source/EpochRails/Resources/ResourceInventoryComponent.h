// ResourceInventoryComponent.h
// Component for managing resource inventory on train or character
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ResourceTypes.h"
#include "ResourceInventoryComponent.generated.h"

/**
 * Event broadcast when resource amount changes
 * @param ResourceType - Type of resource that changed
 * @param NewAmount - New amount of the resource
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceChanged, 
    EResourceType, ResourceType, int32, NewAmount);

/**
 * Component for storing and managing resources
 * Add this to ARailsTrain or ARailsPlayerCharacter for inventory functionality
 * 
 * Usage:
 * - AddResource(EResourceType::Metal, 10) - adds 10 metal
 * - RemoveResource(EResourceType::Wood, 5) - removes 5 wood
 * - GetResourceAmount(EResourceType::Books) - returns current books count
 * - HasEnoughResource(EResourceType::Metal, 50) - checks if has 50+ metal
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EPOCHRAILS_API UResourceInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UResourceInventoryComponent();

    /**
     * Add resources to inventory
     * @param Type - Resource type to add
     * @param Amount - Amount to add (must be positive)
     * @return true if successfully added, false if capacity reached or invalid params
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddResource(EResourceType Type, int32 Amount);

    /**
     * Remove resources from inventory
     * @param Type - Resource type to remove
     * @param Amount - Amount to remove (must be positive)
     * @return true if successfully removed, false if not enough resources
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveResource(EResourceType Type, int32 Amount);

    /**
     * Get current amount of specific resource
     * @param Type - Resource type to check
     * @return Current amount of the resource
     */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetResourceAmount(EResourceType Type) const;

    /**
     * Check if inventory has enough of specific resource
     * @param Type - Resource type to check
     * @param Amount - Required amount
     * @return true if has enough, false otherwise
     */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasEnoughResource(EResourceType Type, int32 Amount) const;

    /**
     * Get all resources as a map
     * @return Map of resource types to amounts
     */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    TMap<EResourceType, int32> GetAllResources() const { return Resources; }

    /**
     * Set capacity limit for specific resource
     * @param Type - Resource type
     * @param Capacity - Maximum amount (0 = unlimited)
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetResourceCapacity(EResourceType Type, int32 Capacity);

    /**
     * Get capacity limit for specific resource
     * @param Type - Resource type
     * @return Capacity limit (0 = unlimited)
     */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetResourceCapacity(EResourceType Type) const;

    /**
     * Get available space for specific resource
     * @param Type - Resource type
     * @return Remaining capacity (0 if full, -1 if unlimited)
     */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetAvailableSpace(EResourceType Type) const;

    /**
     * Check if resource storage is full
     * @param Type - Resource type
     * @return true if at capacity, false otherwise
     */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool IsResourceFull(EResourceType Type) const;

    /**
     * Clear all resources (reset to zero)
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ClearAllResources();

    /**
     * Event broadcast when any resource amount changes
     * Bind to this in Blueprint for UI updates
     */
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnResourceChanged OnResourceChanged;

protected:
    /**
     * Storage for resource amounts
     * Key: Resource type, Value: Current amount
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TMap<EResourceType, int32> Resources;

    /**
     * Capacity limits per resource type
     * Key: Resource type, Value: Maximum amount (0 = unlimited)
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TMap<EResourceType, int32> ResourceCapacities;

    /**
     * Default capacity for resources not explicitly set
     * 0 = unlimited (default behavior)
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 DefaultCapacity = 0;

private:
    /**
     * Get capacity for specific resource type
     * Returns DefaultCapacity if not explicitly set
     */
    int32 GetCapacity(EResourceType Type) const;
};
