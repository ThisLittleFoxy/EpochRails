# Руководство по реализации системы взаимодействия

## Быстрый старт

Это пошаговое руководство поможет вам создать систему взаимодействия с поездом на чистых Blueprints.

---

## Шаг 1: Создание Blueprint Interface

### 1.1 Создайте папку для интерфейсов
```
Content/Interaction/Interfaces/
```

### 1.2 Создайте Blueprint Interface

1. ПКМ в Content Browser → Blueprint → Blueprint Interface
2. Назовите: `BP_InteractableInterface`
3. Откройте интерфейс

### 1.3 Добавьте функции

**Функция 1: OnInteract**
- Inputs: `Player` (Actor)
- Outputs: нет
- Описание: Вызывается при взаимодействии игрока

**Функция 2: GetInteractionPrompt**  
- Inputs: нет
- Outputs: `Prompt` (Text)
- Описание: Возвращает текст подсказки (например, "Нажмите E")

**Функция 3: CanInteract**
- Inputs: `Player` (Actor)
- Outputs: `bCanInteract` (Boolean)
- Описание: Проверяет, возможно ли взаимодействие

---

## Шаг 2: Создание компонента сидения (BP_TrainSeat)

### 2.1 Создайте Actor Component

1. Создайте папку: `Content/Interaction/Components/`
2. ПКМ → Blueprint → Blueprint Component
3. Parent Class: `Actor Component`
4. Назовите: `BP_TrainSeat`

### 2.2 Добавьте переменные

| Имя | Тип | Значение по умолчанию | Описание |
|-----|-----|----------------------|----------|
| `SeatLocation` | Scene Component | null | Позиция сидения |
| `IsOccupied` | Boolean | false | Занято ли место |
| `SeatedCharacter` | Actor | null | Ссылка на сидящего персонажа |
| `SittingAnimation` | Animation Montage | null | Анимация сидения |
| `SeatOffset` | Vector | (0,0,0) | Смещение персонажа |

### 2.3 Создайте функцию SitCharacter

**Function: SitCharacter**

Inputs:
- `Character` (Actor)

Логика:
```
1. Cast Character to ARailsPlayerCharacter
2. If Cast Successful:
   a. Store reference in SeatedCharacter
   b. Disable Character Movement
      - Get Character Movement Component
      - Set Movement Mode: None
   c. Attach Character to Component
      - Target: Character
      - Parent: Get Owner → Get Component By Class (Scene Component "SeatLocation")
      - Socket Name: None
      - Location Rule: Snap to Target
      - Rotation Rule: Snap to Target  
      - Scale Rule: Keep World
   d. Play Animation Montage
      - Target: Character → Get Mesh
      - Montage to Play: SittingAnimation
   e. Set IsOccupied = true
   f. Broadcast Event (optional): OnCharacterSeated
```

### 2.4 Создайте функцию ExitSeat

**Function: ExitSeat**

Логика:
```
1. If IsOccupied == true:
   a. Get SeatedCharacter
   b. Detach From Actor
      - Target: SeatedCharacter
      - Location Rule: Keep World
      - Rotation Rule: Keep World
      - Scale Rule: Keep World
   c. Enable Character Movement
      - Get Character Movement Component  
      - Set Movement Mode: Walking
   d. Set Character Location
      - Location: Get Owner Location + Vector(200, 0, 0) (рядом с поездом)
   e. Set SeatedCharacter = null
   f. Set IsOccupied = false
   g. Stop Animation Montage (optional)
   h. Broadcast Event (optional): OnCharacterExited
```

---

## Шаг 3: Создание компонента взаимодействия (BP_TrainInteraction)

### 3.1 Создайте Actor Component

1. ПКМ → Blueprint → Blueprint Component  
2. Parent Class: `Actor Component`
3. Назовите: `BP_TrainInteraction`
4. Class Settings → Interfaces → Add `BP_InteractableInterface`

### 3.2 Добавьте переменные

