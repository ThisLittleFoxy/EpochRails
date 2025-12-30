
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "TrainCheatManager.generated.h"

/**
 * Cheat manager for train debug commands
 */
UCLASS()
class EPOCHRAILS_API UTrainCheatManager : public UCheatManager {
  GENERATED_BODY()

public:
  /** Add wagons to nearest train */
  UFUNCTION(Exec, Category = "Train")
  void AddWagons(int32 Count = 1);

  /** Remove wagons from nearest train */
  UFUNCTION(Exec, Category = "Train")
  void RemoveWagons(int32 Count = 1);

  /** Get current train info */
  UFUNCTION(Exec, Category = "Train")
  void TrainInfo();

private:
  /** Find nearest train to player */
  class ARailsTrain *FindNearestTrain() const;
};
