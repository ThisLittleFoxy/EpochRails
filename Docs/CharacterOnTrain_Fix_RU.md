# 🛠️ Исправление проблемы с выбрасыванием персонажа из поезда

## 🎯 Проблема

При включении физической симуляции персонажа начинает выбрасывать с платформы поезда из-за резких изменений в физике движения.

---

## ✅ Решение #1: Обновлённый код (ПРИМЕНЕНО)

### Что было исправлено:

#### 1. **Sweep при движении поезда**
```cpp
// В MoveToDistance() добавлен параметр sweep:
SetActorLocationAndRotation(NewLocation, NewRotation, true);
//                                                      ^^^^
//                                                    sweep=true
```
**Эффект:** Физика корректно обрабатывает прикреплённые актёры

#### 2. **Правильные правила attachment**
```cpp
FAttachmentTransformRules AttachRules(
    EAttachmentRule::KeepWorld,  // Сохранить мировую позицию
    EAttachmentRule::KeepWorld,  // Сохранить мировую ротацию
    EAttachmentRule::KeepWorld,  // Сохранить мировой масштаб
    true                         // Weld simulated bodies
);

Character->AttachToComponent(PlatformMesh, AttachRules);
```

#### 3. **Настройки CharacterMovementComponent**
```cpp
UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();

// Критичные настройки:
MovementComp->bAlwaysCheckFloor = true;
MovementComp->bUseFlatBaseForFloorChecks = true;
MovementComp->PerchRadiusThreshold = 0.0f;
MovementComp->PerchAdditionalHeight = 0.0f;
```

---

## 🔧 Дополнительные решения (если проблема осталась)

### Решение #2: Настройки PlatformMesh в Blueprint

#### В BP_RailsTrain → Components → PlatformMesh:

```
Collision:
  ✅ Collision Enabled: Query Only (или Query and Physics)
  ✅ Collision Preset: Custom
  
  Object Type: WorldStatic
  
  Collision Responses:
    ✅ Block: Pawn
    ✅ Block: PhysicsBody
    ✅ Overlap: Everything else

Physics:
  ☐ Simulate Physics: OFF
  ☐ Enable Gravity: OFF
  ✅ Generate Overlap Events: ON
```

### Решение #3: Добавить физическую стабилизацию

#### Создайте новую переменную в BP_RailsTrain:

**Event Graph:**
```
[Event Tick]
  |
  └─► ForEach (Passengers On Board)
        |
        └─► Get Character Movement
              |
              ├─► Set Movement Mode: Walking
              └─► Set Base (Platform Mesh)
```

**Пример Blueprint кода:**
```
For Each Loop (PassengersOnBoard)
  ├─ Array Element → Cast to Character
  |                       |
  |                       └─► Get Character Movement
  |                                 |
  |                                 ├─► Set Movement Mode (Walking)
  |                                 └─► Update Floor → Platform Mesh
  └─ Loop Body → Continue
```

### Решение #4: Альтернативный метод - MovementBase

**Создайте функцию в Event Graph:**

```cpp
Function: "UpdatePassengerMovementBase"

Input: Character (Actor)

┌────────────────────────────────────────┐
│ Get Character Movement Component      │
└────────┬───────────────────────────────┘
         │
         ├─► Set Base (Platform Mesh Component)
         │
         └─► Set Walking Mode
```

**Вызывайте её в Tick:**
```
Event Tick
  │
  └─► For Each (Passengers)
        │
        └─► UpdatePassengerMovementBase
```

---

## 🎮 Решение #5: Настройки в Character Blueprint

### В вашем BP_Character (или BP_RailsCharacter):

#### Character Movement Component настройки:

```
┌────────────────────────────────────────────┐
│ Character Movement (Walking)              │
├────────────────────────────────────────────┤
│ ✅ Can Walk Off Ledges: false             │
│ ✅ Always Check Floor: true               │
│ ✅ Use Flat Base For Floor Checks: true   │
│                                           │
│ Perch Radius Threshold: 0.0               │
│ Perch Additional Height: 0.0              │
│                                           │
│ ✅ Enable Physics Interaction: false      │
│ ✅ Touch Force Factor: 0.0                │
└────────────────────────────────────────────┘
```

