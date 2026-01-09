# Blueprint API Reference для ARailsPlayerCharacter

## 📚 Полный справочник доступных переменных, функций и компонентов

Этот документ описывает **ВСЁ**, что доступно в Blueprint при создании класса на основе `ARailsPlayerCharacter`.

---

## 📦 Компоненты (Components)

### 🎥 CameraBoom (USpringArmComponent)
```
Type: USpringArmComponent*
Access: BlueprintReadOnly (через геттер)
Category: Components
```

**Описание**: Spring Arm компонент, позиционирующий камеру за персонажем.

**Как получить**:
```
Get Camera Boom
└─ Return: USpringArmComponent*
```

**Доступные настройки** (в Details panel):
- `Target Arm Length` - Длина "руки" (дистанция камеры)
- `Socket Offset` - Смещение точки крепления
- `Target Offset` - Смещение цели
- `Use Pawn Control Rotation` - Следовать за поворотом контроллера
- `Enable Camera Lag` - Плавное следование камеры
- `Camera Lag Speed` - Скорость сглаживания

**Частые функции**:
- `Set Target Arm Length` - Изменить дистанцию камеры
- `Set Relative Rotation` - Повернуть камеру

---

### 📷 FollowCamera (UCameraComponent)
```
Type: UCameraComponent*
Access: BlueprintReadOnly (через геттер)
Category: Components
```

**Описание**: Компонент камеры, которая следует за персонажем.

**Как получить**:
```
Get Follow Camera
└─ Return: UCameraComponent*
```

**Доступные настройки**:
- `Field of View (FOV)` - Угол обзора (90 = default)
- `Aspect Ratio` - Соотношение сторон
- `Post Process Settings` - Пост-обработка (цвет, bloom, etc)

**Частые функции**:
- `Set Field Of View` - Изменить FOV (для zoom, бег, etc)
- `Get Camera Location` - Получить позицию камеры (для трейсов)
- `Get Forward Vector` - Направление взгляда камеры

---

### 🔍 InteractionComponent (UInteractionComponent)
```
Type: UInteractionComponent*
Access: BlueprintReadOnly (через геттер)
Category: Components
```

**Описание**: Компонент для обнаружения и взаимодействия с объектами.

**Как получить**:
```
Get Interaction Component
└─ Return: UInteractionComponent*
```

**Доступные функции**:
```
Try Interact
├─ Return: bool (успешно ли?)

Get Focused Actor
├─ Return: AActor* (объект в фокусе)

Has Focused Actor
├─ Return: bool (есть ли объект в фокусе?)

Get Focused Actor Name
├─ Return: FText (имя объекта)

Get Focused Actor Action
├─ Return: FText (действие: "Открыть", "Взять", etc)

Can Interact With Focused Actor
├─ Return: bool (можно ли взаимодействовать?)

Press Widget Interaction
└─ Нажать на UI виджет (для кнопок в мире)

Release Widget Interaction
└─ Отпустить UI виджет

Is Hovering Widget
├─ Return: bool (навёл ли курсор на виджет?)
```

**Переменные** (настройки):
- `Default Interaction Distance` - Дистанция взаимодействия (300 см default)
- `Interaction Check Frequency` - Как часто проверять (0.1 сек)
- `Interaction Trace Channel` - Канал коллизии для трейса
- `bShow Debug Trace` - Показывать ли debug линии
- `bEnable Widget Interaction` - Включить взаимодействие с UI

---

### 🏃 CharacterMovement (UCharacterMovementComponent)
```
Type: UCharacterMovementComponent*
Access: Inherited from ACharacter
Category: Movement
```

**Описание**: Компонент движения персонажа (встроенный в ACharacter).

**Как получить**:
```
Get Character Movement
└─ Return: UCharacterMovementComponent*
```

**Важные переменные**:

#### Movement Speed:
- `Max Walk Speed` (float) - Максимальная скорость ходьбы (500 default)
- `Max Walk Speed Crouched` (float) - Скорость приседания (300)
- `Max Swim Speed` (float) - Скорость плавания
- `Max Fly Speed` (float) - Скорость полёта