| Имя | Тип | Значение по умолчанию | Описание |
|-----|-----|----------------------|----------|
| `OwnerTrain` | Actor | null | Ссылка на поезд |
| `TrainSeatComponent` | BP_TrainSeat | null | Ссылка на компонент сидения |
| `InteractionPrompt` | Text | "Нажмите E чтобы войти" | Подсказка при входе |
| `ExitPrompt` | Text | "Нажмите E чтобы выйти" | Подсказка при выходе |
| `PlayerInside` | Boolean | false | Находится ли игрок внутри |
| `InteractionRange` | Float | 200.0 | Дистанция взаимодействия |

### 3.3 Реализуйте функцию OnInteract (из интерфейса)

**Event OnInteract**

Inputs:
- `Player` (Actor)

Логика:
```
1. Branch: PlayerInside?
   
   FALSE (игрок снаружи - войти в поезд):
   a. Get TrainSeatComponent
   b. Call SitCharacter(Player)
   c. Set PlayerInside = true
   d. Enable Train Controller
      - Get Owner → Get Component By Class (BP_TrainController)
      - Call EnableControl(Player)
   
   TRUE (игрок внутри - выйти из поезда):
   a. Get TrainSeatComponent  
   b. Call ExitSeat
   c. Set PlayerInside = false
   d. Disable Train Controller
      - Get Owner → Get Component By Class (BP_TrainController)
      - Call DisableControl
```

### 3.4 Реализуйте функцию GetInteractionPrompt (из интерфейса)

**Function GetInteractionPrompt**

Outputs:
- `Prompt` (Text)

Логика:
```
1. Branch: PlayerInside?
   - TRUE: Return ExitPrompt
   - FALSE: Return InteractionPrompt
```

### 3.5 Реализуйте функцию CanInteract (из интерфейса)

**Function CanInteract**

Inputs:
- `Player` (Actor)

Outputs:  
- `bCanInteract` (Boolean)

Логика:
```
1. Get Player Location
2. Get Owner (Train) Location  
3. Calculate Distance: Vector Length (PlayerLoc - TrainLoc)
4. If Distance <= InteractionRange:
   a. If PlayerInside == false:
      - Check if TrainSeatComponent->IsOccupied == false
      - Return true if seat not occupied
   b. If PlayerInside == true:
      - Return true (can always exit)
5. Else:
   - Return false (too far)
```

---

## Шаг 4: Создание компонента управления поездом (BP_TrainController)

### 4.1 Создайте Actor Component

1. ПКМ → Blueprint → Blueprint Component
2. Parent Class: `Actor Component`  
3. Назовите: `BP_TrainController`

### 4.2 Добавьте переменные

| Имя | Тип | Значение по умолчанию | Описание |
|-----|-----|----------------------|----------|
| `OwnerTrain` | Actor | null | Ссылка на поезд |
| `ControllingPlayer` | Actor | null | Игрок, управляющий поездом |
| `ForwardSpeed` | Float | 500.0 | Скорость вперед |
| `BackwardSpeed` | Float | -300.0 | Скорость назад |
| `IsControlEnabled` | Boolean | false | Включено ли управление |

### 4.3 Создайте функцию EnableControl

**Function: EnableControl**

Inputs:
- `Player` (Actor)

Логика:
```
1. Set ControllingPlayer = Player
2. Set IsControlEnabled = true
3. Get Player Controller
   - Cast Player to Pawn
   - Get Controller
4. Enable Input
   - Target: Self (BP_TrainController)
   - Player Controller: From step 3
5. Print String: "Train control enabled" (debug)
```

### 4.4 Создайте функцию DisableControl

**Function: DisableControl**

Логика:
```
1. If IsControlEnabled:
   a. Stop Train (call StopTrain function)
   b. Disable Input
      - Target: Self
      - Player Controller: Get Player Controller
   c. Set ControllingPlayer = null
   d. Set IsControlEnabled = false
   e. Print String: "Train control disabled" (debug)
```

### 4.5 Создайте функции движения

**Function: MoveForward**

```
1. If IsControlEnabled:
   a. Get OwnerTrain
   b. Cast to BP_Train (or your train class)
   c. Set Train Speed = ForwardSpeed
      - Call SetCurrentSpeed(ForwardSpeed) on train
```

**Function: MoveBackward**

```
1. If IsControlEnabled:
   a. Get OwnerTrain  
   b. Cast to BP_Train
   c. Set Train Speed = BackwardSpeed
```

