# Controllers System (Система контроллера)

## Описание

Контроллер игрока, обрабатывающий ввод и управление камерой с поддержкой Enhanced Input System.

### ARailsPlayerController
**Локация:** `Source/EpochRails/Controllers/RailsPlayerController.h`

#### Функции для анимаций

```cpp
// Получить поворот камеры
UFUNCTION(BlueprintPure, Category = "Controller Animation")
FRotator GetCameraRotation() const;

// Получить угол подъёма для Aim Offset
UFUNCTION(BlueprintPure, Category = "Controller Animation")
float GetAimPitch() const;

// Получить горизонтальный поворот
UFUNCTION(BlueprintPure, Category = "Controller Animation")
float GetAimYaw() const;

// Проверить, управляет ли персонаж поездом
UFUNCTION(BlueprintPure, Category = "Controller Animation")
bool IsControllingTrain() const;
```

## Версия документации
- **EpochRails Version:** 0.0.22
- **Last Updated:** 2025-12-04
