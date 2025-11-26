#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractableInterface : public UInterface {
  GENERATED_BODY()
};

/**
 * Интерфейс для всех объектов, с которыми можно взаимодействовать
 */
class EPOCHRAILS_API IInteractableInterface {
  GENERATED_BODY()

public:
  // Вызывается при наведении взгляда на объект
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
  void OnLookAtStart();
  virtual void OnLookAtStart_Implementation() {}

  // Вызывается при прекращении наведения
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
  void OnLookAtEnd();
  virtual void OnLookAtEnd_Implementation() {}

  // Вызывается при взаимодействии (клавиша E)
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
  void OnInteract(AController *InstigatorController);
  virtual void OnInteract_Implementation(AController *InstigatorController) {}

  // Получить название объекта для UI
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
  FText GetInteractionText() const;
  virtual FText GetInteractionText_Implementation() const {
    return FText::FromString(TEXT("Interact"));
  }

  // Можно ли взаимодействовать в данный момент
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
  bool CanInteract(AController *InstigatorController) const;
  virtual bool
  CanInteract_Implementation(AController *InstigatorController) const {
    return true;
  }
};