#### Jump Settings:
- `Jump Z Velocity` (float) - Сила прыжка (420 default)
- `Air Control` (float) - Контроль в воздухе (0.2 = 20%)
- `Gravity Scale` (float) - Множитель гравитации (1.0)

#### Rotation:
- `Rotation Rate` (FRotator) - Скорость поворота
- `bOrient Rotation To Movement` (bool) - Поворачиваться в сторону движения
- `bUse Controller Desired Rotation` (bool) - Использовать желаемый поворот контроллера

#### Ground Movement:
- `Ground Friction` (float) - Трение об землю
- `Braking Deceleration Walking` (float) - Замедление при остановке
- `Max Acceleration` (float) - Максимальное ускорение

**Частые функции**:
```
Set Max Walk Speed
├─ New Max Walk Speed: float
└─ Изменить скорость (для спринта, например)

Is Falling
├─ Return: bool
└─ Проверить, в воздухе ли персонаж

Is Moving On Ground
├─ Return: bool
└─ Двигается ли по земле

Is Swimming
├─ Return: bool

Get Velocity
├─ Return: FVector
└─ Получить скорость движения

Stop Active Movement
└─ Остановить движение

Launch Character
├─ Launch Velocity: FVector
├─ Override XY: bool
├─ Override Z: bool
└─ "Толкнуть" персонажа (для dash, knockback)
```

---

### 🗿 Capsule Component (UCapsuleComponent)
```
Type: UCapsuleComponent*
Access: Inherited from ACharacter
Category: Collision
```

**Описание**: Коллизия персонажа (капсула).

**Как получить**:
```
Get Capsule Component
└─ Return: UCapsuleComponent*
```

**Важные переменные**:
- `Capsule Half Height` (float) - Половина высоты капсулы
- `Capsule Radius` (float) - Радиус капсулы

**Частые функции**:
```
Set Capsule Size
├─ New Radius: float
├─ New Half Height: float
└─ Изменить размер (для приседания)

Get Scaled Capsule Size
├─ Out Radius: float (out)
├─ Out Half Height: float (out)
└─ Получить текущий размер
```

---

### 🦴 Skeletal Mesh (USkeletalMeshComponent)
```
Type: USkeletalMeshComponent*
Access: Inherited from ACharacter
Category: Mesh
```

**Описание**: Скелетный меш персонажа.

**Как получить**:
```
Get Mesh
└─ Return: USkeletalMeshComponent*
```

**Важные функции**:
```
Play Animation (не рекомендуется, используйте Animation Blueprint)
├─ Anim to Play: UAnimationAsset*

Get Socket Location
├─ Socket Name: FName
├─ Return: FVector
└─ Получить позицию сокета (для эффектов, оружия)

Get Socket Rotation
├─ Socket Name: FName
├─ Return: FRotator

Get Socket Transform
├─ Socket Name: FName
├─ Return: FTransform

Does Socket Exist
├─ Socket Name: FName
├─ Return: bool
```

---

## 📊 Переменные ARailsPlayerCharacter

### 🎥 Camera Settings (Protected - доступны в BP)

#### CameraSocketName
```
Type: FName
Access: BlueprintReadWrite
Default: None (пусто)
Category: Camera
```
**Описание**: Имя сокета на меше, к которому привязать камеру. Оставьте пустым для привязки к root.

**Пример**: `"CameraSocket"`, `"HeadSocket"`

---

#### bAttachCameraToSocket
```
Type: bool
Access: BlueprintReadWrite
Default: false
Category: Camera
```
**Описание**: Привязывать ли камеру к сокету на меше (вместо root компонента).

**Когда использовать**: Для камеры от первого лица, или специальной камеры на транспорте.

---

#### CameraRelativeLocationOffset
```
Type: FVector
Access: BlueprintReadWrite
Default: (0, 0, 0)
Category: Camera
```
**Описание**: Смещение позиции камеры относительно точки крепления.

**Пример**: `(0, 0, 60)` - поднять камеру на 60 см вверх

---

