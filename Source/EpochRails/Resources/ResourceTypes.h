// ResourceTypes.h
// Defines resource types available in Epoch Rails
#pragma once

#include "CoreMinimal.h"
#include "ResourceTypes.generated.h"

/**
 * Available resource types in the game
 * Add new resources here as needed
 */
UENUM(BlueprintType)
enum class EResourceType : uint8
{
    None        UMETA(DisplayName = "None"),
    Metal       UMETA(DisplayName = "Metal"),
    Wood        UMETA(DisplayName = "Wood"),
    Books       UMETA(DisplayName = "Books")
};
