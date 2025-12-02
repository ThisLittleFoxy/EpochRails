// InteractionTypes.h
// Common types and enums for interaction system

#pragma once

#include "CoreMinimal.h"
#include "InteractionTypes.generated.h"

/**
 * Types of interactions available in the game
 */
UENUM(BlueprintType)
enum class EInteractionType : uint8 {
  None UMETA(DisplayName = "None"),
  Seat UMETA(DisplayName = "Seat"),
  DriverSeat UMETA(DisplayName = "Driver Seat"),
  Door UMETA(DisplayName = "Door"),
  Lever UMETA(DisplayName = "Lever/Switch"),
  Container UMETA(DisplayName = "Container"),
  ItemPickup UMETA(DisplayName = "Item Pickup"),
  Custom UMETA(DisplayName = "Custom (Blueprint)")
};

/**
 * Door animation types
 */
UENUM(BlueprintType)
enum class EDoorAnimationType : uint8 {
  Hinge UMETA(DisplayName = "Hinge (Rotate)"),
  Slide UMETA(DisplayName = "Slide (Translate)")
};

/**
 * Interaction state for animations
 */
UENUM(BlueprintType)
enum class EInteractionState : uint8 {
  None UMETA(DisplayName = "None"),
  Interacting UMETA(DisplayName = "Interacting"),
  Sitting UMETA(DisplayName = "Sitting"),
  Driving UMETA(DisplayName = "Driving Train"),
  Opening UMETA(DisplayName = "Opening"),
  Closing UMETA(DisplayName = "Closing")
};

/**
 * Data structure for interaction settings
 */
USTRUCT(BlueprintType)
struct FInteractionSettings {
  GENERATED_BODY()

  /** Text shown to player when focused on this interactable */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  FText InteractionPrompt = FText::FromString("Press E to interact");

  /** Maximum distance for interaction trigger */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction",
            meta = (ClampMin = "50.0", ClampMax = "500.0"))
  float InteractionRadius = 150.0f;

  /** Use additional raycast check for precision */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  bool bUseRaycastCheck = true;

  /** Maximum raycast distance from camera */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction",
            meta = (ClampMin = "100.0", ClampMax = "1000.0",
                    EditCondition = "bUseRaycastCheck"))
  float RaycastDistance = 300.0f;

  /** Can interact while train is moving */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  bool bCanInteractDuringTrainMovement = true;

  /** Highlight color for custom depth outline */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Visual")
  FLinearColor HighlightColor = FLinearColor::Yellow;

  /** Custom depth stencil value (1-255) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Visual",
            meta = (ClampMin = "1", ClampMax = "255"))
  uint8 CustomDepthStencilValue = 1;

  FInteractionSettings()
      : InteractionPrompt(FText::FromString("Press E to interact")),
        InteractionRadius(150.0f), bUseRaycastCheck(true),
        RaycastDistance(300.0f), bCanInteractDuringTrainMovement(true),
        HighlightColor(FLinearColor::Yellow), CustomDepthStencilValue(1) {}
};

/**
 * Animation settings for interactions
 */
USTRUCT(BlueprintType)
struct FInteractionAnimationSettings {
  GENERATED_BODY()

  /** Animation to play when interacting */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
  TSoftObjectPtr<UAnimMontage> InteractionMontage;

  /** Blend in time for animation */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation",
            meta = (ClampMin = "0.0", ClampMax = "2.0"))
  float BlendInTime = 0.2f;

  /** Blend out time for animation */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation",
            meta = (ClampMin = "0.0", ClampMax = "2.0"))
  float BlendOutTime = 0.2f;

  /** Play rate multiplier */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation",
            meta = (ClampMin = "0.1", ClampMax = "5.0"))
  float PlayRate = 1.0f;

  FInteractionAnimationSettings()
      : BlendInTime(0.2f), BlendOutTime(0.2f), PlayRate(1.0f) {}
};