#### CameraRelativeRotationOffset
```
Type: FRotator
Access: BlueprintReadWrite
Default: (0, 0, 0)
Category: Camera
```
**Описание**: Смещение поворота камеры относительно точки крепления.

**Пример**: `(0, 90, 0)` - повернуть камеру на 90° по Yaw

---

#### bIgnoreSocketRotation
```
Type: bool
Access: BlueprintReadWrite
Default: true
Category: Camera
```
**Описание**: Игнорировать ли вращение сокета (использовать world/pawn rotation).

**true** = камера не вращается вместе с анимациями меша
**false** = камера следует за поворотом сокета (может качаться при ходьбе)

---

### 🏃 Movement Settings (Protected)

#### WalkSpeed
```
Type: float
Access: BlueprintReadWrite
Default: 500.0
Category: Movement|Sprint
```
**Описание**: Обычная скорость ходьбы (см/сек).

**Диапазон**: 0 - 1000+ (обычно 300-600)

---

#### SprintSpeed
```
Type: float
Access: BlueprintReadWrite
Default: 800.0
Category: Movement|Sprint
```
**Описание**: Скорость спринта (см/сек).

**Диапазон**: 0 - 1500+ (обычно 600-1000)

---

### 🎮 Input Actions (Protected)

Все Input Actions имеют тип `UInputAction*` и категорию `Input`.

#### JumpAction
```
Type: UInputAction*
Access: EditAnywhere
Category: Input
```
**Описание**: Enhanced Input Action для прыжка.

**Привязать к**: IA_Jump в Content Browser

---

#### MoveAction
```
Type: UInputAction*
Access: EditAnywhere
Category: Input
```
**Описание**: Enhanced Input Action для движения (WASD).

**Привязать к**: IA_Move (Vector2D)

---

#### LookAction
```
Type: UInputAction*
Access: EditAnywhere
Category: Input
```
**Описание**: Enhanced Input Action для взгляда геймпадом.

**Привязать к**: IA_Look (Vector2D)

---

#### MouseLookAction
```
Type: UInputAction*
Access: EditAnywhere
Category: Input
```
**Описание**: Enhanced Input Action для взгляда мышью.

**Привязать к**: IA_MouseLook (Vector2D)

---

#### SprintAction
```
Type: UInputAction*
Access: EditAnywhere
Category: Input
```
**Описание**: Enhanced Input Action для спринта.

**Привязать к**: IA_Sprint

---

#### InteractAction
```
Type: UInputAction*
Access: EditAnywhere
Category: Input
```
**Описание**: Enhanced Input Action для взаимодействия (E).

**Привязать к**: IA_Interact

---

#### FireAction
```
Type: UInputAction*
Access: BlueprintReadOnly
Category: Input
```
**Описание**: Enhanced Input Action для стрельбы / UI клика.

**Привязать к**: IA_Fire

---

#### ThrottleAction
```
Type: UInputAction*
Access: BlueprintReadOnly
Category: Input|Train
```
**Описание**: Enhanced Input Action для газа поезда.

**Привязать к**: IA_Throttle (Axis1D)

---

#### BrakeAction
```
Type: UInputAction*
Access: BlueprintReadOnly
Category: Input|Train
```
**Описание**: Enhanced Input Action для тормоза поезда.

**Привязать к**: IA_Brake (Axis1D)

---

### 🚂 Train Control (Protected/Public)

#### ControlledTrain
```
Type: ARailsTrain*
Access: BlueprintReadOnly
Default: nullptr
Category: Train
```
**Описание**: Ссылка на поезд, которым управляет персонаж.

**nullptr** = не управляет поездом
**Valid** = сейчас в кабине машиниста

---

### 🎬 Animation Variables (Public - для Animation Blueprint)

#### bIsSprinting
```
Type: bool
Access: BlueprintReadOnly
Default: false
Category: Movement|Animation
```
**Описание**: Спринтит ли персонаж (для анимаций).

**Использование в AnimBP**: Для переключения между Walk/Run/Sprint анимациями.

---