**Function: StopTrain**

```
1. Get OwnerTrain
2. Cast to BP_Train  
3. Set Train Speed = 0.0
```

### 4.6 Настройте Input Actions

В Event Graph компонента:

**Input Action: TrainForward (W)**
```
Event InputAction TrainForward:
- Pressed: Call MoveForward
- Released: Call StopTrain
```

**Input Action: TrainBackward (S)**  
```
Event InputAction TrainBackward:
- Pressed: Call MoveBackward
- Released: Call StopTrain
```

**Input Action: TrainStop (Space)**
```
Event InputAction TrainStop:
- Pressed: Call StopTrain
```

---

## Шаг 5: Настройка Blueprint поезда (BP_Train)

### 5.1 Откройте ваш Blueprint поезда

Например: `Content/Train/BP_Train/BP_MyTrain`

### 5.2 Добавьте компоненты

1. **Add Component** → `BP_TrainSeat`
   - Назовите: `TrainSeat`
   
2. **Add Component** → `BP_TrainInteraction`
   - Назовите: `Interaction`
   
3. **Add Component** → `BP_TrainController`  
   - Назовите: `Controller`

4. **Add Component** → `Scene` (обычный Scene Component)
   - Назовите: `DriverSeatLocation`
   - Переместите его в позицию водительского сидения
   - Сделайте его дочерним элементом меша кабины поезда

### 5.3 Настройте ссылки в Construction Script

```
Event Construction Script:

1. Get TrainSeat Component
   - Set SeatLocation = DriverSeatLocation (перетащите из Components)
   
2. Get Interaction Component
   - Set OwnerTrain = Self
   - Set TrainSeatComponent = TrainSeat Component
   
3. Get Controller Component
   - Set OwnerTrain = Self
```

### 5.4 (Опционально) Добавьте визуальную обратную связь

В Event Graph:

```
Event BeginPlay:
1. Get Interaction Component
2. Bind Event to OnCharacterSeated
   - When character sits: 
     a. Change seat mesh color (SetVectorParameterValue)
     b. Play sound
     
3. Bind Event to OnCharacterExited  
   - When character exits:
     a. Reset seat mesh color
     b. Play sound
```

---

## Шаг 6: Настройка персонажа игрока (BP_PlayerCharacter)

### 6.1 Откройте Blueprint персонажа

Например: `Content/Characters/BP_Characters/BP_ThirdPersonCharacter`

### 6.2 Добавьте переменные

| Имя | Тип | Значение по умолчанию | Описание |
|-----|-----|----------------------|----------|
| `InteractableActor` | Actor | null | Текущий интерактивный объект |
| `InteractionRange` | Float | 250.0 | Дистанция поиска |
| `InteractionTraceChannel` | Trace Channel | Visibility | Канал трассировки |

### 6.3 Создайте функцию поиска интерактивных объектов

**Function: FindInteractable**

```
1. Get Actor Location (Self)
2. Get Actor Forward Vector
3. Calculate End Location:
   - Start + (Forward * InteractionRange)
   
4. Sphere Trace By Channel:
   - Start: Start Location  
   - End: End Location
   - Radius: 50.0
   - Trace Channel: InteractionTraceChannel
   - Trace Complex: false
   - Actors to Ignore: Self
   
5. Branch: Hit?
   TRUE:
   a. Get Hit Actor
   b. Does Implement Interface: BP_InteractableInterface?
      - YES: 
        * Set InteractableActor = Hit Actor
        * Call CanInteract on Hit Actor (pass Self)
        * If CanInteract returns true:
          - Show interaction prompt on screen
      - NO:
        * Set InteractableActor = null
   FALSE:
   a. Set InteractableActor = null
   b. Hide interaction prompt
```

### 6.4 Вызывайте FindInteractable регулярно

Вариант 1 - Custom Event + Timer:
```
Event BeginPlay:
- Set Timer by Event
  - Event: FindInteractable
  - Time: 0.1 (10 раз в секунду)
  - Looping: true
```

Вариант 2 - Event Tick:
```
Event Tick:
- Call FindInteractable
```

### 6.5 Создайте Input Action для взаимодействия

