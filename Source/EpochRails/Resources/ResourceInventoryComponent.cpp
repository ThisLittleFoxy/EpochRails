// ResourceInventoryComponent.cpp
#include "ResourceInventoryComponent.h"

UResourceInventoryComponent::UResourceInventoryComponent()
{
    // Disable tick - this component doesn't need to update every frame
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize all resource types with zero amount
    Resources.Add(EResourceType::Metal, 0);
    Resources.Add(EResourceType::Wood, 0);
    Resources.Add(EResourceType::Books, 0);
}

bool UResourceInventoryComponent::AddResource(EResourceType Type, int32 Amount)
{
    // Validate input parameters
    if (Type == EResourceType::None || Amount <= 0)
    {
        return false;
    }

    int32 CurrentAmount = GetResourceAmount(Type);
    int32 Capacity = GetCapacity(Type);
    
    // Check capacity limit (0 means unlimited)
    if (Capacity > 0)
    {
        int32 AvailableSpace = Capacity - CurrentAmount;
        
        // Already at capacity
        if (AvailableSpace <= 0)
        {
            return false;
        }
        
        // Clamp amount to available space
        if (Amount > AvailableSpace)
        {
            Amount = AvailableSpace;
        }
    }

    // Add the resource
    int32 NewAmount = CurrentAmount + Amount;
    Resources.Add(Type, NewAmount);
    
    // Broadcast event for UI updates
    OnResourceChanged.Broadcast(Type, NewAmount);
    
    return true;
}

bool UResourceInventoryComponent::RemoveResource(EResourceType Type, int32 Amount)
{
    // Validate input parameters
    if (Type == EResourceType::None || Amount <= 0)
    {
        return false;
    }

    int32 CurrentAmount = GetResourceAmount(Type);
    
    // Check if we have enough resources
    if (CurrentAmount < Amount)
    {
        return false;
    }

    // Remove the resource
    int32 NewAmount = CurrentAmount - Amount;
    Resources.Add(Type, NewAmount);
    
    // Broadcast event for UI updates
    OnResourceChanged.Broadcast(Type, NewAmount);
    
    return true;
}

int32 UResourceInventoryComponent::GetResourceAmount(EResourceType Type) const
{
    // Find resource in map
    if (const int32* Found = Resources.Find(Type))
    {
        return *Found;
    }
    
    // Resource not found, return zero
    return 0;
}

bool UResourceInventoryComponent::HasEnoughResource(EResourceType Type, int32 Amount) const
{
    return GetResourceAmount(Type) >= Amount;
}

void UResourceInventoryComponent::SetResourceCapacity(EResourceType Type, int32 Capacity)
{
    // Capacity must be non-negative (0 = unlimited)
    if (Capacity < 0)
    {
        Capacity = 0;
    }
    
    ResourceCapacities.Add(Type, Capacity);
}

int32 UResourceInventoryComponent::GetResourceCapacity(EResourceType Type) const
{
    return GetCapacity(Type);
}

int32 UResourceInventoryComponent::GetAvailableSpace(EResourceType Type) const
{
    int32 Capacity = GetCapacity(Type);
    
    // Unlimited capacity
    if (Capacity == 0)
    {
        return -1;
    }
    
    int32 CurrentAmount = GetResourceAmount(Type);
    int32 Available = Capacity - CurrentAmount;
    
    // Clamp to zero minimum
    return FMath::Max(0, Available);
}

bool UResourceInventoryComponent::IsResourceFull(EResourceType Type) const
{
    int32 Capacity = GetCapacity(Type);
    
    // Unlimited capacity = never full
    if (Capacity == 0)
    {
        return false;
    }
    
    return GetResourceAmount(Type) >= Capacity;
}

void UResourceInventoryComponent::ClearAllResources()
{
    // Reset all resources to zero
    for (auto& Pair : Resources)
    {
        Pair.Value = 0;
        OnResourceChanged.Broadcast(Pair.Key, 0);
    }
}

int32 UResourceInventoryComponent::GetCapacity(EResourceType Type) const
{
    // Check if capacity is explicitly set for this resource type
    if (const int32* Found = ResourceCapacities.Find(Type))
    {
        return *Found;
    }
    
    // Return default capacity if not explicitly set
    return DefaultCapacity;
}
