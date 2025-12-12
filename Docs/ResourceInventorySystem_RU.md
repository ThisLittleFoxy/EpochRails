# Resource & Inventory System (Система Ресурсов и Инвентаря)

## Описание

Система управления ресурсами для Epoch Rails. Позволяет хранить и управлять ресурсами на поезде или у персонажа.

**Локация кода:** `Source/EpochRails/Resources/`

## Основные компоненты

### EResourceType (Типы Ресурсов)

Enum с доступными типами ресурсов в игре.

**Файл:** `ResourceTypes.h`

```cpp
enum class EResourceType : uint8
{
    None,   // Пустой тип
    Metal,  // Металл
    Wood,   // Дерево
    Books   // Книжки
};
```

### UResourceInventoryComponent

Компонент для хранения и управления ресурсами. Добавьте его к `ARailsTrain` для инвентаря поезда.

**Файл:** `ResourceInventoryComponent.h/cpp`

#### Основные функции

```cpp
// Add resources to inventory
bool AddResource(EResourceType Type, int32 Amount);

// Remove resources from inventory
bool RemoveResource(EResourceType Type, int32 Amount);

// Get current amount
int32 GetResourceAmount(EResourceType Type) const;

// Check if has enough
bool HasEnoughResource(EResourceType Type, int32 Amount) const;

// Set capacity limit (0 = unlimited)
void SetResourceCapacity(EResourceType Type, int32 Capacity);
```

#### События

```cpp
// Event fired when resource amount changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceChanged, 
    EResourceType, ResourceType, 
    int32, NewAmount);

UPROPERTY(BlueprintAssignable)
FOnResourceChanged OnResourceChanged;
```

## Интеграция с RailsTrain

### Шаг 1: Добавить компонент в RailsTrain.h

```cpp
// В секции protected:
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
class UResourceInventoryComponent* TrainInventory;
```

### Шаг 2: Инициализировать в конструкторе

В файле `RailsTrain.cpp` в конструкторе:

```cpp
ARailsTrain::ARailsTrain()
{
    // ... existing code ...
    
    // Create inventory component
    TrainInventory = CreateDefaultSubobject<UResourceInventoryComponent>(TEXT("TrainInventory"));
}
```

### Шаг 3: Настроить ёмкости (опционально)

В `BeginPlay()` или в Blueprint:

```cpp
void ARailsTrain::BeginPlay()
{
    Super::BeginPlay();
    
    // Set capacity limits (0 = unlimited)
    if (TrainInventory)
    {
        TrainInventory->SetResourceCapacity(EResourceType::Metal, 1000);
        TrainInventory->SetResourceCapacity(EResourceType::Wood, 500);
        TrainInventory->SetResourceCapacity(EResourceType::Books, 100);
    }
}
```

## Использование в Blueprint

### Создание Pickup Actor

1. Создайте новый Blueprint Actor: `BP_ResourcePickup`
2. Добавьте компоненты:
   - `Static Mesh Component` (визуал предмета)
   - `Sphere Collision` (для взаимодействия)
3. Добавьте интерфейс `IInteractableInterface`
4. Добавьте переменные:
   - `Resource Type` (EResourceType) - выбираемый тип ресурса
   - `Amount` (int32) = 10 - количество

### Реализация взаимодействия

В функции `OnInteract` (из IInteractableInterface):

```blueprint
Event OnInteract (Interacting Character)
  ├─ Get Player Pawn
  ├─ Cast to RailsPlayerCharacter
  ├─ Get Current Train (from character)
  ├─ Get Train Inventory (component)
  ├─ Add Resource (Resource Type, Amount)
  └─ If Success:
      ├─ Play Sound
      ├─ Spawn Particle FX
      └─ Destroy Actor
```

### Привязка UI к событиям

В вашем UI Widget (например, `WBP_ResourceHUD`):