**Event InputAction Interact (E key)**

```
Event InputAction Interact (Pressed):

1. Is Valid: InteractableActor?
   TRUE:
   a. Call Interface: BP_InteractableInterface->CanInteract
      - Target: InteractableActor
      - Player: Self
   b. Branch: Can Interact?
      TRUE:
      - Call Interface: BP_InteractableInterface->OnInteract
        * Target: InteractableActor  
        * Player: Self
      FALSE:
      - Print String: "Cannot interact" (debug)
   FALSE:
   - Print String: "No interactable object" (debug)
```

### 6.6 (Опционально) Добавьте UI для подсказки

Создайте простой Widget:

1. Create Widget Blueprint: `WBP_InteractionPrompt`
2. Add Text Block: `PromptText`
3. Bind text to function that calls:
   ```
   Get InteractableActor → Call Interface GetInteractionPrompt
   ```

В персонаже:
```
Event BeginPlay:
- Create Widget: WBP_InteractionPrompt
- Add to Viewport
- Set Visibility: Hidden

В FindInteractable:
- When InteractableActor is found: Set Visibility to Visible
- When not found: Set Visibility to Hidden  
```

---

## Шаг 7: Настройка Input в Project Settings

### 7.1 Откройте Project Settings

Edit → Project Settings → Input

### 7.2 Добавьте Action Mappings

**Action Mappings:**

| Name | Key | Shift | Ctrl | Alt | Cmd |
|------|-----|-------|------|-----|-----|
| Interact | E | ❌ | ❌ | ❌ | ❌ |
| TrainForward | W | ❌ | ❌ | ❌ | ❌ |
| TrainBackward | S | ❌ | ❌ | ❌ | ❌ |
| TrainStop | Space | ❌ | ❌ | ❌ | ❌ |

*(Если W и S уже используются для движения персонажа, они будут работать в обоих контекстах)*

---

## Шаг 8: Тестирование

### 8.1 Разместите поезд на уровне

1. Откройте ваш уровень
2. Перетащите BP_Train на уровень
3. Убедитесь, что все компоненты добавлены:
   - TrainSeat
   - Interaction  
   - Controller
   - DriverSeatLocation

### 8.2 Проверьте персонажа

1. Убедитесь, что BP_PlayerCharacter содержит логику FindInteractable
2. Input Action "Interact" настроен правильно

### 8.3 Тестируйте функциональность

**Чек-лист тестирования:**

- [ ] Подойдите к поезду
- [ ] Появляется ли подсказка "Нажмите E чтобы войти"?
- [ ] Нажмите E
- [ ] Персонаж прикрепляется к сидению?
- [ ] Отключается ли движение персонажа?
- [ ] Нажмите W - поезд движется вперед?
- [ ] Нажмите S - поезд движется назад?
- [ ] Нажмите Space - поезд останавливается?
- [ ] Нажмите E снова - персонаж выходит?
- [ ] Персонаж спавнится рядом с поездом?
- [ ] Управление поездом отключено после выхода?

---

## Решение проблем

### Проблема: Подсказка не появляется

**Решение:**
1. Проверьте, что BP_Train реализует BP_InteractableInterface через компонент
2. Убедитесь, что Sphere Trace в FindInteractable достигает поезда
3. Включите Draw Debug для Sphere Trace (установите Draw Debug Type: For Duration)
4. Проверьте InteractionRange (увеличьте значение для теста)

### Проблема: Персонаж не прикрепляется к сидению

**Решение:**  
1. Убедитесь, что DriverSeatLocation создан и правильно позиционирован
2. В BP_TrainSeat проверьте, что SeatLocation ссылается на DriverSeatLocation
3. Проверьте правила AttachToComponent (должно быть Snap to Target)
4. Проверьте, что Cast to ARailsPlayerCharacter успешен

### Проблема: Управление поездом не работает

**Решение:**
1. Проверьте, что EnableControl вызывается при посадке
2. Убедитесь, что Input Actions настроены в Project Settings
3. Проверьте Enable Input в BP_TrainController (должен получить Player Controller)
4. Debug: добавьте Print String в функции MoveForward/Backward

