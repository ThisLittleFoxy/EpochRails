// Пример взаимодействия между компонентами через события
// Компоненты НЕ знают друг о друге напрямую - общаются через делегаты

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "ComponentInteractionExample.generated.h"

// ========== ПРИМЕР 1: Stamina Component (энергия) ==========

// Делегат для оповещения о изменении стамины
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, CurrentStamina, float, MaxStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaDepleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaRecovered);

/**
 * Компонент управления стаминой
 * Другие компоненты могут подписаться на события и реагировать
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EPOCHRAILS_API UStaminaComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UStaminaComponent();

  virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                             FActorComponentTickFunction *ThisTickFunction) override;

  /** Попытка потратить стамину */
  UFUNCTION(BlueprintCallable, Category = "Stamina")
  bool TryConsumeStamina(float Amount);

  /** Восстановить стамину */
  UFUNCTION(BlueprintCallable, Category = "Stamina")
  void RestoreStamina(float Amount);

  /** Есть ли достаточно стамины */
  UFUNCTION(BlueprintPure, Category = "Stamina")
  bool HasEnoughStamina(float Amount) const { return CurrentStamina >= Amount; }

  /** Текущая стамина */
  UFUNCTION(BlueprintPure, Category = "Stamina")
  float GetCurrentStamina() const { return CurrentStamina; }

  /** Максимальная стамина */
  UFUNCTION(BlueprintPure, Category = "Stamina")
  float GetMaxStamina() const { return MaxStamina; }

  // ========== События (другие компоненты подписываются) ==========

  /** Событие изменения стамины */
  UPROPERTY(BlueprintAssignable, Category = "Stamina")
  FOnStaminaChanged OnStaminaChanged;

  /** Событие истощения стамины (достигла 0) */
  UPROPERTY(BlueprintAssignable, Category = "Stamina")
  FOnStaminaDepleted OnStaminaDepleted;

  /** Событие восстановления (была 0, стала > 0) */
  UPROPERTY(BlueprintAssignable, Category = "Stamina")
  FOnStaminaRecovered OnStaminaRecovered;

protected:
  virtual void BeginPlay() override;

  // ========== Настройки ==========

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
  float MaxStamina = 100.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
  float StaminaRegenRate = 10.0f;  // в секунду

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
  float RegenDelay = 1.0f;  // задержка перед реген

private:
  float CurrentStamina;
  float TimeSinceLastConsume;
  bool bWasDepleted;
};

// ========== ПРИМЕР 2: Sprint Component с интеграцией стамины ==========

/**
 * Компонент спринта, использующий систему стамины
 * НЕ хранит ссылку на StaminaComponent напрямую
 * Находит его динамически при необходимости
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EPOCHRAILS_API UStaminaAwareSprintComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UStaminaAwareSprintComponent();

  virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                             FActorComponentTickFunction *ThisTickFunction) override;

  UFUNCTION(BlueprintCallable, Category = "Sprint")
  void StartSprint();

  UFUNCTION(BlueprintCallable, Category = "Sprint")
  void StopSprint();

  UFUNCTION(BlueprintPure, Category = "Sprint")
  bool IsSprinting() const { return bIsSprinting; }

protected:
  virtual void BeginPlay() override;

  /** Обработчик истощения стамины (автоматически остановить спринт) */
  UFUNCTION()
  void OnStaminaDepleted();

  // ========== Настройки ==========

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
  float WalkSpeed = 500.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
  float SprintSpeed = 800.0f;

  /** Расход стамины в секунду при спринте */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
  float StaminaDrainRate = 20.0f;

  /** Минимальная стамина для начала спринта */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
  float MinStaminaToSprint = 10.0f;

private:
  bool bIsSprinting;

  UPROPERTY()
  class ACharacter *OwnerCharacter;

  UPROPERTY()
  class UCharacterMovementComponent *MovementComponent;

  /** Ссылка на компонент стамины (находится динамически) */
  UPROPERTY()
  UStaminaComponent *StaminaComponent;
};

// ========== ПРИМЕР 3: Jump Component с интеграцией стамины ==========

/**
 * Компонент прыжков, использующий стамину
 * Прыжок стоит стамину
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EPOCHRAILS_API UStaminaAwareJumpComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UStaminaAwareJumpComponent();

  UFUNCTION(BlueprintCallable, Category = "Jump")
  void TryJump();

  UFUNCTION(BlueprintCallable, Category = "Jump")
  void StopJump();

protected:
  virtual void BeginPlay() override;

  UFUNCTION()
  void OnLanded(const FHitResult &Hit);

  // ========== Настройки ==========

  /** Стоимость обычного прыжка */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
  float JumpStaminaCost = 15.0f;

  /** Стоимость двойного прыжка */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
  float DoubleJumpStaminaCost = 25.0f;

  /** Разрешить двойной прыжок */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
  bool bAllowDoubleJump = true;

  /** Максимум прыжков */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
  int32 MaxJumps = 2;

private:
  int32 RemainingJumps;

  UPROPERTY()
  class ACharacter *OwnerCharacter;

  UPROPERTY()
  UStaminaComponent *StaminaComponent;
};

// ========== ПРИМЕР 4: UI Component (слушает события) ==========

/**
 * Компонент для отображения UI
 * Подписывается на события других компонентов
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EPOCHRAILS_API UCharacterUIComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UCharacterUIComponent();

protected:
  virtual void BeginPlay() override;

  /** Обработчик изменения стамины */
  UFUNCTION()
  void OnStaminaChanged(float Current, float Max);

  /** Обработчик истощения */
  UFUNCTION()
  void OnStaminaDepleted();

  // Здесь можно добавить логику обновления UI виджетов
};

// ========== КАК ЭТО РАБОТАЕТ ==========
/*

1. StaminaComponent - независимый, сам управляет стаминой
   - Имеет события: OnStaminaChanged, OnStaminaDepleted
   - НЕ знает о других компонентах

2. SprintComponent - подписывается на StaminaDepleted
   - При старте спринта проверяет: есть ли StaminaComponent на владельце?
   - Если есть - проверяет стамину, если нет - спринтит без стамины
   - Подписывается на OnStaminaDepleted → автоматически останавливает спринт

3. JumpComponent - потребляет стамину при прыжке
   - При прыжке проверяет: есть ли StaminaComponent?
   - Если есть - вызывает TryConsumeStamina()
   - Если нет - прыгает без стамины

4. UIComponent - отображает стамину
   - Подписывается на OnStaminaChanged
   - Обновляет UI при изменениях

ВАЖНО: Компоненты НЕ зависят друг от друга жестко!
- SprintComponent работает БЕЗ StaminaComponent (просто без ограничения)
- StaminaComponent работает БЕЗ SprintComponent
- Можно добавить/удалить любой компонент без поломок

*/
