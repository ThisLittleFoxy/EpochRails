// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "RailsTrainSeat.generated.h"

class ARailsTrain;
class ARailsPlayerCharacter;
class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;

/**
 * Driver seat used to control the train.
 * Player can interact with this seat to start/stop driving the train.
 */
UCLASS()
class EPOCHRAILS_API ARailsTrainSeat : public AInteractableActor {
  GENERATED_BODY()

public:
  ARailsTrainSeat();

protected:
  virtual void BeginPlay() override;

public:
  // Called when player interacts with this seat
  virtual bool
  OnInteract_Implementation(ARailsPlayerCharacter *PlayerCharacter) override;

  // Optional: customize name and action text
  virtual FText GetInteractionName_Implementation() const override;
  virtual FText GetInteractionAction_Implementation() const override;

  // Optional: disallow interaction if there is no train or IMCs
  virtual bool CanInteract_Implementation(
      ARailsPlayerCharacter *PlayerCharacter) const override;

protected:
  /** Train that will be controlled from this seat */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train")
  ARailsTrain *ControlledTrain;

  /** Default input mapping context used for regular gameplay */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
  UInputMappingContext *DefaultInputMappingContext;

  /** Input mapping context used while controlling the train */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
  UInputMappingContext *TrainControlInputMappingContext;

  /** Priority for default IMC when added to the input subsystem */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
  int32 DefaultIMCPriority = 0;

  /** Priority for train control IMC when added to the input subsystem */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
  int32 TrainControlIMCPriority = 1;

  /** Whether player is currently seated and controlling the train */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Train")
  bool bIsPlayerSeated;

  /** Cached reference to the seated player */
  UPROPERTY()
  ARailsPlayerCharacter *SeatedPlayer;

  /** Point where player will be positioned when seated */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USceneComponent *SeatPoint;

protected:
  /** Put player into the seat and enable train control */
  void SeatPlayer(ARailsPlayerCharacter *PlayerCharacter);

  /** Remove player from the seat and restore default controls */
  void UnseatPlayer();

  /** Helper to switch input mapping contexts */
  void ApplyInputMappingContexts(ARailsPlayerCharacter *PlayerCharacter,
                                 bool bUseTrainControl);

  /** Helper to get Enhanced Input subsystem for a player */
  UEnhancedInputLocalPlayerSubsystem *
  GetInputSubsystem(ARailsPlayerCharacter *PlayerCharacter) const;
};