#### CurrentSpeed
```
Type: float
Access: BlueprintReadOnly
Default: 0.0
Category: Movement|Animation
```
**Описание**: Текущая скорость персонажа в см/сек.

**Использование в AnimBP**: Для Blend Space (Idle → Walk → Run).

**Диапазон**: 0 (стоит) - 800+ (бежит)

---

#### MovementDirection
```
Type: float
Access: BlueprintReadOnly
Default: 0.0
Category: Movement|Animation
```
**Описание**: Направление движения относительно поворота персонажа (-180 до 180).

**Использование в AnimBP**: Для Directional Blend Space (вперёд/назад/вбок).

**Значения**:
- `0°` = движение вперёд
- `90°` = движение вправо
- `-90°` = движение влево
- `180°` или `-180°` = движение назад

---

#### bIsInAir
```
Type: bool
Access: BlueprintReadOnly
Default: false
Category: Movement|Animation
```
**Описание**: Находится ли персонаж в воздухе (прыжок/падение).

**Использование в AnimBP**: Для переключения на анимацию прыжка/падения.

---

## 🔧 Функции ARailsPlayerCharacter

### 🎥 Camera Functions

#### SetCameraSocket
```
UFUNCTION(BlueprintCallable, Category = "Camera")
void SetCameraSocket(FName NewSocketName, bool bIgnoreRotation = true)
```
**Описание**: Динамически изменить сокет камеры в рантайме.

**Параметры**:
- `NewSocketName` - Имя нового сокета
- `bIgnoreRotation` - Игнорировать вращение сокета (default: true)

**Пример использования**:
```
Событие: Сесть в машину
└─► Set Camera Socket
    ├─ New Socket Name: "DriverCameraSocket"
    └─ bIgnore Rotation: true
```

---

#### ResetCameraToDefault
```
UFUNCTION(BlueprintCallable, Category = "Camera")
void ResetCameraToDefault()
```
**Описание**: Сбросить камеру к дефолтной привязке (root компонент).

**Пример использования**:
```
Событие: Выйти из машины
└─► Reset Camera To Default
```

---

### 🏃 Movement Functions

#### DoStartSprint
```
UFUNCTION(BlueprintCallable, Category = "Movement")
void DoStartSprint()
```
**Описание**: Начать спринт.

**Что делает**:
1. Устанавливает `bIsSprinting = true`
2. Меняет `MaxWalkSpeed` на `SprintSpeed`

**Пример**:
```
Input: Sprint Pressed
└─► Do Start Sprint
```

---

#### DoStopSprint
```
UFUNCTION(BlueprintCallable, Category = "Movement")
void DoStopSprint()
```
**Описание**: Остановить спринт.

**Что делает**:
1. Устанавливает `bIsSprinting = false`
2. Возвращает `MaxWalkSpeed` на `WalkSpeed`

**Пример**:
```
Input: Sprint Released
└─► Do Stop Sprint
```

---

### 🎮 Input Handler Functions (можно вызывать из UI/других систем)

#### DoMove
```
UFUNCTION(BlueprintCallable, Category = "Input")
virtual void DoMove(float Right, float Forward)
```
**Описание**: Обработать движение персонажа.

**Параметры**:
- `Right` - Движение вправо/влево (-1.0 до 1.0)
- `Forward` - Движение вперёд/назад (-1.0 до 1.0)

**Пример**:
```
Mobile Joystick Output
├─ X: Right (float)
├─ Y: Forward (float)
└─► Do Move (Right, Forward)
```

---

#### DoLook
```
UFUNCTION(BlueprintCallable, Category = "Input")
virtual void DoLook(float Yaw, float Pitch)
```
**Описание**: Обработать поворот камеры.

**Параметры**:
- `Yaw` - Поворот по горизонтали
- `Pitch` - Поворот по вертикали

**Пример**:
```
Mobile Look Joystick
├─ X: Yaw
├─ Y: Pitch
└─► Do Look (Yaw, Pitch)
```

---

#### DoJumpStart
```
UFUNCTION(BlueprintCallable, Category = "Input")
virtual void DoJumpStart()
```
**Описание**: Начать прыжок.

