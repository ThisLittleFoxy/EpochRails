// Реализация модульного персонажа
// Смотрите как мало кода - вся логика в компонентах!

#include "ModularCharacterExample.h"
#include "SprintComponent.h"
#include "JumpControlComponent.h"
#include "InteractionComponent.h"
// #include "CrouchControlComponent.h"  // Пример
// #include "StaminaComponent.h"        // Пример

AModularCharacterExample::AModularCharacterExample() {
  // Создаем компоненты
  SprintComponent = CreateDefaultSubobject<USprintComponent>(TEXT("SprintComponent"));
  JumpComponent = CreateDefaultSubobject<UJumpControlComponent>(TEXT("JumpComponent"));
  InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
  // CrouchComponent = CreateDefaultSubobject<UCrouchControlComponent>(TEXT("CrouchComponent"));
  // StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));

  // ВСЁ! Больше ничего не нужно в конструкторе.
  // Компоненты сами настроят персонажа в своем BeginPlay()
}

void AModularCharacterExample::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) {
  Super::SetupPlayerInputComponent(PlayerInputComponent);

  // Просто биндим инпуты к функциям-делегатам
  // Сами функции просто вызывают методы компонентов

  // Спринт
  PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AModularCharacterExample::OnSprintPressed);
  PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AModularCharacterExample::OnSprintReleased);

  // Прыжок
  PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AModularCharacterExample::OnJumpPressed);
  PlayerInputComponent->BindAction("Jump", IE_Released, this, &AModularCharacterExample::OnJumpReleased);

  // Взаимодействие
  PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AModularCharacterExample::OnInteractPressed);

  // Приседание
  PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AModularCharacterExample::OnCrouchPressed);
}

// ========== Input Handlers (просто делегаты в компоненты) ==========

void AModularCharacterExample::OnSprintPressed() {
  if (SprintComponent) {
    SprintComponent->StartSprint();  // ВСЯ логика в компоненте!
  }
}

void AModularCharacterExample::OnSprintReleased() {
  if (SprintComponent) {
    SprintComponent->StopSprint();  // ВСЯ логика в компоненте!
  }
}

void AModularCharacterExample::OnJumpPressed() {
  if (JumpComponent) {
    JumpComponent->TryJump();  // ВСЯ логика в компоненте!
  }
}

void AModularCharacterExample::OnJumpReleased() {
  if (JumpComponent) {
    JumpComponent->StopJump();  // ВСЯ логика в компоненте!
  }
}

void AModularCharacterExample::OnInteractPressed() {
  if (InteractionComponent) {
    InteractionComponent->TryInteract();  // ВСЯ логика в компоненте!
  }
}

void AModularCharacterExample::OnCrouchPressed() {
  // Пример toggle crouch
  if (bIsCrouched) {
    UnCrouch();
  } else {
    Crouch();
  }
}