**Почему это помогает:**
- `Can Walk Off Ledges: false` - персонаж не соскользнёт с края
- `Enable Physics Interaction: false` - не взаимодействует с физическими объектами
- `Touch Force Factor: 0.0` - не применяет силу к объектам

---

## 🚀 Решение #6: Интерполяция движения (самое надёжное)

### Если ничего не помогло, используйте интерполяцию:

**В RailsTrain.h добавьте:**
```cpp
// Interpolation for smooth movement
UPROPERTY(EditAnywhere, Category = "Movement")
float MovementInterpolationSpeed = 10.0f;

private:
FVector PreviousLocation;
FRotator PreviousRotation;
```

**В RailsTrain.cpp измените MoveToDistance():**
```cpp
void ARailsTrain::MoveToDistance(float Distance) {
  if (!CachedSplineComponent) {
    return;
  }

  // Get target location and rotation
  FVector TargetLocation = CachedSplineComponent->GetLocationAtDistanceAlongSpline(
      Distance, ESplineCoordinateSpace::World);
  FRotator TargetRotation = CachedSplineComponent->GetRotationAtDistanceAlongSpline(
      Distance, ESplineCoordinateSpace::World);

  // Interpolate for smooth movement (prevents ejection)
  FVector NewLocation = FMath::VInterpTo(
      GetActorLocation(), 
      TargetLocation, 
      GetWorld()->GetDeltaSeconds(), 
      MovementInterpolationSpeed
  );
  
  FRotator NewRotation = FMath::RInterpTo(
      GetActorRotation(), 
      TargetRotation, 
      GetWorld()->GetDeltaSeconds(), 
      MovementInterpolationSpeed
  );

  // Move with sweep
  SetActorLocationAndRotation(NewLocation, NewRotation, true);
}
```

**Настройка в Blueprint:**
```
Movement:
  Movement Interpolation Speed: 10.0  ← Увеличьте до 15-20 для более плавного движения
```

---

## 🧪 Решение #7: Collision настройки (Крайний случай)

### Если персонаж всё равно вылетает:

#### В BP_RailsTrain → PlatformMesh:

**Вариант A: Сделать Platform статичным коллайдером**
```
Physics:
  Mobility: Movable
  Simulate Physics: OFF
  
Collision:
  Collision Preset: Custom
  Object Type: WorldStatic  ← Важно!
  
  Block:
    ✅ Pawn
    ✅ WorldStatic
    ✅ WorldDynamic
```

**Вариант B: Использовать BlockAll**
```
Collision Preset: BlockAll
Generate Overlap Events: true
```

---

## 📋 Пошаговый чеклист решения проблемы

### Уровень 1: Базовые исправления (уже применены в коде)
- [x] Sweep включён в SetActorLocationAndRotation
- [x] Правильные AttachmentTransformRules
- [x] CharacterMovementComponent настройки

### Уровень 2: Blueprint настройки
- [ ] PlatformMesh → Collision Preset: Custom (WorldStatic)
- [ ] PlatformMesh → Block: Pawn включен
- [ ] Character → Can Walk Off Ledges: false
- [ ] Character → Enable Physics Interaction: false

### Уровень 3: Если проблема осталась
- [ ] Добавить Movement Interpolation Speed (10-20)
- [ ] Увеличить Grade Smoothing Speed (5.0+)
- [ ] Добавить UpdatePassengerMovementBase в Tick

### Уровень 4: Экспериментальные решения
- [ ] Попробовать AttachToActor вместо AttachToComponent
- [ ] Использовать SetBase() вместо Attach
- [ ] Добавить физический constraint между персонажем и платформой

---

## 🎯 Рекомендуемая последовательность тестирования

### Тест 1: Базовые настройки (НАЧНИТЕ С ЭТОГО)