**Что делает**: Вызывает `Jump()` из ACharacter.

**Пример**:
```
Mobile Button: Jump Pressed
└─► Do Jump Start
```

---

#### DoJumpEnd
```
UFUNCTION(BlueprintCallable, Category = "Input")
virtual void DoJumpEnd()
```
**Описание**: Остановить прыжок (отпустили кнопку).

**Что делает**: Вызывает `StopJumping()` из ACharacter.

**Пример**:
```
Mobile Button: Jump Released
└─► Do Jump End
```

---

#### DoInteract
```
UFUNCTION(BlueprintCallable, Category = "Interaction")
virtual void DoInteract()
```
**Описание**: Попытаться взаимодействовать с объектом в фокусе.

**Что делает**: Вызывает `InteractionComponent->TryInteract()`.

**Пример**:
```
Input: E Pressed
└─► Do Interact
```

---

### 🚂 Train Functions

#### SetControlledTrain
```
UFUNCTION(BlueprintCallable, Category = "Train")
void SetControlledTrain(ARailsTrain* Train)
```
**Описание**: Установить поезд, которым управляет персонаж.

**Параметры**:
- `Train` - Ссылка на поезд (или nullptr для выхода)

**Пример**:
```
Событие: Войти в кабину
└─► Set Controlled Train
    └─ Train: RailsTrain Reference
```

---

#### GetControlledTrain
```
UFUNCTION(BlueprintPure, Category = "Train")
ARailsTrain* GetControlledTrain() const
```
**Описание**: Получить поезд, которым управляет персонаж.

**Return**: ARailsTrain* (или nullptr, если не управляет)

**Пример**:
```
Branch
├─ Condition: Is Valid (Get Controlled Train)
├─ True: Управляет поездом
└─ False: Не управляет
```

---

### 🔫 Weapon Functions

#### Fire
```
UFUNCTION(BlueprintCallable, Category = "Weapon")
void Fire()
```
**Описание**: Выстрелить из оружия (или активировать основное действие).

**Пример**:
```
Input: Fire Pressed
└─► Fire
```

---

### 📍 Getter Functions (FORCEINLINE - очень быстрые)

#### GetCameraBoom
```
FORCEINLINE USpringArmComponent* GetCameraBoom() const
```
**Описание**: Получить компонент Spring Arm.

---

#### GetFollowCamera
```
FORCEINLINE UCameraComponent* GetFollowCamera() const
```
**Описание**: Получить компонент камеры.

---

#### GetInteractionComponent
```
FORCEINLINE UInteractionComponent* GetInteractionComponent() const
```
**Описание**: Получить компонент взаимодействия.

---

#### IsSprinting
```
FORCEINLINE bool IsSprinting() const
```
**Описание**: Проверить, спринтит ли персонаж.

**Return**: bool

---

#### GetCurrentSpeed
```
FORCEINLINE float GetCurrentSpeed() const
```
**Описание**: Получить текущую скорость.

**Return**: float (см/сек)

---

#### GetMovementDirection
```
FORCEINLINE float GetMovementDirection() const
```
**Описание**: Получить направление движения.

**Return**: float (градусы, -180 до 180)

---

#### IsInAir
```
FORCEINLINE bool IsInAir() const
```
**Описание**: Проверить, в воздухе ли персонаж.

**Return**: bool

---

## 🎭 Наследуемые функции от ACharacter

### Jump & Movement

#### Jump
```
UFUNCTION(BlueprintCallable, Category = "Character")
void Jump()
```
**Описание**: Прыгнуть (если на земле).

---

#### StopJumping
```
UFUNCTION(BlueprintCallable, Category = "Character")
void StopJumping()
```
**Описание**: Остановить прыжок (отпустить кнопку).

---

#### Crouch
```
UFUNCTION(BlueprintCallable, Category = "Character")
void Crouch()
```
**Описание**: Присесть.

---

#### UnCrouch
```
UFUNCTION(BlueprintCallable, Category = "Character")
void UnCrouch()
```
**Описание**: Встать из приседа.

---

