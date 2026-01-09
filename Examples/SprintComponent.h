// Пример компонента для управления спринтом
// Вся логика спринта ВЫНЕСЕНА из класса персонажа

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "SprintComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;

/**
 * Компонент для управления спринтом персонажа
 * Подключается к любому персонажу через AddComponent
 * Персонаж просто вызывает StartSprint/StopSprint - всю логику делает компонент
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EPOCHRAILS_API USprintComponent : public UActorComponent {
  GENERATED_BODY()

public:
  USprintComponent();

  /** Начать спринт */
  UFUNCTION(BlueprintCallable, Category = "Movement|Sprint")
  void StartSprint();

  /** Остановить спринт */
  UFUNCTION(BlueprintCallable, Category = "Movement|Sprint")
  void StopSprint();

  /** Проверка, спринтит ли персонаж */
  UFUNCTION(BlueprintPure, Category = "Movement|Sprint")
  bool IsSprinting() const { return bIsSprinting; }

protected:
  virtual void BeginPlay() override;

  // ========== Настройки ==========

  /** Обычная скорость ходьбы */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
  float WalkSpeed = 500.0f;

  /** Скорость спринта */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
  float SprintSpeed = 800.0f;

  /** Можно ли спринтить в воздухе */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
  bool bAllowSprintInAir = false;

  /** Расход стамины в секунду (если нужно) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint|Stamina")
  float StaminaDrainRate = 0.0f;

private:
  /** Флаг спринта */
  bool bIsSprinting = false;

  /** Кешированная ссылка на персонажа */
  UPROPERTY()
  ACharacter *OwnerCharacter;

  /** Кешированный CharacterMovement */
  UPROPERTY()
  UCharacterMovementComponent *MovementComponent;
};
