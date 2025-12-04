# 🚀 Train Physics - Быстрый старт

## 5 минут до реалистичной физики!

---

## Шаг 1: Добавьте компонент в Blueprint

1. Откройте ваш Blueprint поезда
2. Нажмите **Add Component**
3. Найдите **Train Physics Component**
4. Добавьте его

---

## Шаг 2: Выберите пресет (или настройте вручную)

### Пресет 1: Электровоз (быстрый)
```
LocomotiveMass: 92000
WagonCount: 0
MaxTractiveForce: 420000
MaxBrakingForce: 650000
```

### Пресет 2: Грузовой (тяжёлый)
```
LocomotiveMass: 138000
WagonMass: 80000
WagonCount: 20
MaxTractiveForce: 550000
MaxBrakingForce: 800000
```

### Пресет 3: Электричка (лёгкий)
```
LocomotiveMass: 45000
WagonMass: 38000
WagonCount: 5
MaxTractiveForce: 280000
MaxBrakingForce: 400000
```

---

## Шаг 3: Подключите управление (Event Graph)

### Минимальный код:

```
Event Tick
  │
  ├── Get Input Axis Value ("Throttle") → TrainPhysicsComponent → SetThrottle
  └── Get Input Axis Value ("Brake") → TrainPhysicsComponent → SetBrake
```

**Или в коде:**
```cpp
void ATrain::Tick(float DeltaTime)
{
    float Throttle = GetInputAxisValue("Throttle");
    float Brake = GetInputAxisValue("Brake");
    
    PhysicsComponent->SetThrottle(Throttle);
    PhysicsComponent->SetBrake(Brake);
}
```

---

## Шаг 4: Используйте скорость для движения

```
Event Tick
  │
  └── TrainPhysicsComponent → GetVelocityMs → Используйте для движения по сплайну
```

**Пример:**
```cpp
float VelocityMs = PhysicsComponent->GetVelocityMs();
CurrentDistance += VelocityMs * DeltaTime;
UpdatePositionOnSpline(CurrentDistance);
```

---

## Шаг 5: (Опционально) Добавьте уклоны

Если у вас есть spline с уклонами:

```cpp
// Рассчитайте угол наклона пути
FVector Tangent = Spline->GetTangentAtDistanceAlongSpline(CurrentDistance);
float GradeDegrees = FMath::RadiansToDegrees(FMath::Asin(Tangent.Z / Tangent.Size()));

// Передайте в физику
PhysicsComponent->SetTrackGrade(GradeDegrees);
```

---

## 🎯 Результат

Теперь у вас есть:

✅ **Реалистичное ускорение** - поезд не мгновенно набирает скорость  
✅ **Инерция** - тяжёлый состав медленно разгоняется  
✅ **Длинный тормозной путь** - нужно тормозить заранее  
✅ **Сопротивление воздуха** - чем быстрее, тем сильнее сопротивление  
✅ **Уклоны** - поезд замедляется на подъёмах  
✅ **Пробуксовка** - при перегрузке колёса пробуксовывают  

---

## 💡 Полезные функции

### Показать скорость на UI
```cpp
float SpeedKmh = PhysicsComponent->GetVelocityKmh();
SpeedometerText->SetText(FText::FromString(FString::Printf(TEXT("%.0f km/h"), SpeedKmh)));
```

### Рассчитать тормозной путь
```cpp
float StoppingDistance = PhysicsComponent->CalculateStoppingDistance();
if (StoppingDistance > DistanceToStation)
{
    ShowWarning("Слишком быстро! Не остановитесь!");
}
```

### Аварийное торможение
```cpp
// Кнопка "E" для экстренного торможения
if (Input->IsKeyPressed(EKeys::E))
{
    PhysicsComponent->EmergencyBrake();
}
```

### Добавить/убрать вагоны
```cpp
PhysicsComponent->AddWagons(5);      // Добавить 5 вагонов
PhysicsComponent->RemoveWagons(2);   // Убрать 2 вагона
```

---

## 🔧 Отладка

### Показать состояние физики на экране:

```cpp
void DrawDebugPhysics()
{
    FString DebugText = FString::Printf(
        TEXT("Speed: %.0f km/h\nAccel: %.2f m/s²\nMass: %.0f kg\nBraking Dist: %.0f m\nWheel Slip: %s"),
        PhysicsComponent->GetVelocityKmh(),
        PhysicsComponent->PhysicsState.Acceleration,
        PhysicsComponent->PhysicsState.TotalMass,
        PhysicsComponent->CalculateStoppingDistance(),
        PhysicsComponent->PhysicsState.bIsWheelSlipping ? TEXT("YES") : TEXT("NO")
    );
    
    GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, DebugText);
}
```

---

## ❓ FAQ

**Q: Поезд слишком медленно разгоняется?**  
A: Увеличьте `MaxTractiveForce` или уменьшите `LocomotiveMass`/`WagonCount`

**Q: Поезд не может остановиться?**  
A: Увеличьте `MaxBrakingForce`

**Q: Колёса постоянно пробуксовывают?**  
A: Увеличьте `AdhesionCoefficient` (0.30-0.35) или уменьшите `MaxTractiveForce`

**Q: Поезд не чувствует подъёмы?**  
A: Убедитесь, что вы вызываете `SetTrackGrade()` с правильным углом

---

## 📚 Дополнительно

Полная документация: [TrainPhysicsSystem_RU.md](TrainPhysicsSystem_RU.md)

---

🚂 **Теперь у вас есть реалистичная физика поезда! Приятной разработки!** 🎉