```blueprint
Event Construct
  ├─ Get Player Train
  ├─ Get Train Inventory
  └─ Bind Event to OnResourceChanged
      └─ Update Text: "Metal: {NewAmount}"
```

## Примеры использования

### Добавление ресурсов

```cpp
// В C++
if (TrainInventory->AddResource(EResourceType::Metal, 50))
{
    UE_LOG(LogTemp, Log, TEXT("Added 50 metal"));
}
else
{
    UE_LOG(LogTemp, Warning, TEXT("Cannot add metal - storage full or invalid"));
}
```

```blueprint
// В Blueprint
Add Resource (Train Inventory, Metal, 50)
  Branch (Success)
    ├─ True: Print "Added 50 metal"
    └─ False: Print "Storage full!"
```

### Проверка наличия ресурсов

```cpp
// В C++
if (TrainInventory->HasEnoughResource(EResourceType::Wood, 100))
{
    // Craft module
}
```

```blueprint
// В Blueprint
Has Enough Resource (Train Inventory, Wood, 100)
  Branch
    ├─ True: Enable Craft Button
    └─ False: Disable Craft Button
```

### Расход ресурсов при крафте

```cpp
// В C++
bool CraftModule()
{
    // Check if has resources
    if (!TrainInventory->HasEnoughResource(EResourceType::Metal, 50)) return false;
    if (!TrainInventory->HasEnoughResource(EResourceType::Wood, 30)) return false;
    
    // Remove resources
    TrainInventory->RemoveResource(EResourceType::Metal, 50);
    TrainInventory->RemoveResource(EResourceType::Wood, 30);
    
    // Create module
    SpawnModule();
    return true;
}
```

## Создание вариантов Pickup в Blueprint

### BP_MetalPickup
- Parent: `BP_ResourcePickup`
- Static Mesh: Metal_Bar_SM
- Resource Type: `Metal`
- Amount: `10`

### BP_WoodPickup
- Parent: `BP_ResourcePickup`
- Static Mesh: Wood_Log_SM
- Resource Type: `Wood`
- Amount: `5`

### BP_BooksPickup
- Parent: `BP_ResourcePickup`
- Static Mesh: Book_Stack_SM
- Resource Type: `Books`
- Amount: `1`

## Debug функции

### Вывод всех ресурсов

```cpp
// В C++
void DebugPrintResources()
{
    TMap<EResourceType, int32> AllResources = TrainInventory->GetAllResources();
    
    for (auto& Pair : AllResources)
    {
        FString TypeName = UEnum::GetValueAsString(Pair.Key);
        UE_LOG(LogTemp, Log, TEXT("%s: %d"), *TypeName, Pair.Value);
    }
}
```

### Console Commands (для добавления)

Можно добавить в `RailsTrain.cpp`:

```cpp
// Add console command for testing
if (GEngine && GEngine->GameViewport)
{
    GEngine->GameViewport->ViewportConsole->ConsoleCommand(TEXT("AddMetal 100"));
}
```

## Расширение системы

### Добавление новых ресурсов

1. В `ResourceTypes.h` добавьте новый тип:
```cpp
enum class EResourceType : uint8
{
    None,
    Metal,
    Wood,
    Books,
    Electronics  // NEW
};
```

2. В `ResourceInventoryComponent.cpp` конструкторе инициализируйте:
```cpp
Resources.Add(EResourceType::Electronics, 0);
```

3. Создайте Blueprint вариант `BP_ElectronicsPickup`

## Совместимость

- **Unreal Engine:** 5.3+
- **Networking:** Ready for replication (добавьте `Replicated` UPROPERTY при необходимости)
- **Save System:** Используйте `GetAllResources()` для сохранения

## Производительность

- Компонент не использует Tick
- Операции с TMap: O(1) для Add/Get
- События используют multicast delegates (эффективно)
- Рекомендуется для 100+ типов ресурсов без проблем

## Версия документации

- **EpochRails Version:** 0.0.23
- **Last Updated:** 2025-12-12
- **Language:** Russian (Русский)
