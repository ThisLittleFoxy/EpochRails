# 📚 Полный справочник Blueprint API проекта EpochRails

## Все доступные классы, переменные, функции и компоненты

---

# 📑 Содержание

1. [ARailsPlayerCharacter](#1-arailsplayercharacter) - Игровой персонаж (C++ базовый класс)
2. [IControllableCharacterInterface](#2-icontrollablecharacterinterface) - Интерфейс для BP персонажей
3. [ARailsTrain](#3-arailstrain) - Поезд
4. [ARailsSplinePath](#4-arailssplinepath) - Пути для поездов
5. [UInteractionComponent](#5-uinteractioncomponent) - Компонент взаимодействия
6. [AInteractableActor](#6-ainteractableactor) - Интерактивные объекты
7. [IInteractableInterface](#7-iinteractableinterface) - Интерфейс взаимодействия
8. [ARailsPlayerController](#8-arailsplayercontroller) - Контроллер игрока
9. [UTrainCheatManager](#9-utraincheatmanager) - Читы для разработки
10. [FAimTraceService](#10-faimtraceservice) - Утилиты трейсинга
11. [Создание FPS Blueprint Character](#11-создание-fps-blueprint-character) - Руководство

---

# 1. ARailsPlayerCharacter

**Тип**: Игровой персонаж (наследует ACharacter)
**Путь**: `Source/EpochRails/Character/RailsPlayerCharacter.h`
**Доступен в BP**: ✅ Да (UCLASS abstract)

## 📦 Компоненты

### CameraBoom
```cpp
Type: USpringArmComponent*
Access: BlueprintReadOnly (через GetCameraBoom())
Category: Components
```
Spring Arm для позиционирования камеры за персонажем.

### FollowCamera
```cpp
Type: UCameraComponent*
Access: BlueprintReadOnly (через GetFollowCamera())
Category: Components
```
Камера, следующая за персонажем.

### InteractionComponent
```cpp
Type: UInteractionComponent*
Access: BlueprintReadOnly (через GetInteractionComponent())
Category: Components
```
Компонент для взаимодействия с объектами мира.

---

## 📊 Переменные

### 🎥 Camera Settings (Protected - EditAnywhere/BlueprintReadWrite)

#### CameraSocketName
```cpp
Type: FName
Access: EditAnywhere, BlueprintReadWrite
Default: None
Category: Camera
```
Имя сокета на меше для привязки камеры. Пусто = привязка к root.

#### bAttachCameraToSocket
```cpp
Type: bool
Access: EditAnywhere, BlueprintReadWrite
Default: false
Category: Camera
```
Привязать ли камеру к сокету меша.

#### CameraRelativeLocationOffset
```cpp
Type: FVector
Access: EditAnywhere, BlueprintReadWrite
Default: (0, 0, 0)
Category: Camera
```
Смещение позиции камеры относительно точки крепления.

#### CameraRelativeRotationOffset
```cpp
Type: FRotator
Access: EditAnywhere, BlueprintReadWrite
Default: (0, 0, 0)
Category: Camera
```
Смещение поворота камеры.

#### bIgnoreSocketRotation
```cpp
Type: bool
Access: EditAnywhere, BlueprintReadWrite
Default: true
Category: Camera
```
Игнорировать вращение сокета (использовать world rotation).

---

### 🏃 Movement Settings

#### WalkSpeed
```cpp
Type: float
Access: EditAnywhere, BlueprintReadWrite
Default: 500.0
Category: Movement|Sprint
```
Обычная скорость ходьбы (см/сек).

#### SprintSpeed
```cpp
Type: float
Access: EditAnywhere, BlueprintReadWrite
Default: 800.0
Category: Movement|Sprint
```
Скорость спринта (см/сек).

---

### 🎮 Input Actions (Protected - EditAnywhere)

#### JumpAction
```cpp
Type: UInputAction*
Access: EditAnywhere
Category: Input
```
Enhanced Input Action для прыжка.

#### MoveAction
```cpp
Type: UInputAction*
Access: EditAnywhere
Category: Input
```
Enhanced Input Action для движения (WASD).

#### LookAction
```cpp
Type: UInputAction*
Access: EditAnywhere
Category: Input
```
Enhanced Input Action для взгляда (геймпад).

#### MouseLookAction
```cpp
Type: UInputAction*
Access: EditAnywhere
Category: Input
```
Enhanced Input Action для взгляда (мышь).

#### SprintAction
```cpp
Type: UInputAction*
Access: EditAnywhere
Category: Input
```
Enhanced Input Action для спринта.

#### InteractAction
```cpp
Type: UInputAction*
Access: EditAnywhere
Category: Input
```
Enhanced Input Action для взаимодействия.

#### FireAction
```cpp
Type: UInputAction*
Access: EditAnywhere, BlueprintReadOnly
Category: Input
```
Enhanced Input Action для стрельбы/UI клика.

#### ThrottleAction
```cpp
Type: UInputAction*
Access: EditDefaultsOnly, BlueprintReadOnly
Category: Input|Train
```
Input Action для газа поезда.

#### BrakeAction
```cpp
Type: UInputAction*
Access: EditDefaultsOnly, BlueprintReadOnly
Category: Input|Train
```
Input Action для тормоза поезда.

---

### 🚂 Train Control

#### ControlledTrain
```cpp
Type: ARailsTrain*
Access: BlueprintReadOnly
Default: nullptr
Category: Train
```
Ссылка на поезд, которым управляет персонаж.

---

### 🎬 Animation Variables (Public - BlueprintReadOnly)

#### bIsSprinting
```cpp
Type: bool
Access: VisibleAnywhere, BlueprintReadOnly
Default: false
Category: Movement|Animation
```
Спринтит ли персонаж (для AnimBP).

#### CurrentSpeed
```cpp
Type: float
Access: VisibleAnywhere, BlueprintReadOnly
Default: 0.0
Category: Movement|Animation
```
Текущая скорость в см/сек (для AnimBP).

#### MovementDirection
```cpp
Type: float
Access: VisibleAnywhere, BlueprintReadOnly
Default: 0.0
Category: Movement|Animation
```
Направление движения в градусах (-180 до 180).

#### bIsInAir
```cpp
Type: bool
Access: VisibleAnywhere, BlueprintReadOnly
Default: false
Category: Movement|Animation
```
В воздухе ли персонаж (для AnimBP).

---

## 🔧 Функции

### 🎥 Camera Functions

#### SetCameraSocket
```cpp
UFUNCTION(BlueprintCallable, Category = "Camera")
void SetCameraSocket(FName NewSocketName, bool bIgnoreRotation = true)
```
Динамически изменить сокет камеры.

**Параметры**:
- `NewSocketName` - Новый сокет
- `bIgnoreRotation` - Игнорировать вращение

#### ResetCameraToDefault
```cpp
UFUNCTION(BlueprintCallable, Category = "Camera")
void ResetCameraToDefault()
```
Сбросить камеру к root компоненту.

---

### 🏃 Movement Functions

#### DoStartSprint
```cpp
UFUNCTION(BlueprintCallable, Category = "Movement")
void DoStartSprint()
```
Начать спринт.

#### DoStopSprint
```cpp
UFUNCTION(BlueprintCallable, Category = "Movement")
void DoStopSprint()
```
Остановить спринт.

---

### 🎮 Input Handler Functions

#### DoMove
```cpp
UFUNCTION(BlueprintCallable, Category = "Input")
virtual void DoMove(float Right, float Forward)
```
Обработать движение.

**Параметры**:
- `Right` - Движение вправо/влево (-1 до 1)
- `Forward` - Движение вперёд/назад (-1 до 1)

#### DoLook
```cpp
UFUNCTION(BlueprintCallable, Category = "Input")
virtual void DoLook(float Yaw, float Pitch)
```
Обработать поворот камеры.

#### DoJumpStart
```cpp
UFUNCTION(BlueprintCallable, Category = "Input")
virtual void DoJumpStart()
```
Начать прыжок.

#### DoJumpEnd
```cpp
UFUNCTION(BlueprintCallable, Category = "Input")
virtual void DoJumpEnd()
```
Остановить прыжок.

#### DoInteract
```cpp
UFUNCTION(BlueprintCallable, Category = "Interaction")
virtual void DoInteract()
```
Взаимодействовать с объектом.

---

### 🚂 Train Functions

#### SetControlledTrain
```cpp
UFUNCTION(BlueprintCallable, Category = "Train")
void SetControlledTrain(ARailsTrain* Train)
```
Установить поезд под управлением.

#### GetControlledTrain
```cpp
UFUNCTION(BlueprintPure, Category = "Train")
ARailsTrain* GetControlledTrain() const
```
Получить управляемый поезд.

---

### 🔫 Weapon Functions

#### Fire
```cpp
UFUNCTION(BlueprintCallable, Category = "Weapon")
void Fire()
```
Выстрелить/активировать оружие.

---

### 📍 Getters (FORCEINLINE - очень быстрые)

#### GetCameraBoom
```cpp
FORCEINLINE USpringArmComponent* GetCameraBoom() const
```

#### GetFollowCamera
```cpp
FORCEINLINE UCameraComponent* GetFollowCamera() const
```

#### GetInteractionComponent
```cpp
FORCEINLINE UInteractionComponent* GetInteractionComponent() const
```

#### IsSprinting
```cpp
FORCEINLINE bool IsSprinting() const
```

#### GetCurrentSpeed
```cpp
FORCEINLINE float GetCurrentSpeed() const
```

#### GetMovementDirection
```cpp
FORCEINLINE float GetMovementDirection() const
```

#### IsInAir
```cpp
FORCEINLINE bool IsInAir() const
```

---

# 2. IControllableCharacterInterface

**Тип**: Blueprint Interface
**Путь**: `Source/EpochRails/Character/ControllableCharacterInterface.h`
**Доступен в BP**: ✅ Да (BlueprintType, BlueprintNativeEvent)

Интерфейс для персонажей, управляемых `ARailsPlayerController`. Реализуйте этот интерфейс в вашем Blueprint Character, чтобы получать input события от контроллера.

## 🎯 Назначение

Контроллер обрабатывает весь input и делегирует его персонажу:

| Input | Метод контроллера | Метод интерфейса |
|-------|-------------------|------------------|
| Move (WASD) | HandleMove | - (прямой вызов AddMovementInput) |
| Look (Mouse) | HandleLook | - (прямой вызов AddYaw/PitchInput) |
| Jump | HandleJump | - (прямой вызов Jump/StopJumping) |
| **Sprint** | HandleSprint | **StartSprint / StopSprint** |
| **Interact** | HandleInteract | **DoInteract** |
| **Fire** | HandleFire | **StartFire / StopFire** |

## 🔧 Функции (BlueprintNativeEvent)

### HandleMovementInput
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input|Movement")
void HandleMovementInput(const FVector& WorldDirection, float ScaleValue)
```
Опциональный: получить направление движения, уже рассчитанное контроллером.

**Параметры**:
- `WorldDirection` - Направление в мировых координатах
- `ScaleValue` - Величина input (0-1)

---

### StartSprint
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input|Movement")
void StartSprint()
```
Вызывается когда нажата кнопка спринта.

**Рекомендуемая реализация в BP**:
```
Event StartSprint
  → Set bIsSprinting = true
  → CharacterMovement → Set Max Walk Speed = SprintSpeed
```

---

### StopSprint
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input|Movement")
void StopSprint()
```
Вызывается когда отпущена кнопка спринта.

**Рекомендуемая реализация в BP**:
```
Event StopSprint
  → Set bIsSprinting = false
  → CharacterMovement → Set Max Walk Speed = WalkSpeed
```

---

### DoInteract
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input|Interaction")
void DoInteract()
```
Вызывается когда нажата кнопка взаимодействия.

**Рекомендуемая реализация в BP**:
```
Event DoInteract
  → InteractionComponent → Try Interact
```

---

### StartFire
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input|Combat")
void StartFire()
```
Вызывается когда нажата кнопка стрельбы.

**Рекомендуемая реализация в BP**:
```
Event StartFire
  → Branch: InteractionComponent → Is Hovering Widget?
    → True: InteractionComponent → Press Widget Interaction
    → False: Fire Weapon logic
```

---

### StopFire
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input|Combat")
void StopFire()
```
Вызывается когда отпущена кнопка стрельбы.

**Рекомендуемая реализация в BP**:
```
Event StopFire
  → InteractionComponent → Release Widget Interaction
```

---

# 3. ARailsTrain

**Тип**: Поезд (наследует APawn)
**Путь**: `Source/EpochRails/Train/RailsTrain.h`
**Доступен в BP**: ✅ Да (UCLASS Blueprintable)

## 📦 Компоненты

### Root
```cpp
Type: USceneComponent*
Access: VisibleAnywhere, BlueprintReadOnly
Category: Components
```
Корневой компонент.

### BodyMesh
```cpp
Type: UStaticMeshComponent*
Access: VisibleAnywhere, BlueprintReadOnly
Category: Components
```
Меш поезда.

### Movement
```cpp
Type: UFloatingPawnMovement*
Access: VisibleAnywhere, BlueprintReadOnly
Category: Components
```
Компонент движения.

### InteriorTrigger
```cpp
Type: UBoxComponent*
Access: VisibleAnywhere, BlueprintReadOnly
Category: Components
```
Триггер для определения когда игрок внутри поезда.

---

## 📊 Переменные

### 🛤️ Path Settings (Protected - EditAnywhere/BlueprintReadWrite)

#### ActivePath
```cpp
Type: ARailsSplinePath*
Access: EditAnywhere, BlueprintReadWrite
Default: nullptr
Category: Train|Path
```
Активный путь (spline), по которому движется поезд.

#### LookAheadDistance
```cpp
Type: float
Access: EditAnywhere, BlueprintReadWrite
Default: 50.0
Category: Train|Path
```
Дистанция просмотра вперёд по пути (см).

#### StopTolerance
```cpp
Type: float
Access: EditAnywhere, BlueprintReadWrite
Default: 50.0
Category: Train|Path
```
Допуск расстояния для остановки (см).

---

### 🚂 Movement Settings

#### Speed
```cpp
Type: float
Access: EditAnywhere, BlueprintReadWrite
Default: 1.0
Category: Train|Movement
```
Текущая скорость поезда.

#### bStop
```cpp
Type: bool
Access: EditAnywhere, BlueprintReadWrite
Default: false
Category: Train|Movement
```
Остановлен ли поезд.

#### bAutoStart
```cpp
Type: bool
Access: EditAnywhere, BlueprintReadWrite
Default: false
Category: Train|Movement
```
Автоматически стартовать при BeginPlay.

---

### 🎮 Input Settings

#### DefaultInputMappingContext
```cpp
Type: UInputMappingContext*
Access: EditAnywhere, BlueprintReadOnly
Default: nullptr
Category: Train|Input
```
Дефолтный input mapping context (для обычного управления).

#### TrainPassengerInputMappingContext
```cpp
Type: UInputMappingContext*
Access: EditAnywhere, BlueprintReadOnly
Default: nullptr
Category: Train|Input
```
Input mapping context когда игрок в поезде.

#### IMCPriority
```cpp
Type: int32
Access: EditAnywhere, BlueprintReadOnly
Default: 0
Category: Train|Input
```
Приоритет Input Mapping Context.

---

## 🔧 Функции

### 🚂 Movement API

#### StartTrain
```cpp
UFUNCTION(BlueprintCallable, Category = "Train")
void StartTrain()
```
Запустить поезд.

#### StopTrain
```cpp
UFUNCTION(BlueprintCallable, Category = "Train")
void StopTrain()
```
Остановить поезд.

#### GetSpeed
```cpp
UFUNCTION(BlueprintPure, Category = "Train")
float GetSpeed() const
```
Получить текущую скорость.

#### SetSpeed
```cpp
UFUNCTION(BlueprintCallable, Category = "Train")
void SetSpeed(float NewSpeed)
```
Установить скорость.

#### IsStopped
```cpp
UFUNCTION(BlueprintPure, Category = "Train")
bool IsStopped() const
```
Проверить, остановлен ли поезд.

---

### 👥 Passenger Management

#### IsPassengerInside
```cpp
UFUNCTION(BlueprintCallable, Category = "Train|Passengers")
bool IsPassengerInside(ARailsPlayerCharacter* Character) const
```
Проверить, находится ли персонаж внутри поезда.

#### OnPlayerEnterTrain
```cpp
UFUNCTION(BlueprintCallable, Category = "Train|Passengers")
void OnPlayerEnterTrain(ARailsPlayerCharacter* Character)
```
Вызвать когда игрок входит в поезд.

#### OnPlayerExitTrain
```cpp
UFUNCTION(BlueprintCallable, Category = "Train|Passengers")
void OnPlayerExitTrain(ARailsPlayerCharacter* Character)
```
Вызвать когда игрок выходит из поезда.

---

### 🛤️ Path Functions

#### UpdatePath
```cpp
UFUNCTION(BlueprintCallable, Category = "Train|Path")
void UpdatePath()
```
Обновить движение по пути.

---

# 3. ARailsSplinePath

**Тип**: Spline путь для поездов (наследует AActor)
**Путь**: `Source/EpochRails/Train/RailsSplinePath.h`
**Доступен в BP**: ✅ Да (UCLASS Blueprintable)

## 📦 Компоненты

### SplineComponent
```cpp
Type: USplineComponent*
Access: VisibleAnywhere, BlueprintReadOnly
Category: Components
```
Spline компонент, определяющий путь.

### PathVisualizationMesh
```cpp
Type: USplineMeshComponent*
Access: VisibleAnywhere, BlueprintReadOnly
Category: Components
```
Меш для визуализации пути (опционально).

---

## 📊 Переменные

### 🐛 Debug Settings

#### bShowDebugVisualization
```cpp
Type: bool
Access: EditAnywhere
Default: true
Category: Debug
```
Показывать debug визуализацию в редакторе.

#### DebugColor
```cpp
Type: FLinearColor
Access: EditAnywhere
Default: Yellow
Category: Debug
```
Цвет debug визуализации.

---

## 🔧 Функции

### 🛤️ Spline Functions

#### GetSpline
```cpp
UFUNCTION(BlueprintPure, Category = "Spline")
USplineComponent* GetSpline() const
```
Получить spline компонент.

#### GetLocationAtDistance
```cpp
UFUNCTION(BlueprintPure, Category = "Spline")
FVector GetLocationAtDistance(float Distance) const
```
Получить позицию на дистанции по spline.

#### GetRotationAtDistance
```cpp
UFUNCTION(BlueprintPure, Category = "Spline")
FRotator GetRotationAtDistance(float Distance) const
```
Получить поворот на дистанции по spline.

#### GetSplineLength
```cpp
UFUNCTION(BlueprintPure, Category = "Spline")
float GetSplineLength() const
```
Получить общую длину spline.

---

# 4. UInteractionComponent

**Тип**: Actor Component
**Путь**: `Source/EpochRails/Interaction/InteractionComponent.h`
**Доступен в BP**: ✅ Да (BlueprintSpawnableComponent)

## 📊 Переменные

### 🔍 Interaction Settings (Protected - EditAnywhere/BlueprintReadWrite)

#### DefaultInteractionDistance
```cpp
Type: float
Access: EditAnywhere, BlueprintReadWrite
Default: 300.0
Category: Interaction
```
Максимальная дистанция взаимодействия (см).

#### InteractionCheckFrequency
```cpp
Type: float
Access: EditAnywhere, BlueprintReadWrite
Default: 0.1
Category: Interaction
```
Как часто проверять объекты (секунды).

#### InteractionTraceChannel
```cpp
Type: TEnumAsByte<ECollisionChannel>
Access: EditAnywhere, BlueprintReadWrite
Default: ECC_Visibility
Category: Interaction
```
Канал коллизии для трейса.

---

### 🐛 Debug Settings

#### bShowDebugTrace
```cpp
Type: bool
Access: EditAnywhere, BlueprintReadWrite
Default: false
Category: Interaction|Debug
```
Показывать debug линии трейса.

#### DebugTraceDuration
```cpp
Type: float
Access: EditAnywhere, BlueprintReadWrite
Default: 0.1
Category: Interaction|Debug
Condition: bShowDebugTrace
```
Длительность показа debug линий.

---

### 🖼️ Widget Interaction Settings

#### bEnableWidgetInteraction
```cpp
Type: bool
Access: EditAnywhere, BlueprintReadWrite
Default: true
Category: Interaction|Widget
```
Включить взаимодействие с UI виджетами.

#### bShowWidgetDebug
```cpp
Type: bool
Access: EditAnywhere, BlueprintReadWrite
Default: false
Category: Interaction|Widget
```
Показывать debug для widget interaction.

---

## 🔧 Функции

### 🔍 Interaction Functions

#### TryInteract
```cpp
UFUNCTION(BlueprintCallable, Category = "Interaction")
bool TryInteract()
```
Попытаться взаимодействовать с объектом в фокусе.

**Return**: true если успешно

#### GetFocusedActor
```cpp
UFUNCTION(BlueprintPure, Category = "Interaction")
AActor* GetFocusedActor() const
```
Получить актора в фокусе.

#### HasFocusedActor
```cpp
UFUNCTION(BlueprintPure, Category = "Interaction")
bool HasFocusedActor() const
```
Есть ли актор в фокусе.

#### GetFocusedActorName
```cpp
UFUNCTION(BlueprintPure, Category = "Interaction")
FText GetFocusedActorName() const
```
Получить имя объекта в фокусе.

#### GetFocusedActorAction
```cpp
UFUNCTION(BlueprintPure, Category = "Interaction")
FText GetFocusedActorAction() const
```
Получить текст действия ("Открыть", "Взять", etc).

#### CanInteractWithFocusedActor
```cpp
UFUNCTION(BlueprintPure, Category = "Interaction")
bool CanInteractWithFocusedActor() const
```
Можно ли взаимодействовать с объектом в фокусе.

---

### 🖼️ Widget Interaction Functions

#### PressWidgetInteraction
```cpp
UFUNCTION(BlueprintCallable, Category = "Interaction")
void PressWidgetInteraction()
```
Нажать на UI виджет (клик мыши).

#### ReleaseWidgetInteraction
```cpp
UFUNCTION(BlueprintCallable, Category = "Interaction")
void ReleaseWidgetInteraction()
```
Отпустить кнопку на UI виджете.

#### IsHoveringWidget
```cpp
UFUNCTION(BlueprintPure, Category = "Interaction")
bool IsHoveringWidget() const
```
Навёл ли курсор на UI виджет.

#### GetWidgetInteractionComponent
```cpp
UFUNCTION(BlueprintPure, Category = "Interaction")
UWidgetInteractionComponent* GetWidgetInteractionComponent() const
```
Получить компонент widget interaction.

---

# 5. AInteractableActor

**Тип**: Интерактивный актор (наследует AActor, реализует IInteractableInterface)
**Путь**: `Source/EpochRails/Interaction/InteractableActor.h`
**Доступен в BP**: ✅ Да (UCLASS Blueprintable)

## 📦 Компоненты

### SceneRoot
```cpp
Type: USceneComponent*
Access: VisibleAnywhere, BlueprintReadOnly
Category: Components
```
Корневой компонент.

### MeshComponent
```cpp
Type: UStaticMeshComponent*
Access: VisibleAnywhere, BlueprintReadOnly
Category: Components
```
Static mesh для визуального представления (опционально).

---

## 📊 Переменные

### 🔍 Interaction Settings (Protected - EditAnywhere/BlueprintReadWrite)

#### InteractionName
```cpp
Type: FText
Access: EditAnywhere, BlueprintReadWrite
Default: "Interactable Object"
Category: Interaction
```
Имя объекта для отображения.

#### InteractionAction
```cpp
Type: FText
Access: EditAnywhere, BlueprintReadWrite
Default: "Interact"
Category: Interaction
```
Текст действия ("Открыть", "Взять", "Использовать").

#### bCanInteract
```cpp
Type: bool
Access: EditAnywhere, BlueprintReadWrite
Default: true
Category: Interaction
```
Можно ли взаимодействовать с объектом.

#### MaxInteractionDistance
```cpp
Type: float
Access: EditAnywhere, BlueprintReadWrite
Default: 300.0
Category: Interaction
```
Максимальная дистанция взаимодействия (см).

#### bEnableDebugLog
```cpp
Type: bool
Access: EditAnywhere, BlueprintReadWrite
Default: false
Category: Interaction|Debug
```
Включить debug логи для этого объекта.

---

### 🎬 Animation Settings

#### InteractionType
```cpp
Type: EInteractionType (enum)
Access: EditAnywhere, BlueprintReadWrite
Default: None
Category: Interaction|Animation

Возможные значения:
- None          - Без анимации
- Pickup        - Поднять предмет
- Sit           - Сесть/встать
- OpenDoor      - Открыть дверь
- PullLever     - Потянуть рычаг
- PressButton   - Нажать кнопку
- Custom        - Кастомная анимация
```
Тип взаимодействия для выбора анимации.

#### bPlayAnimationOnInteract
```cpp
Type: bool
Access: EditAnywhere, BlueprintReadWrite
Default: false
Category: Interaction|Animation
```
Проигрывать ли анимацию при взаимодействии.

#### CustomAnimationMontage
```cpp
Type: UAnimMontage*
Access: EditAnywhere, BlueprintReadWrite
Default: nullptr
Category: Interaction|Animation
```
Кастомный animation montage (переопределяет InteractionType).

#### AnimationPlayRate
```cpp
Type: float
Access: EditAnywhere, BlueprintReadWrite
Default: 1.0
Range: 0.1 - 5.0
Category: Interaction|Animation
```
Множитель скорости анимации.

#### bWaitForAnimationComplete
```cpp
Type: bool
Access: EditAnywhere, BlueprintReadWrite
Default: true
Category: Interaction|Animation
```
Ждать ли завершения анимации перед выполнением действия.

#### AnimationSectionName
```cpp
Type: FName
Access: EditAnywhere, BlueprintReadWrite
Default: None
Category: Interaction|Animation
```
Имя секции montage для проигрывания (опционально).

---

### 🎭 Default Animation Montages (Protected - EditDefaultsOnly/BlueprintReadOnly)

#### PickupAnimationMontage
```cpp
Type: UAnimMontage*
Access: EditDefaultsOnly, BlueprintReadOnly
Default: nullptr
Category: Interaction|Animation|Defaults
```
Дефолтная анимация поднятия предмета.

#### SitAnimationMontage
```cpp
Type: UAnimMontage*
Access: EditDefaultsOnly, BlueprintReadOnly
Default: nullptr
Category: Interaction|Animation|Defaults
```
Дефолтная анимация сидения.

#### OpenDoorAnimationMontage
```cpp
Type: UAnimMontage*
Access: EditDefaultsOnly, BlueprintReadOnly
Default: nullptr
Category: Interaction|Animation|Defaults
```
Дефолтная анимация открытия двери.

#### PullLeverAnimationMontage
```cpp
Type: UAnimMontage*
Access: EditDefaultsOnly, BlueprintReadOnly
Default: nullptr
Category: Interaction|Animation|Defaults
```
Дефолтная анимация рычага.

#### PressButtonAnimationMontage
```cpp
Type: UAnimMontage*
Access: EditDefaultsOnly, BlueprintReadOnly
Default: nullptr
Category: Interaction|Animation|Defaults
```
Дефолтная анимация кнопки.

---

## 🔧 Функции

### 🔍 IInteractableInterface Implementation

#### OnInteractionFocusBegin_Implementation
```cpp
virtual void OnInteractionFocusBegin_Implementation(ARailsPlayerCharacter* PlayerCharacter)
```
Вызывается когда игрок начинает смотреть на объект.

#### OnInteractionFocusEnd_Implementation
```cpp
virtual void OnInteractionFocusEnd_Implementation(ARailsPlayerCharacter* PlayerCharacter)
```
Вызывается когда игрок перестаёт смотреть на объект.

#### OnInteract_Implementation
```cpp
virtual bool OnInteract_Implementation(ARailsPlayerCharacter* PlayerCharacter)
```
Вызывается при взаимодействии. **Return**: true если успешно.

#### GetInteractionName_Implementation
```cpp
virtual FText GetInteractionName_Implementation() const
```
Вернуть имя объекта.

#### GetInteractionAction_Implementation
```cpp
virtual FText GetInteractionAction_Implementation() const
```
Вернуть текст действия.

#### CanInteract_Implementation
```cpp
virtual bool CanInteract_Implementation(ARailsPlayerCharacter* PlayerCharacter) const
```
Проверить, можно ли взаимодействовать.

#### GetInteractionDistance_Implementation
```cpp
virtual float GetInteractionDistance_Implementation() const
```
Вернуть максимальную дистанцию взаимодействия.

---

### 🎬 Animation Functions

#### PlayInteractionAnimation
```cpp
UFUNCTION(BlueprintCallable, Category = "Interaction|Animation")
bool PlayInteractionAnimation(ARailsPlayerCharacter* PlayerCharacter)
```
Проиграть анимацию взаимодействия на персонаже.

#### GetInteractionAnimationMontage
```cpp
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Interaction|Animation")
UAnimMontage* GetInteractionAnimationMontage() const
```
Получить montage для текущего типа взаимодействия.

#### OnInteractionAnimationComplete
```cpp
UFUNCTION(BlueprintCallable, Category = "Interaction|Animation")
void OnInteractionAnimationComplete(ARailsPlayerCharacter* PlayerCharacter)
```
Вызывается когда анимация завершена.

---

### 📢 Blueprint Events

#### BP_OnAnimationStart
```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Animation")
void BP_OnAnimationStart(ARailsPlayerCharacter* PlayerCharacter)
```
BP событие: анимация началась.

#### BP_OnAnimationComplete
```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Animation")
void BP_OnAnimationComplete(ARailsPlayerCharacter* PlayerCharacter)
```
BP событие: анимация завершена.

#### BP_OnInteractionFocusBegin
```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
void BP_OnInteractionFocusBegin(ARailsPlayerCharacter* PlayerCharacter)
```
BP событие: игрок начал смотреть.

#### BP_OnInteractionFocusEnd
```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
void BP_OnInteractionFocusEnd(ARailsPlayerCharacter* PlayerCharacter)
```
BP событие: игрок перестал смотреть.

#### BP_OnInteract
```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
bool BP_OnInteract(ARailsPlayerCharacter* PlayerCharacter)
```
BP событие: взаимодействие. **Return**: true если успешно.

---

# 6. IInteractableInterface

**Тип**: Interface
**Путь**: `Source/EpochRails/Interaction/InteractableInterface.h`
**Доступен в BP**: ✅ Да (BlueprintType)

## 🔧 Функции (BlueprintNativeEvent)

### OnInteractionFocusBegin
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
void OnInteractionFocusBegin(ARailsPlayerCharacter* PlayerCharacter)
```
Игрок начал смотреть на объект.

### OnInteractionFocusEnd
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
void OnInteractionFocusEnd(ARailsPlayerCharacter* PlayerCharacter)
```
Игрок перестал смотреть на объект.

### OnInteract
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
bool OnInteract(ARailsPlayerCharacter* PlayerCharacter)
```
Взаимодействие с объектом. **Return**: success.

### GetInteractionName
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
FText GetInteractionName() const
```
Получить имя объекта для UI.

### GetInteractionAction
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
FText GetInteractionAction() const
```
Получить текст действия ("Open", "Use", etc).

### CanInteract
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
bool CanInteract(ARailsPlayerCharacter* PlayerCharacter) const
```
Можно ли сейчас взаимодействовать.

### GetInteractionDistance
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
float GetInteractionDistance() const
```
Максимальная дистанция взаимодействия (см).

---

# 7. ARailsPlayerController

**Тип**: Player Controller
**Путь**: `Source/EpochRails/Controllers/RailsPlayerController.h`
**Доступен в BP**: ✅ Да (UCLASS abstract)

## 📊 Переменные

### 🎮 Input Mappings (Protected - EditAnywhere)

#### DefaultMappingContexts
```cpp
Type: TArray<UInputMappingContext*>
Access: EditAnywhere
Category: Input|Input Mappings
```
Массив дефолтных Input Mapping Contexts.

#### MobileExcludedMappingContexts
```cpp
Type: TArray<UInputMappingContext*>
Access: EditAnywhere
Category: Input|Input Mappings
```
Массив contexts, которые не используются на мобильных.

---

### 📱 Mobile Controls

#### MobileControlsWidgetClass
```cpp
Type: TSubclassOf<UUserWidget>
Access: EditAnywhere
Category: Input|Touch Controls
```
Класс виджета для мобильного управления.

#### bForceTouchControls
```cpp
Type: bool
Access: EditAnywhere, Config
Default: false
Category: Input|Touch Controls
```
Принудительно использовать touch controls (даже на PC).

---

### 🖱️ UI State

#### bIsInteractingWithUI
```cpp
Type: bool
Access: BlueprintReadOnly
Default: false
Category: UI
```
Взаимодействует ли игрок с UI сейчас.

---

## 🔧 Функции

### 🖱️ UI Functions

#### SetMouseCursorVisible
```cpp
UFUNCTION(BlueprintCallable, Category = "UI")
void SetMouseCursorVisible(bool bVisible)
```
Показать/скрыть курсор мыши.

---

# 8. UTrainCheatManager

**Тип**: Cheat Manager
**Путь**: `Source/EpochRails/Utils/TrainCheatManager.h`
**Доступен в BP**: ⚠️ Только для разработки

## 🔧 Функции (Console Commands)

### AddWagons
```cpp
UFUNCTION(Exec, Category = "Train")
void AddWagons(int32 Count = 1)
```
**Console**: `AddWagons 5`
Добавить вагоны к ближайшему поезду.

### RemoveWagons
```cpp
UFUNCTION(Exec, Category = "Train")
void RemoveWagons(int32 Count = 1)
```
**Console**: `RemoveWagons 2`
Убрать вагоны с ближайшего поезда.

### TrainInfo
```cpp
UFUNCTION(Exec, Category = "Train")
void TrainInfo()
```
**Console**: `TrainInfo`
Показать информацию о ближайшем поезде.

---

# 9. FAimTraceService

**Тип**: Static Utility Class
**Путь**: `Source/EpochRails/Utils/AimTraceService.h`
**Доступен в BP**: ❌ Нет (только C++)

## 🔧 Static Functions

### MakeScreenCenterRay
```cpp
static bool MakeScreenCenterRay(const APlayerController* PC, float Distance,
                                FVector& OutStart, FVector& OutEnd)
```
Создать луч из центра экрана.

**Параметры**:
- `PC` - Player Controller
- `Distance` - Дальность луча
- `OutStart` - Начало луча (out)
- `OutEnd` - Конец луча (out)

**Return**: true если успешно

---

### MakeViewPointRay
```cpp
static bool MakeViewPointRay(const AController* Controller, float Distance,
                             FVector& OutStart, FVector& OutEnd)
```
Создать луч от точки взгляда контроллера.

---

### TraceFromScreenCenter
```cpp
static bool TraceFromScreenCenter(UWorld* World, const APlayerController* PC,
                                  float Distance, ECollisionChannel Channel,
                                  const TArray<AActor*>& ActorsToIgnore,
                                  FHitResult& OutHit, bool bTraceComplex = false)
```
Выполнить line trace из центра экрана.

**Return**: true если что-то задело

---

### TraceFromViewPoint
```cpp
static bool TraceFromViewPoint(UWorld* World, const AController* Controller,
                               float Distance, ECollisionChannel Channel,
                               const TArray<AActor*>& ActorsToIgnore,
                               FHitResult& OutHit, bool bTraceComplex = false)
```
Выполнить line trace от точки взгляда.

**Return**: true если что-то задело

---

# 📋 Типы данных

## EInteractionType (Enum)
```cpp
None          - Без анимации
Pickup        - Поднять предмет
Sit           - Сесть/встать
OpenDoor      - Открыть дверь
PullLever     - Потянуть рычаг
PressButton   - Нажать кнопку
Custom        - Кастомная анимация
```

---

# 🎯 Частые паттерны использования

## Создание интерактивного объекта

```
1. Создать Blueprint от AInteractableActor
2. Установить:
   - InteractionName: "Door"
   - InteractionAction: "Open"
   - InteractionType: OpenDoor
   - bPlayAnimationOnInteract: true
3. Реализовать BP_OnInteract event
```

## Управление поездом

```
1. Get Controlled Train от персонажа
2. Branch: Is Valid?
3. Если Valid:
   - Get Speed
   - Set Speed
   - Start Train / Stop Train
```

## Создание spline пути

```
1. Разместить ARailsSplinePath в уровне
2. Редактировать spline точки в viewport
3. Присвоить поезду: ActivePath = SplinePath
4. UpdatePath
```

---

# 💡 Советы по использованию

## ✅ DO (делайте):
- Используйте **Pure** функции (геттеры) для оптимизации
- Кэшируйте компоненты в BeginPlay
- Проверяйте **IsValid** перед использованием
- Используйте Interface для полиморфизма

## ❌ DON'T (не делайте):
- Не вызывайте геттеры каждый Tick
- Не забывайте nullptr проверки
- Не дублируйте логику
- Не храните "мёртвые" ссылки

---

# 📊 Иерархия классов

```
AActor
├─ APawn
│  └─ ACharacter
│     └─ ARailsPlayerCharacter
│  └─ ARailsTrain
├─ ARailsSplinePath
└─ AInteractableActor (+ IInteractableInterface)

APlayerController
└─ ARailsPlayerController

UActorComponent
└─ UInteractionComponent

UCheatManager
└─ UTrainCheatManager

UInterface
└─ IInteractableInterface

Static Class
└─ FAimTraceService
```

---

# 📖 Краткая справка по категориям

## По функциональности:

**Персонаж:**
- ARailsPlayerCharacter
- ARailsPlayerController

**Поезда:**
- ARailsTrain
- ARailsSplinePath

**Взаимодействие:**
- UInteractionComponent
- IInteractableInterface
- AInteractableActor

**Утилиты:**
- FAimTraceService
- UTrainCheatManager

---

## 🎓 Итого

Этот документ содержит **ВСЁ** из вашего проекта EpochRails:

- ✅ **9 классов** с полным API
- ✅ **100+ переменных** с описанием
- ✅ **80+ функций** с параметрами
- ✅ **Все компоненты** и их настройки
- ✅ **Enums** и типы данных
- ✅ **Паттерны** использования
- ✅ **Иерархия** классов

**Используйте как полный справочник при разработке!** 📚

---

# 11. Создание FPS Blueprint Character

Полное руководство по созданию персонажа от первого лица в Blueprint.

## 🎯 Архитектура

```
RailsPlayerController (C++)
    ↓ обрабатывает input
    ↓ вызывает методы на Pawn
    ↓
BP_FirstPersonCharacter (Blueprint)
    ├─ Реализует IControllableCharacterInterface
    ├─ Компоненты (добавляются в BP):
    │   ├─ CameraBoom (SpringArmComponent)
    │   ├─ FollowCamera (CameraComponent)
    │   └─ InteractionComponent
    └─ Логика Sprint/Interact/Fire
```

## 📋 Шаг 1: Создание Blueprint Character

1. **Content Browser** → Right Click → **Blueprint Class**
2. Выбрать **Character** как родительский класс
3. Назвать: `BP_FirstPersonCharacter`

## 📋 Шаг 2: Настройка FPS Movement

В **Class Defaults** (панель Details):

### Character Movement Component:
| Параметр | Значение | Описание |
|----------|----------|----------|
| `Orient Rotation to Movement` | ❌ **false** | Персонаж НЕ поворачивается по движению |
| `Max Walk Speed` | 500 | Обычная скорость |
| `Jump Z Velocity` | 500 | Высота прыжка |
| `Air Control` | 0.35 | Управление в воздухе |

### Character (Self):
| Параметр | Значение | Описание |
|----------|----------|----------|
| `Use Controller Rotation Pitch` | ❌ false | |
| `Use Controller Rotation Yaw` | ✅ **true** | Персонаж поворачивается с камерой |
| `Use Controller Rotation Roll` | ❌ false | |

## 📋 Шаг 3: Добавление компонентов

В **Components** панели добавить:

### 1. Spring Arm (CameraBoom)
```
Parent: CapsuleComponent (или Mesh socket)
```

| Параметр | Значение |
|----------|----------|
| `Target Arm Length` | 0 (FPS) или 300 (TPS) |
| `Use Pawn Control Rotation` | ✅ true |
| `Do Collision Test` | ❌ false (для FPS) |
| `Socket Offset` | (0, 0, 60) - на уровне глаз |

### 2. Camera (FollowCamera)
```
Parent: CameraBoom
```

| Параметр | Значение |
|----------|----------|
| `Use Pawn Control Rotation` | ❌ false |

### 3. Interaction Component
```
Parent: Root
```
Добавить компонент: **Interaction Component** (из списка)

## 📋 Шаг 4: Реализация интерфейса

1. **Class Settings** → **Interfaces** → **Add**
2. Найти и добавить: `ControllableCharacterInterface`
3. **Compile** Blueprint

## 📋 Шаг 5: Реализация событий интерфейса

В **Event Graph** реализовать события:

### Event Start Sprint
```
Event Start Sprint
    │
    ├─► Set bIsSprinting = true
    │
    └─► Get Character Movement
            │
            └─► Set Max Walk Speed = 800.0
```

### Event Stop Sprint
```
Event Stop Sprint
    │
    ├─► Set bIsSprinting = false
    │
    └─► Get Character Movement
            │
            └─► Set Max Walk Speed = 500.0
```

### Event Do Interact
```
Event Do Interact
    │
    └─► Interaction Component
            │
            └─► Try Interact
```

### Event Start Fire
```
Event Start Fire
    │
    └─► Branch: Interaction Component → Is Hovering Widget?
            │
            ├─► True: Interaction Component → Press Widget Interaction
            │
            └─► False: [Your weapon fire logic]
```

### Event Stop Fire
```
Event Stop Fire
    │
    └─► Interaction Component → Release Widget Interaction
```

## 📋 Шаг 6: Переменные для анимации (опционально)

Создать переменные:

| Имя | Тип | Описание |
|-----|-----|----------|
| `bIsSprinting` | bool | Для Animation Blueprint |
| `CurrentSpeed` | float | Скорость для AnimBP |
| `bIsInAir` | bool | В воздухе ли |
| `WalkSpeed` | float (default 500) | Скорость ходьбы |
| `SprintSpeed` | float (default 800) | Скорость спринта |

### Event Tick (для обновления):
```
Event Tick
    │
    └─► Get Velocity → Vector Length → Set CurrentSpeed
    │
    └─► Character Movement → Is Falling → Set bIsInAir
```

## 📋 Шаг 7: Настройка GameMode

1. Открыть ваш **GameMode Blueprint**
2. Установить:
   - **Default Pawn Class**: `BP_FirstPersonCharacter`
   - **Player Controller Class**: `BP_RailsPlayerController` (ваш BP от ARailsPlayerController)

## ⚠️ Важные замечания

### Move и Look
Контроллер **сам** обрабатывает Move и Look. В персонаже **не нужно** реализовывать эти события. Контроллер вызывает:
- `AddMovementInput()` - для движения
- `AddYawInput()` / `AddPitchInput()` - для камеры
- `Jump()` / `StopJumping()` - для прыжка

### Input Actions
Input Actions **не нужно** настраивать в BP персонаже. Они настраиваются в контроллере через Input Mapping Contexts.

### Удаление старых событий
Если в вашем BP есть старые события из скриншотов (EnhancedInputAction IA_Move, IA_Look, etc.), их можно **удалить** - контроллер теперь обрабатывает это сам.

## 📊 Итоговая структура EventGraph

```
┌─────────────────────────────────────────────┐
│              EVENT GRAPH                     │
├─────────────────────────────────────────────┤
│                                              │
│  [Event BeginPlay]                          │
│       └─► Initial setup                     │
│                                              │
│  [Event Tick]                               │
│       └─► Update animation variables        │
│                                              │
│  ═══════ Interface Events ═══════           │
│                                              │
│  [Event Start Sprint]                       │
│       └─► bIsSprinting = true               │
│       └─► MaxWalkSpeed = SprintSpeed        │
│                                              │
│  [Event Stop Sprint]                        │
│       └─► bIsSprinting = false              │
│       └─► MaxWalkSpeed = WalkSpeed          │
│                                              │
│  [Event Do Interact]                        │
│       └─► InteractionComponent.TryInteract  │
│                                              │
│  [Event Start Fire]                         │
│       └─► Check UI hover → Fire/Press       │
│                                              │
│  [Event Stop Fire]                          │
│       └─► Release widget interaction        │
│                                              │
└─────────────────────────────────────────────┘
```

## ✅ Чеклист готовности

- [ ] Blueprint создан от Character
- [ ] `Use Controller Rotation Yaw` = true
- [ ] `Orient Rotation to Movement` = false
- [ ] Spring Arm добавлен с `Use Pawn Control Rotation` = true
- [ ] Camera добавлена к Spring Arm
- [ ] Interaction Component добавлен
- [ ] Интерфейс `ControllableCharacterInterface` реализован
- [ ] События Sprint/Interact/Fire реализованы
- [ ] GameMode настроен с правильным Pawn Class
- [ ] Старые input события удалены из EventGraph
