# Система физики поезда (Train Physics System)

## 📋 Оглавление
- [Обзор](#обзор)
- [Физическая модель](#физическая-модель)
- [Компонент TrainPhysicsComponent](#компонент-trainphysicscomponent)
- [Параметры физики](#параметры-физики)
- [Использование в Blueprint](#использование-в-blueprint)
- [Интеграция с RailsTrain](#интеграция-с-railstrain)
- [Примеры реалистичных настроек](#примеры-реалистичных-настроек)

---

## Обзор

`TrainPhysicsComponent` — это компонент, симулирующий реалистичную физику движения поезда с учетом:
- ✅ Массы локомотива и вагонов
- ✅ Силы тяги двигателя
- ✅ Тормозной системы
- ✅ Сопротивлений (качение, воздух, уклон, повороты)
- ✅ Сцепления колес с рельсами (возможность пробуксовки)
- ✅ Уклонов пути (подъёмы/спуски)

---

## Физическая модель

### Основное уравнение движения

```
F_net = F_traction - F_braking - F_resistance
a = F_net / m
v = v₀ + a * Δt
```

Где:
- `F_net` — результирующая сила
- `F_traction` — сила тяги
- `F_braking` — тормозная сила
- `F_resistance` — суммарное сопротивление
- `m` — общая масса поезда
- `a` — ускорение
- `v` — скорость

### Компоненты сопротивления

#### 1. Сопротивление качению (Rolling Resistance)
```
F_rolling = C_rr * m * g * cos(θ)
```
- `C_rr` — коэффициент сопротивления качению (0.001-0.002)
- `m` — масса
- `g` — ускорение свободного падения (9.81 м/с²)
- `θ` — угол уклона

#### 2. Аэродинамическое сопротивление (Air Drag)
```
F_drag = 0.5 * ρ * C_d * A * v²
```
- `ρ` — плотность воздуха (1.225 кг/м³)
- `C_d * A` — коэффициент лобового сопротивления × площадь (5-10 для поездов)
- `v` — скорость

#### 3. Сопротивление от уклона (Grade Resistance)
```
F_grade = m * g * sin(θ)
```
- Положительное при подъёме (замедляет)
- Отрицательное при спуске (ускоряет)

#### 4. Сопротивление на поворотах (Curve Resistance)
```
F_curve = k * curvature * m * v
```
- Увеличивается на крутых поворотах
- Зависит от скорости

### Сила тяги (Tractive Force)

Сила тяги уменьшается с ростом скорости (ограничение по мощности):

```
F_traction = {
    F_max * throttle,           if v < v_transition
    (F_max * v_transition) / v, if v ≥ v_transition
}
```

Это моделирует реальное поведение: максимальная сила на низких скоростях, постоянная мощность на высоких.

### Сцепление колёс (Adhesion)

Максимальная сила, которую могут передать колёса:

```
F_adhesion_max = μ * m * g * cos(θ)
```

- `μ` — коэффициент сцепления (0.25-0.35 для сталь-сталь)
- Если `F_net > F_adhesion_max`, происходит пробуксовка

---

## Компонент TrainPhysicsComponent

### Структуры данных

#### FTrainPhysicsParameters

Настраиваемые параметры физики:

```cpp
USTRUCT(BlueprintType)
struct FTrainPhysicsParameters
{
    // Масса (Mass)
    float LocomotiveMass;      // Масса локомотива (кг)
    float WagonMass;            // Масса одного вагона (кг)
    int32 WagonCount;           // Количество вагонов
    
    // Мощность (Power)
    float MaxTractiveForce;     // Макс. сила тяги (Н)
    float MaxBrakingForce;      // Макс. тормозная сила (Н)
    
    // Сопротивления (Resistance)
    float RollingResistanceCoefficient;  // Коэф. качения
    float AirDragCoefficient;            // Коэф. аэродинамики
    float CurveResistanceFactor;         // Фактор поворотов
    
    // Сцепление (Adhesion)
    float AdhesionCoefficient;   // Коэф. сцепления
    
    // Окружение (Environment)
    float Gravity;               // Гравитация (м/с²)
    float AirDensity;            // Плотность воздуха (кг/м³)
};
```

#### FTrainPhysicsState

Текущее состояние физики (read-only):

```cpp
USTRUCT(BlueprintType)
struct FTrainPhysicsState
{
    float Velocity;              // Скорость (м/с)
    float Acceleration;          // Ускорение (м/с²)
    float TotalMass;             // Общая масса (кг)
    
    float AppliedTractiveForce;  // Применяемая сила тяги (Н)
    float AppliedBrakingForce;   // Применяемая тормозная сила (Н)
    float TotalResistance;       // Суммарное сопротивление (Н)
    
    float RollingResistance;     // Сопротивление качению (Н)
    float AirDragResistance;     // Сопротивление воздуха (Н)
    float GradeResistance;       // Сопротивление уклона (Н)
    float CurveResistance;       // Сопротивление поворота (Н)
    
    bool bIsWheelSlipping;       // Пробуксовка колёс?
    float DistanceTraveled;      // Пройденное расстояние (м)
};
```

### Основные функции

#### Управление

```cpp
// Set throttle (0.0 = idle, 1.0 = full power)
void SetThrottle(float NewThrottle);

// Set brake (0.0 = no brake, 1.0 = full brake)
void SetBrake(float NewBrake);

// Emergency brake (immediate maximum braking)
void EmergencyBrake();
```

#### Параметры пути

```cpp
// Set track grade in degrees (positive = uphill, negative = downhill)
void SetTrackGrade(float GradeDegrees);

// Set track curvature (0.0 = straight, 1.0 = tight curve)
void SetTrackCurvature(float Curvature);
```

#### Состав поезда

```cpp
// Add wagons to train
void AddWagons(int32 Count);

// Remove wagons from train
void RemoveWagons(int32 Count);

// Set wagon mass (for loaded/unloaded wagons)
void SetWagonMass(float NewWagonMass);
```

#### Информация

```cpp
// Get current velocity in km/h
float GetVelocityKmh() const;

// Get current velocity in m/s
float GetVelocityMs() const;

// Calculate stopping distance at current velocity
float CalculateStoppingDistance() const;
```

---

## Использование в Blueprint

### Базовая настройка

1. **Добавьте компонент к актору поезда:**

```
Blueprint: BP_Train
├─ Static Mesh Component (train body)
├─ TrainPhysicsComponent ← Добавить
└─ ...
```

2. **Настройте параметры физики:**

В деталях компонента `TrainPhysicsComponent`:

```
Physics Parameters:
  Mass:
    └─ Locomotive Mass: 80000 (кг)
    └─ Wagon Mass: 50000 (кг)
    └─ Wagon Count: 5
  
  Power:
    └─ Max Tractive Force: 400000 (Н)
    └─ Max Braking Force: 600000 (Н)
  
  Resistance:
    └─ Rolling Resistance Coefficient: 0.0015
    └─ Air Drag Coefficient: 7.0
    └─ Curve Resistance Factor: 0.5
  
  Adhesion:
    └─ Adhesion Coefficient: 0.30
```

### Пример управления в Blueprint

```
// Event Tick
┌─────────────────────────────────────┐
│ Get Input Axis Value (Throttle)     │  → [0.0 - 1.0]
└──────────────┬──────────────────────┘
               ↓
┌─────────────────────────────────────┐
│ TrainPhysicsComponent.SetThrottle   │
└─────────────────────────────────────┘

// Brake input
┌─────────────────────────────────────┐
│ Get Input Axis Value (Brake)        │  → [0.0 - 1.0]
└──────────────┬──────────────────────┘
               ↓
┌─────────────────────────────────────┐
│ TrainPhysicsComponent.SetBrake      │
└─────────────────────────────────────┘
```

### Получение скорости для UI

```
// Update speedometer
┌─────────────────────────────────────┐
│ TrainPhysicsComponent               │
└──────────────┬──────────────────────┘
               ↓
         GetVelocityKmh
               ↓
┌─────────────────────────────────────┐
│ Set Text (Speedometer Widget)       │  → "{0} km/h"
└─────────────────────────────────────┘
```

### Визуализация состояния

```
// Draw debug info
┌──────────────────────────────────────────────┐
│ TrainPhysicsComponent.PhysicsState           │
├──────────────────────────────────────────────┤
│ ├─ Velocity          → Show on HUD           │
│ ├─ Acceleration      → Show acceleration bar │
│ ├─ TotalResistance   → Debug display         │
│ └─ bIsWheelSlipping  → Show warning icon     │
└──────────────────────────────────────────────┘
```

---

## Интеграция с RailsTrain

### Шаг 1: Добавьте компонент в RailsTrain.h

```cpp
#include "TrainPhysicsComponent.h"

UCLASS()
class EPOCHRAILS_API ARailsTrain : public AActor
{
    GENERATED_BODY()
    
    // Add physics component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Train")
    UTrainPhysicsComponent* PhysicsComponent;
    
    // ...
};
```

### Шаг 2: Инициализация в конструкторе

```cpp
ARailsTrain::ARailsTrain()
{
    // Create physics component
    PhysicsComponent = CreateDefaultSubobject<UTrainPhysicsComponent>(TEXT("PhysicsComponent"));
    
    // ...
}
```

### Шаг 3: Используйте физику для движения

```cpp
void ARailsTrain::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update physics inputs
    PhysicsComponent->SetThrottle(CurrentThrottle);
    PhysicsComponent->SetBrake(CurrentBrake);
    
    // Get calculated velocity from physics
    float VelocityMs = PhysicsComponent->GetVelocityMs();
    
    // Move train along spline using physics velocity
    CurrentDistanceOnSpline += VelocityMs * DeltaTime;
    
    // Update position
    UpdateTrainPosition();
    
    // Update track parameters for physics
    UpdateTrackPhysicsParameters();
}

void ARailsTrain::UpdateTrackPhysicsParameters()
{
    if (SplinePath)
    {
        // Calculate grade from spline
        float Grade = CalculateTrackGrade();
        PhysicsComponent->SetTrackGrade(Grade);
        
        // Calculate curvature from spline
        float Curvature = CalculateTrackCurvature();
        PhysicsComponent->SetTrackCurvature(Curvature);
    }
}
```

---

## Примеры реалистичных настроек

### 1. Современный электровоз (типа ЧС7)

```
LocomotiveMass: 92000 kg
WagonCount: 0
MaxTractiveForce: 420000 N
MaxBrakingForce: 650000 N
RollingResistanceCoefficient: 0.0012
AirDragCoefficient: 6.5
AdhesionCoefficient: 0.33
```

**Характеристики:**
- Максимальная скорость: ~200 км/ч
- Разгон 0-100 км/ч: ~45 секунд
- Тормозной путь со 100 км/ч: ~800 метров

### 2. Грузовой состав (локомотив + 40 вагонов)

```
LocomotiveMass: 138000 kg
WagonMass: 80000 kg (загруженные)
WagonCount: 40
MaxTractiveForce: 550000 N
MaxBrakingForce: 800000 N
RollingResistanceCoefficient: 0.0018
AirDragCoefficient: 9.0
AdhesionCoefficient: 0.28
```

**Характеристики:**
- Общая масса: ~3338 тонн
- Максимальная скорость: ~90 км/ч
- Разгон 0-60 км/ч: ~4-5 минут
- Тормозной путь с 60 км/ч: ~1200 метров

### 3. Легкий пригородный поезд (электричка)

```
LocomotiveMass: 45000 kg (моторный вагон)
WagonMass: 38000 kg
WagonCount: 5
MaxTractiveForce: 280000 N
MaxBrakingForce: 400000 N
RollingResistanceCoefficient: 0.001
AirDragCoefficient: 5.5
AdhesionCoefficient: 0.32
```

**Характеристики:**
- Общая масса: ~235 тонн
- Максимальная скорость: ~130 км/ч
- Разгон 0-100 км/ч: ~35 секунд
- Тормозной путь со 100 км/ч: ~600 метров

### 4. Высокоскоростной поезд (типа Sapsan)

```
LocomotiveMass: 68000 kg (головной вагон)
WagonMass: 52000 kg
WagonCount: 8
MaxTractiveForce: 380000 N
MaxBrakingForce: 750000 N
RollingResistanceCoefficient: 0.0008
AirDragCoefficient: 4.2 (обтекаемая форма)
AdhesionCoefficient: 0.35
```

**Характеристики:**
- Общая масса: ~484 тонн
- Максимальная скорость: ~250 км/ч
- Разгон 0-200 км/ч: ~3-4 минуты
- Тормозной путь с 200 км/ч: ~2500 метров

---

## Расширенные возможности

### Реакция на погоду

```cpp
void SetWeatherConditions(EWeatherType Weather)
{
    switch (Weather)
    {
        case EWeatherType::Rain:
            PhysicsComponent->PhysicsParameters.AdhesionCoefficient = 0.20f; // Снижение сцепления
            PhysicsComponent->PhysicsParameters.RollingResistanceCoefficient = 0.002f;
            break;
            
        case EWeatherType::Snow:
            PhysicsComponent->PhysicsParameters.AdhesionCoefficient = 0.15f; // Ещё хуже
            PhysicsComponent->PhysicsParameters.RollingResistanceCoefficient = 0.0025f;
            break;
            
        case EWeatherType::Clear:
        default:
            PhysicsComponent->PhysicsParameters.AdhesionCoefficient = 0.30f;
            PhysicsComponent->PhysicsParameters.RollingResistanceCoefficient = 0.0015f;
            break;
    }
}
```

### Система предупреждений

```cpp
void CheckPhysicsWarnings()
{
    if (PhysicsComponent->PhysicsState.bIsWheelSlipping)
    {
        // Show "WHEEL SLIP" warning
        ShowWarning("Пробуксовка колёс!");
    }
    
    float StoppingDistance = PhysicsComponent->CalculateStoppingDistance();
    if (StoppingDistance > DistanceToNextStation)
    {
        // Warning: can't stop in time
        ShowWarning("Внимание! Тормозной путь превышает расстояние до станции!");
    }
}
```

---

## Советы по оптимизации

1. **Tick Rate**: Физика не требует обновления каждый кадр для визуального качества. Можно использовать:
   ```cpp
   PrimaryComponentTick.TickInterval = 0.016f; // 60 Hz достаточно
   ```

2. **Плавность движения**: Используйте интерполяцию для визуального позиционирования между физическими апдейтами.

3. **Сетевая игра**: Физика должна считаться только на сервере, клиенты получают результаты через репликацию.

---

## Заключение

Система физики поезда обеспечивает:
- ✅ Реалистичное поведение с учётом массы и сил
- ✅ Правильную реакцию на уклоны и повороты
- ✅ Сложность управления тяжёлыми составами
- ✅ Возможность пробуксовки при перегрузке
- ✅ Реалистичные тормозные пути
- ✅ Простую интеграцию через Blueprint

Это создаёт увлекательный геймплей, где игрок должен учитывать физические ограничения при управлении поездом! 🚂
