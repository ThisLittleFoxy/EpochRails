// Полная реализация компонентов с взаимодействием
// Смотрите как компоненты находят друг друга и подписываются на события

#include "ComponentInteractionExample.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// ========== STAMINA COMPONENT ==========

UStaminaComponent::UStaminaComponent() {
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UStaminaComponent::BeginPlay() {
  Super::BeginPlay();

  // Инициализация
  CurrentStamina = MaxStamina;
  TimeSinceLastConsume = 0.0f;
  bWasDepleted = false;
}

void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  // Регенерация стамины
  if (CurrentStamina < MaxStamina) {
    TimeSinceLastConsume += DeltaTime;

    // Начинаем реген после задержки
    if (TimeSinceLastConsume >= RegenDelay) {
      float OldStamina = CurrentStamina;
      CurrentStamina = FMath::Min(CurrentStamina + StaminaRegenRate * DeltaTime, MaxStamina);

      // Если стамина восстановилась после истощения
      if (bWasDepleted && CurrentStamina > 0.0f) {
        bWasDepleted = false;
        OnStaminaRecovered.Broadcast();
      }

      // Уведомляем об изменении
      if (CurrentStamina != OldStamina) {
        OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
      }
    }
  }
}

bool UStaminaComponent::TryConsumeStamina(float Amount) {
  if (CurrentStamina < Amount) {
    return false;  // Недостаточно стамины
  }

  CurrentStamina = FMath::Max(CurrentStamina - Amount, 0.0f);
  TimeSinceLastConsume = 0.0f;  // Сброс таймера регена

  // Уведомляем об изменении
  OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);

  // Проверяем истощение
  if (CurrentStamina <= 0.0f && !bWasDepleted) {
    bWasDepleted = true;
    OnStaminaDepleted.Broadcast();
    UE_LOG(LogTemp, Warning, TEXT("Stamina depleted!"));
  }

  return true;
}

void UStaminaComponent::RestoreStamina(float Amount) {
  bool WasDepleted = (CurrentStamina <= 0.0f);

  CurrentStamina = FMath::Min(CurrentStamina + Amount, MaxStamina);

  OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);

  if (WasDepleted && CurrentStamina > 0.0f) {
    bWasDepleted = false;
    OnStaminaRecovered.Broadcast();
  }
}

// ========== SPRINT COMPONENT (с интеграцией стамины) ==========

UStaminaAwareSprintComponent::UStaminaAwareSprintComponent() {
  PrimaryComponentTick.bCanEverTick = true;
}

void UStaminaAwareSprintComponent::BeginPlay() {
  Super::BeginPlay();

  // Получаем ссылку на персонажа
  OwnerCharacter = Cast<ACharacter>(GetOwner());
  if (OwnerCharacter) {
    MovementComponent = OwnerCharacter->GetCharacterMovement();

    // Устанавливаем начальную скорость
    if (MovementComponent) {
      MovementComponent->MaxWalkSpeed = WalkSpeed;
    }
  }

  // ВАЖНО: Ищем компонент стамины (если есть)
  StaminaComponent = GetOwner()->FindComponentByClass<UStaminaComponent>();

  // Если компонент стамины есть - подписываемся на события
  if (StaminaComponent) {
    // Подписываемся на истощение стамины
    StaminaComponent->OnStaminaDepleted.AddDynamic(this, &UStaminaAwareSprintComponent::OnStaminaDepleted);
    UE_LOG(LogTemp, Log, TEXT("Sprint: Found and subscribed to StaminaComponent"));
  } else {
    UE_LOG(LogTemp, Warning, TEXT("Sprint: No StaminaComponent found - sprinting without stamina limit"));
  }
}

void UStaminaAwareSprintComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                 FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  // Если спринтим - тратим стамину
  if (bIsSprinting && StaminaComponent) {
    float StaminaCost = StaminaDrainRate * DeltaTime;

    // Пытаемся потратить стамину
    if (!StaminaComponent->TryConsumeStamina(StaminaCost)) {
      // Не хватило стамины - останавливаем спринт
      StopSprint();
    }
  }
}

void UStaminaAwareSprintComponent::StartSprint() {
  if (!OwnerCharacter || !MovementComponent) {
    return;
  }

  // Если есть компонент стамины - проверяем минимальное количество
  if (StaminaComponent) {
    if (!StaminaComponent->HasEnoughStamina(MinStaminaToSprint)) {
      UE_LOG(LogTemp, Warning, TEXT("Not enough stamina to sprint"));
      return;
    }
  }

  // Проверка на полет
  if (MovementComponent->IsFalling()) {
    return;
  }

  bIsSprinting = true;
  MovementComponent->MaxWalkSpeed = SprintSpeed;

  UE_LOG(LogTemp, Log, TEXT("Sprint started"));
}

