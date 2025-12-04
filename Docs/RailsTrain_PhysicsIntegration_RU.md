# 🚂 Интеграция Физики в BP_RailsTrain

## 📋 Содержание
- [Быстрый старт](#быстрый-старт)
- [Настройка в Blueprint](#настройка-в-blueprint)
- [Примеры управления](#примеры-управления)
- [Визуализация и отладка](#визуализация-и-отладка)
- [Советы и рекомендации](#советы-и-рекомендации)

---

## 🚀 Быстрый старт

### Что изменилось?

✅ **Автоматическая интеграция** - Физический компонент уже добавлен в C++ класс  
✅ **Переключатель режимов** - Можно использовать физику или старую систему  
✅ **Автоматический расчёт** - Уклоны и повороты считаются автоматически  
✅ **Обратная совместимость** - Старые Blueprint'ы продолжат работать  

---

## ⚙️ Настройка в Blueprint

### Шаг 1: Откройте BP_RailsTrain

1. Найдите ваш `BP_RailsTrain` в Content Browser
2. Откройте его двойным кликом
3. Перейдите в режим **Class Defaults** (кнопка вверху)

### Шаг 2: Включите физическую симуляцию

В панели **Details** найдите раздел **Movement**:

```
┌─────────────────────────────────────────┐
│ Movement                                │
├─────────────────────────────────────────┤
│ ☑ Use Physics Simulation       ← ВКЛ   │
│                                         │
│ Spline Path Ref: [выберите путь]        │
│ Max Speed: 2000.0              (legacy) │
│ Acceleration Rate: 500.0       (legacy) │
│ Deceleration Rate: 800.0       (legacy) │
│ Loop Path: ☑                            │
│ Auto Start: ☑                           │
└─────────────────────────────────────────┘
```

**⚠️ Важно:** Когда `Use Physics Simulation = true`, параметры `Max Speed`, `Acceleration Rate` и `Deceleration Rate` **игнорируются**. Используются параметры из `Physics Component`.

### Шаг 3: Настройте физические параметры

В **Components** панели найдите **PhysicsComponent** и настройте его:

#### 🎚️ Базовые параметры (для начала)

```
┌─────────────────────────────────────────┐
│ Physics Parameters                      │
├─────────────────────────────────────────┤
│ Mass:                                   │
│   Locomotive Mass: 92000       (кг)     │
│   Wagon Mass: 50000            (кг)     │
│   Wagon Count: 0                        │
│                                         │
│ Power:                                  │
│   Max Tractive Force: 420000   (Н)     │
│   Max Braking Force: 650000    (Н)     │
└─────────────────────────────────────────┘
```

#### 🔧 Продвинутые параметры

```
┌─────────────────────────────────────────┐
│ Resistance:                             │
│   Rolling Resistance: 0.0015            │
│   Air Drag Coefficient: 7.0             │
│   Curve Resistance Factor: 0.5          │
│                                         │
│ Adhesion:                               │
│   Adhesion Coefficient: 0.30            │
│                                         │
│ Environment:                            │
│   Gravity: 9.81           (м/с²)        │
│   Air Density: 1.225      (кг/м³)       │
└─────────────────────────────────────────┘
```

### Шаг 4: (Опционально) Настройте параметры физики пути

В разделе **Physics**:

```
┌─────────────────────────────────────────┐
│ Physics                                 │
├─────────────────────────────────────────┤
│ Physics Sample Distance: 100.0  (см)    │
│ Grade Smoothing Speed: 2.0              │
│                                         │
│ Physics | Debug:                        │
│ ☑ Show Physics Debug                    │
└─────────────────────────────────────────┘
```

**Physics Sample Distance** - на какое расстояние вперёд смотреть для расчёта уклона  
**Grade Smoothing Speed** - скорость сглаживания изменений уклона  
**Show Physics Debug** - показывать информацию о физике на экране  

---

## 🎮 Примеры управления в Blueprint

### Пример 1: Простое управление (W/S клавиши)

```
Event Graph:

┌────────────────────────────────────┐
│ Event Tick                         │
└────────┬───────────────────────────┘
         │
         ├─► Input Axis Value "MoveForward"  → [0.0 - 1.0]
         │         │
         │         └─► ApplyThrottle (Self)
         │
         └─► Input Axis Value "Brake"  → [0.0 - 1.0]
                   │
                   └─► ApplyBrake (Self)
```

**Input Mappings:**
```
MoveForward:
  W → Scale 1.0
  S → Scale -1.0

Brake:
  Space → Scale 1.0
```

### Пример 2: Плавное управление (как в симуляторах)

```
Event Graph:

[Event Tick]
  │
  ├─► Get Input Axis Value "ThrottleAxis"  → Target Throttle
  │         │
  │         └─► FInterpTo (Current → Target, Speed: 2.0)
  │                   │
  │                   └─► ApplyThrottle
  │
  └─► Get Input Axis Value "BrakeAxis"  → Target Brake
            │
            └─► FInterpTo (Current → Target, Speed: 3.0)
                      │
                      └─► ApplyBrake
```

### Пример 3: Автоматическое управление (круиз-контроль)

```
Custom Function: "MaintainSpeed"
Input: Target Speed (float)

┌────────────────────────────────────┐
│ Get Current Speed Kmh              │
└────────┬───────────────────────────┘
         │
         ├─► [Current Speed] < [Target Speed]?
         │         │
         │         ├─── True ──► Set Throttle: 0.7
         │         │             Set Brake: 0.0
         │         │
         │         └─── False ─► [Current Speed] > [Target Speed + 5]?
         │                   │
         │                   ├─── True ──► Set Throttle: 0.0
         │                   │             Set Brake: 0.3
         │                   │
         │                   └─── False ─► Set Throttle: 0.4
         │                                 Set Brake: 0.0
         │
         └─► Apply values
```

### Пример 4: Экстренное торможение

```
[Input Action "EmergencyBrake" Pressed]
  │
  └─► Emergency Brake (Self)
        │
        └─► Play Sound "Warning Siren"
              │
              └─► Show Warning Widget
```

---

## 📊 Визуализация и отладка

### Включение debug информации

**В Class Defaults:**
```
Physics | Debug:
  ☑ Show Physics Debug
```

**Что будет отображаться:**
```
=== TRAIN PHYSICS DEBUG ===
Speed: 85.3 km/h (23.7 m/s)
Acceleration: 0.42 m/s²
Mass: 342000 kg

Forces:
  Tractive: 285000 N
  Braking: 0 N
  Total Resistance: 12450 N

Resistance Breakdown:
  Rolling: 5024 N
  Air Drag: 3915 N
  Grade: 3200 N (1.85°)
  Curve: 311 N (0.12)

Track:
  Grade: 1.85°
  Curvature: 0.12

Status:
  Wheel Slip: NO
  Stopping Distance: 285 m
  Distance Traveled: 4523 m
```

### Создание HUD виджета

**Создайте Widget Blueprint "WBP_TrainHUD":**

```
Canvas Panel
 ├─ Text Block "Speed"
 │    Binding → GetCurrentSpeedKmh + " km/h"
 │
 ├─ Progress Bar "Throttle"
 │    Binding → GetThrottlePosition
 │    Fill Color: Green
 │
 ├─ Progress Bar "Brake"
 │    Binding → GetBrakePosition
 │    Fill Color: Red
 │
 ├─ Text Block "Stopping Distance"
 │    Binding → GetStoppingDistance + " m"
 │
 └─ Image "Warning" (Wheel Slip)
      Visibility → PhysicsComponent.PhysicsState.bIsWheelSlipping
```

### Визуальные индикаторы

**Пример: Мигающий индикатор при пробуксовке**

```
Event Tick
  │
  └─► Get Physics Component
        │
        └─► PhysicsState.bIsWheelSlipping?
              │
              ├─── True ──► Play Timeline "Blink Warning"
              │                   │
              │                   └─► Set Material Parameter "Intensity"
              │
              └─── False ─► Set Material Parameter "Intensity" = 0
```

---

## 💡 Советы и рекомендации

### 🎯 Настройка для разных типов поездов

#### Лёгкий пригородный поезд (быстрый, манёвренный)
```
LocomotiveMass: 45000
WagonMass: 38000
WagonCount: 5
MaxTractiveForce: 280000
MaxBrakingForce: 400000
```
**Характер:** Быстрый разгон, короткий тормозной путь

#### Грузовой состав (медленный, тяжёлый)
```
LocomotiveMass: 138000
WagonMass: 80000
WagonCount: 40
MaxTractiveForce: 550000
MaxBrakingForce: 800000
```
**Характер:** Медленный разгон, очень длинный тормозной путь

#### Высокоскоростной (быстрый, но тяжёлый)
```
LocomotiveMass: 68000
WagonMass: 52000
WagonCount: 8
MaxTractiveForce: 380000
MaxBrakingForce: 750000
AirDragCoefficient: 4.2  ← меньше (обтекаемая форма)
```
**Характер:** Высокая максимальная скорость, среднее торможение

### ⚠️ Распространённые проблемы

**Проблема:** Поезд не движется после включения физики  
**Решение:** Убедитесь, что `CurrentThrottle > 0` или вызывается `StartTrain()`

**Проблема:** Колёса постоянно буксуют  
**Решение:** 
- Увеличьте `AdhesionCoefficient` (0.30 → 0.35)
- Или уменьшите `MaxTractiveForce`
- Или уменьшите массу (`WagonCount`)

**Проблема:** Поезд не чувствует подъёмы  
**Решение:** Убедитесь, что spline имеет вертикальные изменения и `Physics Sample Distance` достаточно большое (100+ см)

**Проблема:** Слишком резкие изменения на переходах между подъёмом/спуском  
**Решение:** Увеличьте `Grade Smoothing Speed` (2.0 → 5.0)

**Проблема:** Поезд слишком быстро разгоняется/тормозит  
**Решение:** Увеличьте массу (`LocomotiveMass`, `WagonMass`, `WagonCount`)

### 🎨 Улучшение геймплея

#### 1. Система предупреждений

```
Custom Function: "CheckWarnings"

[Get Stopping Distance] > [Distance To Station]?
  │
  ├─── True ──► Show Warning "TOO FAST!"
  │              Play Sound "Alarm"
  │
  └─── False ─► Hide Warning

[PhysicsState.bIsWheelSlipping]?
  │
  └─── True ──► Show Warning "WHEEL SLIP!"
                 Play Sound "Screech"
```

#### 2. Система достижений

```
Event: "Arrived At Station"

├─► [Distance From Target] < 5m?
│     │
│     └─── True ──► Achievement "Perfect Stop"
│
├─► [Max Speed Used] < [Speed Limit]?
│     │
│     └─── True ──► Achievement "Safe Driver"
│
└─► [Average Throttle] < 0.7?
      │
      └─── True ──► Achievement "Fuel Efficient"
```

#### 3. Динамические условия

```
[Weather Changed To Rain]
  │
  └─► PhysicsComponent.PhysicsParameters:
        ├─ AdhesionCoefficient = 0.20  (было 0.30)
        └─ RollingResistance = 0.002   (было 0.0015)

[Weather Changed To Snow]
  │
  └─► PhysicsComponent.PhysicsParameters:
        ├─ AdhesionCoefficient = 0.15  (было 0.30)
        └─ RollingResistance = 0.0025  (было 0.0015)
```

### 🚀 Оптимизация производительности

1. **Physics Sample Distance:**
   - Для прямых путей: 50-100 см
   - Для извилистых путей: 100-200 см
   - Слишком большое значение → лишние вычисления

2. **Grade Smoothing Speed:**
   - Для плавных путей: 1.0-2.0
   - Для резких изменений: 3.0-5.0
   - Выше → больше вычислений каждый кадр

3. **Debug Info:**
   - Отключайте `Show Physics Debug` в финальной сборке
   - Используйте только для тестирования

### 📝 Cheat Sheet: Быстрые команды

**В игре (если включён console):**
```
// Показать физику
ShowDebug TrainPhysics

// Добавить вагоны
AddWagons 10

// Убрать вагоны
RemoveWagons 5

// Экстренное торможение
EmergencyBrake
```

**Blueprint функции:**
```cpp
// Получить скорость
GetCurrentSpeedKmh() → float (км/ч)
GetCurrentSpeed() → float (см/с)

// Управление
ApplyThrottle(0.5) → Установить тягу 50%
ApplyBrake(0.8) → Установить тормоз 80%
EmergencyBrake() → Аварийное торможение

// Информация
GetStoppingDistance() → float (метры)
GetPhysicsComponent() → UTrainPhysicsComponent*

// Состав
AddWagons(5) → Добавить 5 вагонов
RemoveWagons(2) → Убрать 2 вагона
```

---

## 🎓 Дополнительные ресурсы

- **Полная документация физики:** [TrainPhysicsSystem_RU.md](TrainPhysicsSystem_RU.md)
- **Быстрый старт:** [TrainPhysics_QuickStart.md](TrainPhysics_QuickStart.md)
- **Система управления поездом:** [TrainSystem_RU.md](TrainSystem_RU.md)

---

## ✅ Контрольный список интеграции

- [ ] Открыл BP_RailsTrain
- [ ] Включил `Use Physics Simulation`
- [ ] Настроил параметры массы и мощности
- [ ] Указал `Spline Path Ref`
- [ ] Создал Input Mappings для управления
- [ ] Подключил `ApplyThrottle` и `ApplyBrake`
- [ ] Протестировал на ровном пути
- [ ] Протестировал на подъёме/спуске
- [ ] Создал HUD виджет для отображения скорости
- [ ] Настроил систему предупреждений
- [ ] Отключил `Show Physics Debug` перед релизом

---

**🎉 Готово! Теперь у вас реалистичная физика в BP_RailsTrain!**

**Вопросы? Проблемы? Смотрите раздел "Распространённые проблемы" выше.** 🚂
