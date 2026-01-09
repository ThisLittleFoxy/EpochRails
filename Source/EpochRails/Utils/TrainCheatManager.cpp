// TrainCheatManager.cpp

#include "TrainCheatManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Train/RailsTrain.h"
#include "Train/RailsWagon.h"

void UTrainCheatManager::AddWagons(int32 Count) {
  ARailsTrain *Train = FindNearestTrain();
  if (!Train) {
    UE_LOG(LogTemp, Warning, TEXT("No train found in level!"));
    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("No train found!"));
    return;
  }

  int32 Added = 0;
  for (int32 i = 0; i < Count; ++i) {
    ARailsWagon *Wagon = Train->AddWagon();
    if (Wagon) {
      ++Added;
    }
  }

  FString Msg = FString::Printf(TEXT("Added %d wagon(s) (total: %d)"), Added, Train->GetWagonCount());
  UE_LOG(LogTemp, Log, TEXT("%s"), *Msg);
  GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, Msg);
}

void UTrainCheatManager::RemoveWagons(int32 Count) {
  ARailsTrain *Train = FindNearestTrain();
  if (!Train) {
    UE_LOG(LogTemp, Warning, TEXT("No train found in level!"));
    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("No train found!"));
    return;
  }

  int32 Removed = 0;
  for (int32 i = 0; i < Count; ++i) {
    if (Train->RemoveLastWagon()) {
      ++Removed;
    }
  }

  FString Msg = FString::Printf(TEXT("Removed %d wagon(s) (remaining: %d)"), Removed, Train->GetWagonCount());
  UE_LOG(LogTemp, Log, TEXT("%s"), *Msg);
  GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, Msg);
}

void UTrainCheatManager::TrainInfo() {
  ARailsTrain *Train = FindNearestTrain();
  if (!Train) {
    UE_LOG(LogTemp, Warning, TEXT("No train found in level!"));
    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("No train found!"));
    return;
  }

  // Count total structures on all wagons
  int32 TotalStructures = 0;
  TArray<ARailsWagon *> Wagons = Train->GetAttachedWagons();
  for (ARailsWagon *Wagon : Wagons) {
    if (Wagon) {
      TotalStructures += Wagon->GetPlacedStructures().Num();
    }
  }

  FString Info = FString::Printf(
      TEXT("=== TRAIN INFO ===\nSpeed: %.1f\nStopped: %s\nWagons: %d\nStructures: %d"),
      Train->GetSpeed(),
      Train->IsStopped() ? TEXT("YES") : TEXT("NO"),
      Train->GetWagonCount(),
      TotalStructures);

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
