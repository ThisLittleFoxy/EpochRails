// Пример персонажа с модульной архитектурой
// Персонаж НЕ содержит логику - только подключает компоненты

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ModularCharacterExample.generated.h"

class USprintComponent;
class UJumpControlComponent;
class UInteractionComponent;
class UCrouchControlComponent;
class UStaminaComponent;

/**
 * Модульный персонаж - сам НЕ содержит игровую логику
 * Вся логика в компонентах, которые можно менять/добавлять/удалять
 *
 * Преимущества:
 * - Легко добавить новую механику (просто новый компонент)
 * - Легко убрать механику (удалить компонент)
 * - Можно переиспользовать компоненты на других персонажах
 * - Логика изолирована - изменения в спринте не влияют на прыжки
 */
UCLASS()
class EPOCHRAILS_API AModularCharacterExample : public ACharacter {
  GENERATED_BODY()

public:
  AModularCharacterExample();

protected:
  virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

  // ========== Компоненты (вся логика здесь) ==========

  /** Компонент спринта */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USprintComponent *SprintComponent;

  /** Компонент прыжков */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UJumpControlComponent *JumpComponent;

  /** Компонент взаимодействия */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UInteractionComponent *InteractionComponent;

  /** Компонент приседания */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UCrouchControlComponent *CrouchComponent;

  /** Компонент стамины */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UStaminaComponent *StaminaComponent;

  // ========== Input Handlers (просто делегируют в компоненты) ==========

  void OnSprintPressed();
  void OnSprintReleased();
  void OnJumpPressed();
  void OnJumpReleased();
  void OnInteractPressed();
  void OnCrouchPressed();

public:
  // ========== Геттеры для внешних систем ==========

  UFUNCTION(BlueprintPure, Category = "Components")
  USprintComponent *GetSprintComponent() const { return SprintComponent; }

  UFUNCTION(BlueprintPure, Category = "Components")
  UJumpControlComponent *GetJumpComponent() const { return JumpComponent; }

  UFUNCTION(BlueprintPure, Category = "Components")
  UInteractionComponent *GetInteractionComponent() const { return InteractionComponent; }
};
