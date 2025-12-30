// TrainCheatManager.cpp

#include "TrainCheatManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Train/RailsTrain.h"

void UTrainCheatManager::AddWagons(int32 Count) {
  ARailsTrain *Train = FindNearestTrain();
  if (!Train) {
    UE_LOG(LogTemp, Warning, TEXT("No train found in level!"));
    return;
  }

  Train->AddWagons(Count);
  UE_LOG(LogTemp, Log, TEXT("Added %d wagon(s) to train"), Count);
}

void UTrainCheatManager::RemoveWagons(int32 Count) {
  ARailsTrain *Train = FindNearestTrain();
  if (!Train) {
    UE_LOG(LogTemp, Warning, TEXT("No train found in level!"));
    return;
  }

  Train->RemoveWagons(Count);
  UE_LOG(LogTemp, Log, TEXT("Removed %d wagon(s) from train"), Count);
}

void UTrainCheatManager::TrainInfo() {
  ARailsTrain *Train = FindNearestTrain();
  if (!Train) {
    UE_LOG(LogTemp, Warning, TEXT("No train found in level!"));
    return;
  }

  FString Info = FString::Printf(
      TEXT("=== TRAIN INFO ===\n") TEXT("Speed: %.1f km/h\n")
          TEXT("Wagons: %d\n") TEXT("Engine: %s\n") TEXT("Passengers: %d"),
      Train->GetCurrentSpeedKmh(),
      Train->GetWagonCount(), // <-- FIX HERE
      Train->IsEngineRunning() ? TEXT("ON") : TEXT("OFF"),
      Train->GetPassengers().Num());

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

  // Return first train (or implement distance check if needed)
  return Cast<ARailsTrain>(FoundTrains[0]);
}