### Проблема: Персонаж застревает в поезде

**Решение:**
1. Проверьте DetachFromActor в ExitSeat
2. Убедитесь, что Character Movement Mode возвращается в Walking
3. Проверьте spawn location при выходе (не внутри коллизии)
4. Добавьте отступ: `GetActorLocation + GetActorForwardVector * 200`

### Проблема: Анимация не проигрывается

**Решение:**
1. Проверьте, что SittingAnimation установлена в BP_TrainSeat
2. Убедитесь, что анимация совместима со скелетом персонажа
3. Проверьте, что Play Animation Montage вызывается на Skeletal Mesh компоненте
4. Используйте Animation Blueprint если нужна более сложная логика

---

## Улучшения и расширения

### 1. Плавная камера

```
В BP_Train:
- Add Component: Camera
- Position: Behind driver seat

В BP_TrainSeat->SitCharacter:
- Get Controlling Player
- Set View Target with Blend
  - Target: Train Camera
  - Blend Time: 1.0
  
В BP_TrainSeat->ExitSeat:
- Set View Target with Blend  
  - Target: Player Character
  - Blend Time: 1.0
```

### 2. Highlight эффект на поезде

```
В BP_Train Event Graph:

Custom Event: EnableHighlight
- Get Train Mesh
- Set Render Custom Depth: true
- Set Custom Depth Stencil Value: 1

Custom Event: DisableHighlight  
- Get Train Mesh
- Set Render Custom Depth: false

В BP_PlayerCharacter->FindInteractable:
- When InteractableActor found: Call EnableHighlight
- When not found: Call DisableHighlight

В Post Process Volume:
- Enable Custom Depth-Stencil Pass
- Add outline material using Custom Depth
```

### 3. Двери поезда

```
Создайте новый компонент: BP_TrainDoor

Variables:
- DoorMesh (Static Mesh Component)
- IsOpen (Boolean)
- OpenRotation (Rotator) - например (0, 90, 0)
- ClosedRotation (Rotator) - (0, 0, 0)  
- AnimationDuration (Float) - 1.0

Functions:
- OpenDoor: Timeline animation to OpenRotation
- CloseDoor: Timeline animation to ClosedRotation

В BP_TrainInteraction->OnInteract:
- Before SitCharacter: Check if door is open
- If closed: Call OpenDoor, wait, then SitCharacter
```

### 4. UI панель управления

```
Создайте Widget: WBP_TrainDashboard

Elements:
- Speed Text
- Direction indicator  
- Distance traveled
- Control buttons (optional)

В BP_TrainController->EnableControl:
- Create Widget: WBP_TrainDashboard
- Add to Viewport

Event Tick in Widget:
- Get Owning Pawn → Cast to BP_Train
- Update Speed Text from train's current speed
```

---

## Итоговая структура файлов

```
Content/
├── Interaction/
│   ├── Interfaces/
│   │   └── BP_InteractableInterface.uasset
│   ├── Components/
│   │   ├── BP_TrainSeat.uasset  
│   │   ├── BP_TrainInteraction.uasset
│   │   └── BP_TrainController.uasset
│   └── Widgets/
│       ├── WBP_InteractionPrompt.uasset
│       └── WBP_TrainDashboard.uasset (опционально)
├── Train/
│   └── BP_Train/
│       └── BP_MyTrain.uasset (обновлен)
└── Characters/
    └── BP_Characters/
        └── BP_ThirdPersonCharacter.uasset (обновлен)
```

---

## Дополнительные ресурсы

### Рекомендуемые анимации

- Sitting Idle
- Sit Down Transition  
- Stand Up Transition
- Driving Idle (руки на руле)

### Рекомендуемые звуки

- Door open/close
- Seat adjustment  
- Train engine start/stop
- Button press sounds

---

## Поддержка и вопросы

Если у вас возникли проблемы:

1. Проверьте все шаги в чек-листе
2. Используйте Print String для отладки
3. Включите Draw Debug для визуализации трассировок
4. Проверьте Output Log на наличие ошибок

---

**Версия документа:** 1.0  
**Дата создания:** 2025-12-03  
**Совместимость:** Unreal Engine 5.x  
**Язык:** Русский
