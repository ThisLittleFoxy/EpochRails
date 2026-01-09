// Пример компонента для управления прыжками
// Может добавлять двойной прыжок, dash в воздухе и т.д.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "JumpControlComponent.generated.h"

class ACharacter;

/**
 * Компонент для расширенного управления прыжками
 * Добавляет двойной прыжок, dash, wall jump и т.д.
 * Персонаж не знает о деталях - просто вызывает TryJump()
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EPOCHRAILS_API UJumpControlComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UJumpControlComponent();

  /** Попытка прыжка (обрабатывает двойной прыжок автоматически) */
  UFUNCTION(BlueprintCallable, Category = "Movement|Jump")
  void TryJump();

  /** Остановка прыжка */
  UFUNCTION(BlueprintCallable, Category = "Movement|Jump")
  void StopJump();

  /** Воздушный dash */
  UFUNCTION(BlueprintCallable, Category = "Movement|Jump")
  void PerformAirDash(FVector Direction);

  /** Сколько прыжков доступно */
  UFUNCTION(BlueprintPure, Category = "Movement|Jump")
  int32 GetRemainingJumps() const { return RemainingJumps; }

protected:
  virtual void BeginPlay() override;
  virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                             FActorComponentTickFunction *ThisTickFunction) override;

  // ========== Настройки ==========

  /** Разрешить двойной прыжок */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
  bool bAllowDoubleJump = true;

  /** Максимальное количество прыжков */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
  int32 MaxJumps = 2;

  /** Сила воздушного dash */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump|AirDash")
  float AirDashStrength = 1000.0f;

  /** Кулдаун между dash */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump|AirDash")
  float AirDashCooldown = 1.0f;

private:
  /** Оставшиеся прыжки */
  int32 RemainingJumps;

  /** Время последнего dash */
  float LastAirDashTime;

  /** Кешированный персонаж */
  UPROPERTY()
  ACharacter *OwnerCharacter;

  /** Обработчик приземления */
  UFUNCTION()
  void OnLanded(const FHitResult &Hit);
};
