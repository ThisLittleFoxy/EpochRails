// Реализация SprintComponent
// ВСЯ логика спринта здесь - персонаж просто вызывает функции

#include "SprintComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

USprintComponent::USprintComponent() {
  PrimaryComponentTick.bCanEverTick = false;
}

void USprintComponent::BeginPlay() {
  Super::BeginPlay();

  // Получаем владельца-персонажа
  OwnerCharacter = Cast<ACharacter>(GetOwner());
  if (OwnerCharacter) {
    MovementComponent = OwnerCharacter->GetCharacterMovement();

    // Устанавливаем начальную скорость
    if (MovementComponent) {
      MovementComponent->MaxWalkSpeed = WalkSpeed;
    }
  }
}

void USprintComponent::StartSprint() {
  // Проверяем, можем ли спринтить
  if (!OwnerCharacter || !MovementComponent) {
    return;
  }

  // Проверка на полет (если нельзя спринтить в воздухе)
  if (!bAllowSprintInAir && MovementComponent->IsFalling()) {
    return;
  }

  // Включаем спринт
  bIsSprinting = true;
  MovementComponent->MaxWalkSpeed = SprintSpeed;

  UE_LOG(LogTemp, Log, TEXT("Sprint started"));
}

void USprintComponent::StopSprint() {
  if (!MovementComponent) {
    return;
  }

  // Выключаем спринт
  bIsSprinting = false;
  MovementComponent->MaxWalkSpeed = WalkSpeed;

  UE_LOG(LogTemp, Log, TEXT("Sprint stopped"));
}
