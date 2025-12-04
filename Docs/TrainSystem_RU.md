# Train System (Система поездов)

## Описание

Система управления поездами, движущимися по сплайн-путям. Поезда могут перевозить персонажей, имеют физику движения с ускорением и торможением. Система не модифицирует сам класс персонажа — использует систему attachment для посадки.

## Основные компоненты

### ARailsTrain
Базовый класс поезда, движущегося по рельсовым путям.

**Локация:** `Source/EpochRails/Train/RailsTrain.h`

#### Компоненты сцены
- `TrainRoot` — корневой компонент сцены
- `TrainBodyMesh` — основной меш корпуса поезда
- `PlatformMesh` — платформа, на которой стоят персонажи
- `BoardingZone` — BoxComponent для обнаружения посадки пассажиров

#### Параметры движения
- `MaxSpeed` (float) — максимальная скорость поезда (см/с)
- `AccelerationRate` (float) — ускорение (см/с²)
- `DecelerationRate` (float) — торможение (см/с²)
- `CurrentSpeed` (float) — текущая скорость
- `CurrentDistance` (float) — текущая позиция на сплайне (см)
- `bLoopPath` (bool) — зацикливается ли путь в конце
- `bAutoStart` (bool) — автостарт движения при начале игры

#### Состояние поезда
- `TrainState` (ETrainState) — текущее состояние (Stopped/Moving/Accelerating/Decelerating)
- `PassengersOnBoard` (TArray<ACharacter*>) — список пассажиров на борту
- `CurrentThrottle` (float) — позиция дросселя (-1.0 до 1.0)
- `CurrentBrake` (float) — позиция тормоза (0.0 до 1.0)
- `SplinePathRef` (ARailsSplinePath*) — ссылка на сплайн-путь

#### Публичное API для управления

```cpp
// Запуск движения поезда
UFUNCTION(BlueprintCallable, Category = "Train Control")
void StartTrain();

// Остановка поезда
UFUNCTION(BlueprintCallable, Category = "Train Control")
void StopTrain();

// Установка скорости (будет ограничена MaxSpeed)
UFUNCTION(BlueprintCallable, Category = "Train Control")
void SetSpeed(float NewSpeed);

// Получить текущую скорость
UFUNCTION(BlueprintPure, Category = "Train Control")
float GetCurrentSpeed() const;

// Получить состояние поезда
UFUNCTION(BlueprintPure, Category = "Train Control")
ETrainState GetTrainState() const;

// Проверить находится ли персонаж на поезде
UFUNCTION(BlueprintPure, Category = "Train Control")
bool IsCharacterOnTrain(ACharacter* Character) const;

// Получить всех пассажиров
UFUNCTION(BlueprintPure, Category = "Train Control")
TArray<ACharacter*> GetPassengers() const;

// Применить дроссель (-1.0 до 1.0)
UFUNCTION(BlueprintCallable, Category = "Train Control")
void ApplyThrottle(float ThrottleValue);

// Применить тормоз (0.0 до 1.0)
UFUNCTION(BlueprintCallable, Category = "Train Control")
void ApplyBrake(float BrakeValue);

// Получить позицию дросселя
UFUNCTION(BlueprintPure, Category = "Train Control")
float GetThrottlePosition() const;
```

## Функции для использования в анимациях

Эти функции предназначены для связи с Animation Blueprint и обновления анимаций в реальном времени:

### Получение нормализованной скорости
```cpp
UFUNCTION(BlueprintPure, Category = "Train Animation")
float GetNormalizedSpeed() const
{
    return FMath::Clamp(CurrentSpeed / MaxSpeed, 0.0f, 1.0f);
}
```
**Применение:** Используется для Blend Space анимаций колёс и корпуса.

### Получение угла поворота колёс
```cpp
UFUNCTION(BlueprintPure, Category = "Train Animation")
float GetWheelRotationAngle() const
{
    // Диаметр колеса 100 см
    const float WheelCircumference = 100.0f * PI;
    return FMath::Fmod(CurrentDistance / WheelCircumference * 360.0f, 360.0f);
}
```
**Применение:** Подключить к Skeletal Mesh для анимации вращения колёс.

