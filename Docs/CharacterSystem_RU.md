# Character System (Система персонажа)

## Описание

Система маевражения игровым персонажом с поддержкой взаимодействия, оседания в транспорт и ех носкими.

## Основные классы

### ARailsPlayerCharacter
**Локация:** `Source/EpochRails/Character/RailsPlayerCharacter.h`

#### Функции для анимаций

```cpp
// Проверить, сидит ли персонаж
 UFUNCTION(BlueprintPure, Category = "Character Animation")
bool IsSeated() const;

// Проверить, на поезде ли
UFUNCTION(BlueprintPure, Category = "Character Animation")
bool IsOnMovingPlatform() const;

// Получить нормализованную скорость движения
UFUNCTION(BlueprintPure, Category = "Character Animation")
float GetNormalizedSpeed() const;

// Получить направление движения
UFUNCTION(BlueprintPure, Category = "Character Animation")
float GetMovementDirection() const;

// Получить скорость платформы для компенсации в анимации
UFUNCTION(BlueprintPure, Category = "Character Animation")
FVector GetPlatformVelocity() const;

// Отснять анимацию сидения
UFUNCTION(BlueprintImplementableEvent, Category = "Character Animation")
void PlaySitAnimation();

// Отснять анимацию вставания
UFUNCTION(BlueprintImplementableEvent, Category = "Character Animation")
void PlayStandUpAnimation();
```

## Версия документации
- **EpochRails Version:** 0.0.22
- **Last Updated:** 2025-12-04