#### CanJump
```
UFUNCTION(BlueprintCallable, Category = "Character")
bool CanJump() const
```
**Описание**: Может ли персонаж прыгнуть сейчас.

**Return**: bool

---

#### GetBaseAimRotation
```
UFUNCTION(BlueprintCallable, Category = "Pawn")
FRotator GetBaseAimRotation() const
```
**Описание**: Получить направление прицеливания контроллера.

**Return**: FRotator

---

## 🎮 Наследуемые функции от APawn

#### AddMovementInput
```
UFUNCTION(BlueprintCallable, Category = "Pawn")
void AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce)
```
**Описание**: Добавить вектор движения.

**Параметры**:
- `WorldDirection` - Направление в мировых координатах
- `ScaleValue` - Множитель силы (-1.0 до 1.0)
- `bForce` - Принудительное движение (игнорирует контроллер)

---

#### AddControllerYawInput
```
UFUNCTION(BlueprintCallable, Category = "Pawn")
void AddControllerYawInput(float Val)
```
**Описание**: Повернуть камеру по горизонтали (Yaw).

**Параметры**:
- `Val` - Значение поворота

---

#### AddControllerPitchInput
```
UFUNCTION(BlueprintCallable, Category = "Pawn")
void AddControllerPitchInput(float Val)
```
**Описание**: Повернуть камеру по вертикали (Pitch).

**Параметры**:
- `Val` - Значение поворота

---

#### GetController
```
UFUNCTION(BlueprintCallable, Category = "Pawn")
AController* GetController() const
```
**Описание**: Получить контроллер, управляющий этим Pawn.

**Return**: AController* (APlayerController* для игрока)

---

#### IsPlayerControlled
```
UFUNCTION(BlueprintPure, Category = "Pawn")
bool IsPlayerControlled() const
```
**Описание**: Управляется ли игроком (или AI).

**Return**: bool

---

## 🌍 Наследуемые функции от AActor

### Transform

#### GetActorLocation
```
UFUNCTION(BlueprintCallable, Category = "Actor")
FVector GetActorLocation() const
```
**Описание**: Получить позицию актора в мире.

---

#### GetActorRotation
```
UFUNCTION(BlueprintCallable, Category = "Actor")
FRotator GetActorRotation() const
```
**Описание**: Получить поворот актора.

---

#### GetActorScale3D
```
UFUNCTION(BlueprintCallable, Category = "Actor")
FVector GetActorScale3D() const
```
**Описание**: Получить масштаб актора.

---

#### SetActorLocation
```
UFUNCTION(BlueprintCallable, Category = "Actor")
bool SetActorLocation(FVector NewLocation, bool bSweep, FHitResult& SweepHitResult, bool bTeleport)
```
**Описание**: Установить позицию актора.

**Параметры**:
- `NewLocation` - Новая позиция
- `bSweep` - Проверять коллизию при перемещении
- `SweepHitResult` - Результат коллизии (если была)
- `bTeleport` - Телепортировать (игнорирует физику)

---

#### SetActorRotation
```
UFUNCTION(BlueprintCallable, Category = "Actor")
bool SetActorRotation(FRotator NewRotation, bool bTeleportPhysics)
```
**Описание**: Установить поворот актора.

---

#### GetActorForwardVector
```
UFUNCTION(BlueprintCallable, Category = "Actor")
FVector GetActorForwardVector() const
```
**Описание**: Получить вектор "вперёд" актора.

**Return**: FVector (normalized, длина = 1)

---

#### GetActorRightVector
```
UFUNCTION(BlueprintCallable, Category = "Actor")
FVector GetActorRightVector() const
```
**Описание**: Получить вектор "вправо" актора.

---

#### GetActorUpVector
```
UFUNCTION(BlueprintCallable, Category = "Actor")
FVector GetActorUpVector() const
```
**Описание**: Получить вектор "вверх" актора.

---

#### GetVelocity
```
UFUNCTION(BlueprintCallable, Category = "Actor")
FVector GetVelocity() const
```
**Описание**: Получить скорость актора (вектор).

**Return**: FVector (направление * скорость)

---

### Lifecycle

