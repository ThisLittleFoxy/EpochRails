#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RailsPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class ARailsPlayerCharacter;
class ABaseVehicle;
class UUserWidget;

/**
 * Контроллер игрока для EpochRails
 * Управляет HUD, режимами игры и взаимодействием
 */
UCLASS()
class EPOCHRAILS_API ARailsPlayerController : public APlayerController {
  GENERATED_BODY()

public:
  ARailsPlayerController();

protected:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaSeconds) override;
  virtual void SetupInputComponent() override;

private:
  // ========== Input Mapping Context ==========

  UPROPERTY(EditDefaultsOnly, Category = "Input")
  UInputMappingContext *DefaultMappingContext;

  UPROPERTY(EditAnywhere, Category = "Input")
  int32 MappingPriority = 0;

  // ========== HUD ==========

  UPROPERTY(EditDefaultsOnly, Category = "UI")
  TSubclassOf<UUserWidget> MainHUDClass;

  UPROPERTY(Transient)
  UUserWidget *MainHUD = nullptr;

  UPROPERTY(EditDefaultsOnly, Category = "UI")
  TSubclassOf<UUserWidget> TrainControlHUDClass;

  UPROPERTY(Transient)
  UUserWidget *TrainControlHUD = nullptr;

  // ========== Current State ==========

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State",
            meta = (AllowPrivateAccess = "true"))
  bool bInTrainControlMode = false;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State",
            meta = (AllowPrivateAccess = "true"))
  bool bInBuildMode = false;

public:
  // ========== HUD Management ==========

  UFUNCTION(BlueprintCallable, Category = "UI")
  void ShowMainHUD();

  UFUNCTION(BlueprintCallable, Category = "UI")
  void HideMainHUD();

  UFUNCTION(BlueprintCallable, Category = "UI")
  void ShowTrainControlHUD();

  UFUNCTION(BlueprintCallable, Category = "UI")
  void HideTrainControlHUD();

  UFUNCTION(BlueprintPure, Category = "UI")
  UUserWidget *GetMainHUD() const { return MainHUD; }

  UFUNCTION(BlueprintPure, Category = "UI")
  UUserWidget *GetTrainControlHUD() const { return TrainControlHUD; }

  // ========== Mode Management ==========

  UFUNCTION(BlueprintCallable, Category = "Mode")
  void EnterTrainControlMode(ABaseVehicle *Train);

  UFUNCTION(BlueprintCallable, Category = "Mode")
  void ExitTrainControlMode();

  UFUNCTION(BlueprintCallable, Category = "Mode")
  void EnterBuildMode();

  UFUNCTION(BlueprintCallable, Category = "Mode")
  void ExitBuildMode();

  UFUNCTION(BlueprintPure, Category = "Mode")
  bool IsInTrainControlMode() const { return bInTrainControlMode; }

  UFUNCTION(BlueprintPure, Category = "Mode")
  bool IsInBuildMode() const { return bInBuildMode; }

  // ========== Getters ==========

  UFUNCTION(BlueprintPure, Category = "Character")
  ARailsPlayerCharacter *GetRailsCharacter() const;

private:
  void AddDefaultIMC();
};