void UStaminaAwareSprintComponent::StopSprint() {
  if (!MovementComponent) {
    return;
  }

  bIsSprinting = false;
  MovementComponent->MaxWalkSpeed = WalkSpeed;

  UE_LOG(LogTemp, Log, TEXT("Sprint stopped"));
}

void UStaminaAwareSprintComponent::OnStaminaDepleted() {
  // Автоматически останавливаем спринт при истощении стамины
  UE_LOG(LogTemp, Warning, TEXT("Sprint: Stamina depleted - stopping sprint"));
  StopSprint();
}

// ========== JUMP COMPONENT (с интеграцией стамины) ==========

UStaminaAwareJumpComponent::UStaminaAwareJumpComponent() {
  PrimaryComponentTick.bCanEverTick = false;
}

void UStaminaAwareJumpComponent::BeginPlay() {
  Super::BeginPlay();

  OwnerCharacter = Cast<ACharacter>(GetOwner());
  if (OwnerCharacter) {
    // Подписываемся на приземление
    OwnerCharacter->LandedDelegate.AddDynamic(this, &UStaminaAwareJumpComponent::OnLanded);
  }

  // Ищем компонент стамины
  StaminaComponent = GetOwner()->FindComponentByClass<UStaminaComponent>();

  if (StaminaComponent) {
    UE_LOG(LogTemp, Log, TEXT("Jump: Found StaminaComponent"));
  } else {
    UE_LOG(LogTemp, Warning, TEXT("Jump: No StaminaComponent found - jumping without stamina cost"));
  }

  RemainingJumps = MaxJumps;
}

void UStaminaAwareJumpComponent::TryJump() {
  if (!OwnerCharacter) {
    return;
  }

  // Определяем стоимость прыжка
  float StaminaCost = (RemainingJumps == MaxJumps) ? JumpStaminaCost : DoubleJumpStaminaCost;

  // Если есть стамина - проверяем
  if (StaminaComponent) {
    if (!StaminaComponent->HasEnoughStamina(StaminaCost)) {
      UE_LOG(LogTemp, Warning, TEXT("Not enough stamina to jump"));
      return;
    }
  }

  // Проверяем наличие прыжков
  if (RemainingJumps <= 0) {
    return;
  }

  // Тратим стамину (если есть компонент)
  if (StaminaComponent) {
    StaminaComponent->TryConsumeStamina(StaminaCost);
  }

  // Прыгаем
  OwnerCharacter->Jump();
  RemainingJumps--;

  UE_LOG(LogTemp, Log, TEXT("Jump performed. Remaining: %d"), RemainingJumps);
}

void UStaminaAwareJumpComponent::StopJump() {
  if (OwnerCharacter) {
    OwnerCharacter->StopJumping();
  }
}

void UStaminaAwareJumpComponent::OnLanded(const FHitResult &Hit) {
  // Восстанавливаем прыжки при приземлении
  RemainingJumps = MaxJumps;
  UE_LOG(LogTemp, Log, TEXT("Landed. Jumps reset to %d"), MaxJumps);
}

// ========== UI COMPONENT ==========

UCharacterUIComponent::UCharacterUIComponent() {
  PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterUIComponent::BeginPlay() {
  Super::BeginPlay();

  // Ищем компонент стамины
  UStaminaComponent *StaminaComponent = GetOwner()->FindComponentByClass<UStaminaComponent>();

  // Подписываемся на события стамины
  if (StaminaComponent) {
    StaminaComponent->OnStaminaChanged.AddDynamic(this, &UCharacterUIComponent::OnStaminaChanged);
    StaminaComponent->OnStaminaDepleted.AddDynamic(this, &UCharacterUIComponent::OnStaminaDepleted);

    UE_LOG(LogTemp, Log, TEXT("UI: Subscribed to stamina events"));

    // Инициализируем UI текущими значениями
    OnStaminaChanged(StaminaComponent->GetCurrentStamina(), StaminaComponent->GetMaxStamina());
  }
}

void UCharacterUIComponent::OnStaminaChanged(float Current, float Max) {
  // Здесь обновляем UI виджет
  float Percentage = (Max > 0.0f) ? (Current / Max) * 100.0f : 0.0f;
  UE_LOG(LogTemp, Log, TEXT("UI: Stamina changed - %.1f / %.1f (%.0f%%)"), Current, Max, Percentage);

  // В реальном проекте:
  // if (StaminaBarWidget) {
  //   StaminaBarWidget->SetPercent(Current / Max);
  // }
}

void UCharacterUIComponent::OnStaminaDepleted() {
  // Показываем предупреждение в UI
  UE_LOG(LogTemp, Warning, TEXT("UI: Stamina depleted - show warning"));

  // В реальном проекте:
  // if (StaminaWarningWidget) {
  //   StaminaWarningWidget->SetVisibility(ESlateVisibility::Visible);
  // }
}
