#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "TrainControlPanel.generated.h"

class UStaticMeshComponent;
class UWidgetComponent;
class ABaseVehicle;

/**
 * Панель управления поездом - интерактивный объект внутри локомотива
 * При взаимодействии открывает UI управления поездом
 */
UCLASS()
class EPOCHRAILS_API ATrainControlPanel : public AActor,
                                          public IInteractableInterface {
  GENERATED_BODY()

public:
  ATrainControlPanel();

protected:
  virtual void BeginPlay() override;

private:
  // ========== Visual Components ==========

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
            meta = (AllowPrivateAccess = "true"))
  UStaticMeshComponent *PanelMesh;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
            meta = (AllowPrivateAccess = "true"))
  UStaticMeshComponent *ScreenMesh;

  // Widget для 3D UI (опционально)
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
            meta = (AllowPrivateAccess = "true"))
  UWidgetComponent *ControlWidget;

  // ========== Train Reference ==========

  UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Train",
            meta = (AllowPrivateAccess = "true"))
  ABaseVehicle *OwningTrain = nullptr;

  // ========== Interaction Settings ==========

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction",
            meta = (AllowPrivateAccess = "true"))
  FText InteractionPrompt = FText::FromString(TEXT("Open Control Panel [E]"));

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction",
            meta = (AllowPrivateAccess = "true"))
  bool bIsHighlighted = false;

public:
  // ========== IInteractableInterface Implementation ==========

  virtual void OnLookAtStart_Implementation() override;
  virtual void OnLookAtEnd_Implementation() override;
  virtual void
  OnInteract_Implementation(AController *InstigatorController) override;
  virtual FText GetInteractionText_Implementation() const override;
  virtual bool
  CanInteract_Implementation(AController *InstigatorController) const override;

  // ========== Setup ==========

  UFUNCTION(BlueprintCallable, Category = "Train")
  void SetOwningTrain(ABaseVehicle *Train);

  UFUNCTION(BlueprintPure, Category = "Train")
  ABaseVehicle *GetOwningTrain() const { return OwningTrain; }

private:
  void CreateDefaultMesh();
  void UpdateHighlight(bool bHighlight);
};