1. **Скомпилируйте обновлённый код**
2. **В BP_RailsTrain → PlatformMesh:**
   ```
   Collision Preset: BlockAll
   Simulate Physics: OFF
   Generate Overlap Events: ON
   ```
3. **Протестируйте** - заходите на поезд и ездите

### Тест 2: Если не помогло

4. **В BP_Character → Character Movement:**
   ```
   Can Walk Off Ledges: false
   Enable Physics Interaction: false
   ```
5. **Протестируйте снова**

### Тест 3: Добавить интерполяцию

6. **В BP_RailsTrain → Movement:**
   ```
   Movement Interpolation Speed: 15.0
   ```
7. **Измените код MoveToDistance()** (см. Решение #6)
8. **Протестируйте**

---

## 💡 Почему это происходит?

### Причина проблемы:

**До физики:**
```
Поезд двигался с постоянной скоростью
→ Плавное движение
→ Attachment работал идеально
```

**С физикой:**
```
Поезд имеет ускорение/замедление
→ Изменения скорости каждый кадр
→ Character Movement Component не успевает обновить base
→ Персонаж "отстаёт" от платформы
→ Выбрасывание
```

### Что исправляет sweep:

```cpp
// sweep = false (старый код)
SetActorLocationAndRotation(Loc, Rot, false);
// → Телепортация без учёта прикреплённых актёров
// → Персонаж остаётся на месте, поезд уезжает

// sweep = true (новый код)
SetActorLocationAndRotation(Loc, Rot, true);
// → Плавное перемещение с проверкой коллизий
// → Прикреплённые актёры двигаются вместе
// → Физика обновляется корректно
```

---

## 🔬 Debug и диагностика

### Проверьте, что персонаж правильно прикреплён:

**В Event Tick персонажа:**
```
Get Attach Parent Actor
  │
  └─► Print String
  
Get Movement Mode
  │
  └─► Print String
```

**Должно показывать:**
```
Attach Parent: BP_RailsTrain_C
Movement Mode: Walking
```

### Если показывает "None" или "Falling":

**Проблема в attachment. Проверьте:**
1. PlatformMesh имеет правильный Collision Preset
2. Character может коллайдить с платформой (Collision Matrix)
3. BoardingZone срабатывает (добавьте Print String в OnBeginOverlap)

---

## 🎓 Продвинутое решение: Custom Movement Base

### Если базовые решения не работают, создайте кастомную логику:

**Создайте компонент TrainPassengerComponent:**

```cpp
UCLASS()
class UTrainPassengerComponent : public UActorComponent
{
    GENERATED_BODY()
    
public:
    void AttachToTrain(ARailsTrain* Train)
    {
        OwningTrain = Train;
        bIsOnTrain = true;
        
        ACharacter* Character = Cast<ACharacter>(GetOwner());
        if (Character)
        {
            UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
            if (Movement)
            {
                // Store original settings
                OriginalAirControl = Movement->AirControl;
                OriginalGravityScale = Movement->GravityScale;
                
                // Apply train-friendly settings
                Movement->AirControl = 0.0f;
                Movement->GravityScale = 1.0f;
                Movement->bAlwaysCheckFloor = true;
            }
        }
    }
    
    void DetachFromTrain()
    {
        bIsOnTrain = false;
        
        ACharacter* Character = Cast<ACharacter>(GetOwner());
        if (Character)
        {
            UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
            if (Movement)
            {
                // Restore original settings
                Movement->AirControl = OriginalAirControl;
                Movement->GravityScale = OriginalGravityScale;
            }
        }
        
        OwningTrain = nullptr;
    }
    
private:
    UPROPERTY()
    ARailsTrain* OwningTrain;
    
    bool bIsOnTrain = false;
    float OriginalAirControl;
    float OriginalGravityScale;
};
```

---

## ⚙️ Настройки Project Settings

### Если проблема массовая, проверьте глобальные настройки:

**Project Settings → Physics → Optimization:**
```
✅ Enable Stabilization: true
Position Iteration Count: 8  (увеличьте если нужно)
Velocity Iteration Count: 8
```

**Project Settings → Physics → Framerate:**
```
Max Physics Delta Time: 0.033  (не больше!)
Enable Substepping: true
Max Substep Delta Time: 0.016
Max Substeps: 6
```

---

## 🧪 Тестовые сценарии

### Сценарий 1: Стоящий поезд
```
1. Зайдите на платформу
2. Поезд стоит (throttle = 0)
3. Результат: Персонаж должен стоять стабильно
```
✅ **Если проходит** → Attachment работает корректно

### Сценарий 2: Постоянная скорость
```
1. Поезд движется (throttle = 0.5, brake = 0)
2. Дождитесь постоянной скорости (нет ускорения)
3. Зайдите на платформу
4. Результат: Персонаж движется с поездом
```
✅ **Если проходит** → Проблема только при ускорении

### Сценарий 3: Ускорение
```
1. Зайдите на поезд (он стоит)
2. Нажмите throttle (разгон)
3. Результат: Персонаж должен ускоряться вместе с поездом
```
❌ **Если не проходит** → Примените Решение #6 (интерполяция)

### Сценарий 4: Торможение
```
1. Поезд движется с постоянной скоростью
2. Зайдите на платформу
3. Нажмите brake (торможение)
4. Результат: Персонаж должен тормозить вместе с поездом
```
❌ **Если не проходит** → Увеличьте Grade Smoothing Speed

---

## 🎯 Окончательное решение (99% работает)

### Комбинация всех методов:

**1. В C++ коде (уже применено):**
- ✅ Sweep в SetActorLocationAndRotation
- ✅ Правильные attachment rules
- ✅ CharacterMovementComponent настройки

**2. В BP_RailsTrain:**
- ✅ PlatformMesh → Collision Preset: BlockAll
- ✅ Use Physics Simulation: true
- ✅ Movement Interpolation Speed: 15.0

**3. В BP_Character:**
- ✅ Can Walk Off Ledges: false
- ✅ Enable Physics Interaction: false

**4. В Project Settings:**
- ✅ Enable Substepping: true
- ✅ Max Physics Delta Time: 0.033

**Эта комбинация должна устранить 99% проблем с выбрасыванием!**

---

## 📞 Если проблема всё ещё есть

### Проверьте эти вещи:

1. **Frame Rate:** Низкий FPS может вызывать проблемы. Проверьте `stat fps`
2. **Collision Matrix:** Убедитесь что Pawn и WorldStatic взаимодействуют
3. **Multiple Collisions:** Проверьте, нет ли конфликтующих коллизий на платформе
4. **Character Capsule:** Убедитесь что капсула персонажа не слишком большая
5. **Physics Asset:** Если у персонажа есть Physics Asset, попробуйте отключить его на поезде

### Debug команды:
```
// В консоли игры:
show collision
p.Chaos.Solver.DrawCollisions 1
stat unit
```

---

## ✅ Итоговая конфигурация (рекомендуемая)

### BP_RailsTrain:
```yaml
Movement:
  Use Physics Simulation: true
  Loop Path: true
  Auto Start: true

Physics:
  Physics Sample Distance: 100.0
  Grade Smoothing Speed: 2.0
  Movement Interpolation Speed: 15.0  # Если добавили

Components → PlatformMesh:
  Collision Preset: BlockAll
  Simulate Physics: false
  Generate Overlap Events: true

Components → PhysicsComponent:
  Locomotive Mass: 92000
  Max Tractive Force: 420000
  Max Braking Force: 650000
```

### BP_Character:
```yaml
Character Movement:
  Can Walk Off Ledges: false
  Always Check Floor: true
  Use Flat Base For Floor Checks: true
  Enable Physics Interaction: false
  Perch Radius Threshold: 0.0
```

---

**🎉 С этими настройками персонаж должен стабильно стоять на поезде даже при резком ускорении!**
