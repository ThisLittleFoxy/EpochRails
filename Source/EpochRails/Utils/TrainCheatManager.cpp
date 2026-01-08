// TrainCheatManager.cpp

#include "TrainCheatManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Train/RailsTrain.h"

void UTrainCheatManager::AddWagons(int32 Count) {
  // Wagons temporarily removed
  UE_LOG(LogTemp, Warning, TEXT("Wagons not implemented yet"));
}

void UTrainCheatManager::RemoveWagons(int32 Count) {
  // Wagons temporarily removed
  UE_LOG(LogTemp, Warning, TEXT("Wagons not implemented yet"));
}

void UTrainCheatManager::TrainInfo() {
  ARailsTrain *Train = FindNearestTrain();
  if (!Train) {
    UE_LOG(LogTemp, Warning, TEXT("No train found in level!"));
    return;
  }

  FString Info = FString::Printf(
      TEXT("=== TRAIN INFO ===\nSpeed: %.1f\nStopped: %s"),
      Train->GetSpeed(),
      Train->IsStopped() ? TEXT("YES") : TEXT("NO"));

  UE_LOG(LogTemp, Log, TEXT("%s"), *Info);
  GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, Info);
}

ARailsTrain *UTrainCheatManager::FindNearestTrain() const {
  if (!GetWorld())
    return nullptr;

  TArray<AActor *> FoundTrains;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARailsTrain::StaticClass(),
                                        FoundTrains);

  if (FoundTrains.Num() == 0)
    return nullptr;

  return Cast<ARailsTrain>(FoundTrains[0]);
}