#### Destroy
```
UFUNCTION(BlueprintCallable, Category = "Actor")
void Destroy()
```
**Описание**: Уничтожить актора.

---

#### SetActorHiddenInGame
```
UFUNCTION(BlueprintCallable, Category = "Actor")
void SetActorHiddenInGame(bool bNewHidden)
```
**Описание**: Скрыть/показать актора в игре.

---

#### SetActorEnableCollision
```
UFUNCTION(BlueprintCallable, Category = "Actor")
void SetActorEnableCollision(bool bNewActorEnableCollision)
```
**Описание**: Включить/выключить коллизию актора.

---

## 📋 Типы данных Blueprint

### FVector
```
X (float) - Координата X (вперёд/назад в Unreal)
Y (float) - Координата Y (вправо/влево)
Z (float) - Координата Z (вверх/вниз)
```

**Частые функции**:
- `Length` - Длина вектора
- `Normalize` - Нормализовать (сделать длину = 1)
- `Dot Product` - Скалярное произведение
- `Cross Product` - Векторное произведение

---

### FRotator
```
Pitch (float) - Поворот вверх/вниз (-90 до 90)
Yaw (float) - Поворот влево/вправо (-180 до 180)
Roll (float) - Крен влево/вправо (-180 до 180)
```

---

### FTransform
```
Location (FVector) - Позиция
Rotation (FRotator) - Поворот
Scale (FVector) - Масштаб
```

---

## 🎯 Частые паттерны использования

### Проверка спринта
```
┌─────────────┐
│ Is Sprinting│
└──────┬──────┘
       │ bool
       ▼
┌─────────────┐
│   Branch    │
├──────┬──────┤
│ True │False│
```

---

### Получение скорости для AnimBP
```
┌──────────────────┐
│ Get Current Speed│
└────────┬─────────┘
         │ float
         ▼
    [Blend Space]
    (Idle/Walk/Run)
```

---

### Взаимодействие с объектом
```
┌────────────────────────┐
│ Get Interaction Comp   │
└───────────┬────────────┘
            │
            ▼
┌────────────────────────┐
│ Has Focused Actor      │
└───────────┬────────────┘
            │ bool
            ▼
       ┌─────────┐
       │ Branch  │
       ├────┬────┤
       │True│    │
       ▼    │    │
┌──────────────┐ │
│ Try Interact │ │
└──────────────┘ │
                 ▼
           [Do Nothing]
```

---

### Смена камеры при входе в поезд
```
┌────────────────────────┐
│ Set Controlled Train   │
│ Train: RailsTrain Ref  │
└───────────┬────────────┘
            │
            ▼
┌────────────────────────┐
│ Set Camera Socket      │
│ Socket: "CabinCamera"  │
│ Ignore Rotation: true  │
└────────────────────────┘
```

---

## 💡 Советы по использованию в Blueprint

### ✅ DO (делайте):
1. Используйте **Pure** функции (геттеры) где возможно - они не имеют execution pins
2. Кэшируйте компоненты в переменных (GetInteractionComponent один раз в BeginPlay)
3. Проверяйте **IsValid** перед использованием объектных ссылок
4. Используйте **Category** для организации переменных

### ❌ DON'T (не делайте):
1. Не вызывайте геттеры каждый Tick (кэшируйте результат)
2. Не забывайте проверять nullptr для объектов
3. Не дублируйте логику - используйте Functions/Macros
4. Не храните референсы на уничтоженные объекты

---

## 📚 Итого

Этот документ содержит **ПОЛНОЕ** описание всего, что доступно в Blueprint для `ARailsPlayerCharacter`:

- ✅ Все компоненты (Camera, Movement, Interaction, Mesh, Capsule)
- ✅ Все переменные (Camera settings, Movement, Input, Animation)
- ✅ Все функции (Movement, Camera, Input handlers, Train, Weapon)
- ✅ Наследуемые функции (ACharacter, APawn, AActor)
- ✅ Типы данных (FVector, FRotator, FTransform)
- ✅ Паттерны использования

**Используйте как справочник при создании Blueprint!** 📖