### Получение фактора ускорения
```cpp
UFUNCTION(BlueprintPure, Category = "Train Animation")
float GetAccelerationFactor() const
{
    if (TrainState == ETrainState::Accelerating) return 1.0f;
    if (TrainState == ETrainState::Decelerating) return -1.0f;
    return 0.0f;
}
```
**Применение:** Для визуальных эффектов, наклона корпуса или звуков.

### Получение состояния поезда
```cpp
UFUNCTION(BlueprintPure, Category = "Train Animation")
ETrainState GetTrainState() const
{
    return TrainState;
}
```
**Применение:** Для переключения между состояниями в State Machine Animation Blueprint.

### ARailsSplinePath
Компонент для управления сплайн-путём.

**Локация:** `Source/EpochRails/Train/RailsSplinePath.h`

#### Функции для анимаций

```cpp
// Получить направление на расстоянии для ориентации колёс
UFUNCTION(BlueprintPure, Category = "Path Animation")
FVector GetDirectionAtDistance(float Distance) const;

// Получить поворот сплайна для наклона поезда
UFUNCTION(BlueprintPure, Category = "Path Animation")
FRotator GetRotationAtDistance(float Distance) const;

// Получить кривизну пути для анимации наклона
UFUNCTION(BlueprintPure, Category = "Path Animation")
float GetCurvatureAtDistance(float Distance) const;
```

## Пример использования в Animation Blueprint

### Анимация вращения колёс
1. Создать переменную типа `float` — `Wheel Rotation`
2. В Event Blueprint обновить каждый frame:
   ```
   Wheel Rotation = Train->GetWheelRotationAngle()
   ```
3. Подключить `Wheel Rotation` к материалу колёс через Dynamic Material Instance

### Анимация ускорения/торможения
1. В State Machine создать переменную `Train State`
2. Используя `GetTrainState()` переключаться между
   - Idle (остановлен)
   - Moving (движется с постоянной скоростью)
   - Accelerating (ускоряется)
   - Decelerating (тормозит)
3. Для каждого состояния создать соответствующую анимацию

### Визуализация с Blend Space
```
Blend Space 1D:
  - Input: GetNormalizedSpeed() (0.0 - 1.0)
  - Animation 0: Idle (0.0)
  - Animation 50: Moving Slow (0.5)
  - Animation 100: Moving Fast (1.0)
```

## Пример настройки в Blueprint

### Создание поезда
1. Создать Actor Blueprint, наследующий ARailsTrain
2. Добавить Components в Construction Script:
   - TrainBodyMesh (Static Mesh)
   - PlatformMesh (Static Mesh)
3. В Details установить:
   - MaxSpeed: 2000
   - AccelerationRate: 500
   - DecelerationRate: 800
   - SplinePathRef: ссылка на созданный сплайн-путь
4. Создать Skeletal Mesh для анимированного поезда (опционально)

### Управление поездом из персонажа
```cpp
// В контроллере игрока или персонаже
if (AreAllActorsOnTrain())
{
    CurrentTrain->ApplyThrottle(ForwardAxis);
    CurrentTrain->ApplyBrake(BrakeAxis);
}
```

## Событие enum ETrainState

```cpp
enum class ETrainState : uint8
{
    Stopped,        // Поезд стоит на месте
    Moving,         // Движется с установленной скоростью
    Accelerating,   // Ускоряется
    Decelerating    // Замедляется/тормозит
};
```

## Производительность и советы

- **Tick оптимизация:** Используйте `TickInterval` для снижения частоты обновлений
- **Attachment система:** Персонажи крепятся к компоненту поезда, что автоматически следит за его движением
- **Анимации:** Используйте Blend Spaces вместо множественных монтажей для плавких переходов
- **Физика:** Поезд движется точно по сплайну без физических расчётов столкновений

## Частые ошибки

1. **Персонаж не движется с поездом**
   - Проверить, что `BoardingZone` имеет правильный размер
   - Убедиться, что персонаж прикреплен к `PlatformMesh`

2. **Поезд движется рывками**
   - Увеличить `CurrentDistance` приращение в `UpdateTrainMovement`
   - Проверить длину сплайна и `MaxSpeed`

3. **Анимация колёс не вращается**
   - Убедиться, что `GetWheelRotationAngle()` подключен к материалу
   - Проверить значение диаметра колеса в функции

## Версия документации
- **EpochRails Version:** 0.0.22
- **Last Updated:** 2025-12-04
- **Language:** Russian (Русский)
