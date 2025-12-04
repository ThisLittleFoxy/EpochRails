# Interaction System Documentation

## Overview

The Interaction System provides a flexible and Blueprint-friendly way to create interactive objects in your game. It consists of three main components:

1. **IInteractableInterface** - Defines the contract for interactable objects
2. **UInteractionComponent** - Handles detection and interaction logic (attached to player)
3. **AInteractableActor** - Base class for creating interactable objects in Blueprint

## Architecture

```
┌─────────────────────────┐
│  ARailsPlayerCharacter  │
│  ┌───────────────────┐  │
│  │ UInteractionComp  │  │
│  │  - Line Trace     │  │
│  │  - Focus Detect   │  │
│  └───────────────────┘  │
└────────────┬────────────┘
             │ Line Trace
             │ (Visibility Channel)
             ↓
   ┌─────────────────────┐
   │ IInteractableInterface│
   │ ┌─────────────────┐ │
   │ │ AInteractableAct│ │
   │ │ or Blueprint    │ │
   │ │ Implementation  │ │
   │ └─────────────────┘ │
   └─────────────────────┘
```

## Setup Guide

### 1. Player Character Setup (Already Done in C++)

The `ARailsPlayerCharacter` already has the `UInteractionComponent` added. You just need to:

1. Open your player character Blueprint (BP_PlayerCharacter)
2. Add an Input Action for "Interact" (e.g., IA_Interact)
3. In the Input Mapping Context, bind it to a key (e.g., "E")
4. Assign the Input Action to the `InteractAction` property in your character Blueprint

### 2. Creating an Interactable Object in Blueprint

#### Method 1: Using AInteractableActor (Recommended)

1. Create a new Blueprint based on `AInteractableActor`
2. Add a Static Mesh or other visual components
3. Set the following properties:
   - **Interaction Name**: Display name (e.g., "Wooden Door")
   - **Interaction Action**: Action text (e.g., "Open")
   - **Max Interaction Distance**: Distance in cm (default: 300)
   - **Can Interact**: Whether the object can be interacted with

4. Implement the Blueprint events:
   - **On Focus Begin**: Called when player looks at the object
   - **On Focus End**: Called when player looks away
   - **On Interact**: Called when player presses interact button

**Example Blueprint Event Graph:**

```
Event On Focus Begin
  ├─ Set Material Parameter (Highlight = true)
  └─ Play Sound (Focus Sound)

Event On Focus End
  ├─ Set Material Parameter (Highlight = false)
  └─ Stop Sound

Event On Interact
  ├─ Branch: Is Door Open?
  │  ├─ True: Close Door
  │  │   ├─ Play Animation (Close)
  │  │   ├─ Play Sound (Close Sound)
  │  │   └─ Set Door Open = false
  │  └─ False: Open Door
  │      ├─ Play Animation (Open)
  │      ├─ Play Sound (Open Sound)
  │      └─ Set Door Open = true
  └─ Return true (interaction successful)
```

#### Method 2: Implementing Interface Directly

1. Create a new Actor Blueprint
2. Go to Class Settings → Interfaces
3. Add `InteractableInterface`
4. Implement all interface functions:
   - `Get Interaction Name`
   - `Get Interaction Action`
   - `Can Interact`
   - `Get Interaction Distance`
   - `On Interaction Focus Begin`
   - `On Interaction Focus End`
   - `On Interact`

### 3. Configuring Interaction Component

You can adjust these settings in your player character Blueprint:

- **Default Interaction Distance**: Maximum reach distance (default: 300 cm)
- **Interaction Check Frequency**: How often to check for objects (default: 0.1s)
- **Interaction Trace Channel**: Collision channel to use (default: Visibility)
- **Show Debug Trace**: Enable visual debugging (shows line trace)
- **Debug Trace Duration**: How long debug lines persist (default: 0.1s)

## Usage Examples

### Example 1: Simple Pickup Item

**Properties:**
- Interaction Name: "Health Potion"
- Interaction Action: "Pickup"
- Can Interact: true
- Max Interaction Distance: 200

**On Interact Event:**
```
Event On Interact
  ├─ Add Item to Inventory (Health Potion)
  ├─ Play Sound (Pickup Sound)
  ├─ Destroy Actor
  └─ Return true
```

### Example 2: Door with Key Requirement

**Properties:**
- Interaction Name: "Locked Door"
- Interaction Action: "Unlock"
- Can Interact: true
- Variables: `bIsLocked` (bool), `RequiredKeyID` (int)

**Can Interact Function:**
```
Can Interact (override)
  ├─ Branch: Is Locked?
  │  ├─ True: Player Has Key?
  │  │   ├─ True: Return true
  │  │   └─ False: Return false
  │  └─ False: Return true
  └─ Return result
```

**On Interact Event:**
```
Event On Interact
  ├─ Branch: Is Locked?
  │  ├─ True:
  │  │   ├─ Remove Key from Inventory
  │  │   ├─ Set Is Locked = false
  │  │   ├─ Set Interaction Name = "Door"
  │  │   ├─ Set Interaction Action = "Open"
  │  │   ├─ Play Sound (Unlock Sound)
  │  │   └─ Return true
  │  └─ False:
  │      ├─ Play Animation (Open Door)
  │      ├─ Play Sound (Open Sound)
  │      └─ Return true
  └─ Return result
```

### Example 3: NPC Conversation Trigger

**Properties:**
- Interaction Name: "Village Elder"
- Interaction Action: "Talk"
- Can Interact: true
- Max Interaction Distance: 250

**On Focus Begin Event:**
```
Event On Focus Begin
  ├─ Make NPC Look at Player
  └─ Play Animation (Wave)
```

**On Interact Event:**
```
Event On Interact
  ├─ Show Dialogue Widget
  ├─ Start Dialogue Tree
  ├─ Set Player Input Mode (UI Only)
  └─ Return true
```

**On Focus End Event:**
```
Event On Focus End
  └─ Play Animation (Idle)
```

## Blueprint Functions Reference

### InteractionComponent (Player)

**Functions available in Blueprint:**

```cpp
// Check if player is looking at an interactable object
bool HasFocusedActor()

// Get the actor currently in focus
AActor* GetFocusedActor()

// Get the name of the focused object
FText GetFocusedActorName()

// Get the action text of the focused object
FText GetFocusedActorAction()

// Check if focused object can be interacted with
bool CanInteractWithFocusedActor()

// Attempt to interact with focused object
bool TryInteract()
```

**Usage Example in HUD Widget:**
```
Event Tick
  ├─ Get Interaction Component (from Player)
  ├─ Has Focused Actor?
  │  ├─ True:
  │  │   ├─ Get Focused Actor Name → Set Text (Object Name)
  │  │   ├─ Get Focused Actor Action → Format String "Press E to {0}"
  │  │   ├─ Set Visibility (Interaction Prompt = Visible)
  │  │   └─ Can Interact With Focused Actor?
  │  │       ├─ True: Set Color (White)
  │  │       └─ False: Set Color (Gray)
  │  └─ False:
  │      └─ Set Visibility (Interaction Prompt = Hidden)
  └─ End
```

### InteractableInterface (Objects)

**Functions to implement:**

```cpp
// Called when player starts looking at object
void OnInteractionFocusBegin(ARailsPlayerCharacter* PlayerCharacter)

// Called when player stops looking at object
void OnInteractionFocusEnd(ARailsPlayerCharacter* PlayerCharacter)

// Called when player presses interact button
bool OnInteract(ARailsPlayerCharacter* PlayerCharacter)

// Get display name
FText GetInteractionName()

// Get action text
FText GetInteractionAction()

// Check if can be interacted with
bool CanInteract(ARailsPlayerCharacter* PlayerCharacter)

// Get interaction distance
float GetInteractionDistance()
```

## Advanced Features

### Custom Interaction Distance Per Object

Each interactable can override the default interaction distance:

```cpp
float GetInteractionDistance_Implementation() const override {
  // Large object = larger interaction distance
  return 500.0f; // 5 meters
}
```

### Conditional Interaction

Control when an object can be interacted with:

```cpp
bool CanInteract_Implementation(ARailsPlayerCharacter* PlayerCharacter) const override {
  // Check player state, inventory, quest progress, etc.
  if (PlayerCharacter->HasItem(RequiredItem)) {
    return true;
  }
  return false;
}
```

### Visual Feedback on Focus

```cpp
void OnInteractionFocusBegin_Implementation(ARailsPlayerCharacter* PlayerCharacter) override {
  // Highlight the object
  if (MeshComponent) {
    MeshComponent->SetRenderCustomDepth(true);
    MeshComponent->SetCustomDepthStencilValue(1);
  }
}

void OnInteractionFocusEnd_Implementation(ARailsPlayerCharacter* PlayerCharacter) override {
  // Remove highlight
  if (MeshComponent) {
    MeshComponent->SetRenderCustomDepth(false);
  }
}
```

### Multiple Interaction States

Implement state machines in Blueprint:

```
Variable: InteractionState (enum)
  - Locked
  - Unlocked
  - Open
  - Broken

Get Interaction Name (override)
  └─ Switch on InteractionState
      ├─ Locked → Return "Locked Door"
      ├─ Unlocked → Return "Door"
      ├─ Open → Return "Open Door"
      └─ Broken → Return "Broken Door"

Get Interaction Action (override)
  └─ Switch on InteractionState
      ├─ Locked → Return "Unlock"
      ├─ Unlocked → Return "Open"
      ├─ Open → Return "Close"
      └─ Broken → Return "Repair"

Can Interact (override)
  └─ Return (InteractionState != Broken)
```

## Debugging

### Enable Debug Visualization

1. Select your player character in the level
2. Find `InteractionComponent`
3. Enable `Show Debug Trace`
4. Green line = hit something
5. Red line = no hit

### Enable Per-Object Debug Logs

In your interactable actor Blueprint:
1. Set `Enable Debug Log = true`
2. Check Output Log for interaction events

### Common Issues

**Problem:** Interaction not detected
- Check collision: Object must have collision on Visibility channel
- Check distance: Is object within `DefaultInteractionDistance`?
- Check `CanInteract`: Does it return true?

**Problem:** Wrong object detected
- Adjust `InteractionTraceChannel`
- Check object collision settings
- Reduce `DefaultInteractionDistance`

**Problem:** Input not working
- Verify `InteractAction` is assigned in player Blueprint
- Check Input Mapping Context includes the action
- Ensure key is bound to the action

## Performance Considerations

- **Interaction Check Frequency**: Default 0.1s (10 checks/second) is optimal
- **Line Trace**: Uses simple line trace, very cheap
- **Focus Events**: Only called when focus changes, not every frame
- **Distance Culling**: Objects beyond interaction distance are ignored

## Best Practices

1. **Use AInteractableActor base class** - Simplifies implementation
2. **Provide clear feedback** - Visual/audio cues on focus and interaction
3. **Meaningful names** - Use descriptive interaction names and actions
4. **State management** - Update interaction text based on object state
5. **Distance tuning** - Adjust per object type (small items = shorter, large = longer)
6. **Collision setup** - Ensure proper collision channels for detection
7. **Error handling** - Return appropriate values from interface functions

## Integration with UI

Example HUD widget setup:

```
Widget Blueprint: WBP_InteractionPrompt

Components:
  ├─ Canvas Panel
  │   └─ Vertical Box (Center-Bottom)
  │       ├─ Text Block: ObjectName
  │       └─ Text Block: ActionPrompt ("Press E to {Action}")

Event Graph:
  Event Tick
    ├─ Get Player Character
    ├─ Get Interaction Component
    ├─ Has Focused Actor?
    │   ├─ True:
    │   │   ├─ Get Focused Actor Name → Set ObjectName
    │   │   ├─ Get Focused Actor Action → Format ActionPrompt
    │   │   ├─ Set Visibility: Visible
    │   │   └─ Can Interact?
    │   │       ├─ True: Set Opacity 1.0
    │   │       └─ False: Set Opacity 0.5
    │   └─ False:
    │       └─ Set Visibility: Hidden
    └─ End
```

## Future Enhancements

Potential additions to the system:

1. **Hold-to-interact** - Require button hold for interaction
2. **Multiple interaction options** - Context menu for objects
3. **Interaction animations** - Play character animation on interact
4. **Interaction cooldowns** - Prevent spam interactions
5. **Network replication** - Multiplayer support
6. **Interaction zones** - Box/sphere triggers instead of line trace
7. **Priority system** - Handle overlapping interactables
8. **Interaction history** - Track what player has interacted with

## Credits

Developed for EpochRails project
Compatible with Unreal Engine 5.x


## Animation Functions (Функции для анимаций)

### UInteractionComponent

```cpp
// Проверить, тчка на объект в фокусе
UFUNCTION(BlueprintPure, Category = "Interaction Animation")
bool HasFocusedActor() const;

// Нормализованное расстояние до объекта
UFUNCTION(BlueprintPure, Category = "Interaction Animation")
float GetFocusedActorDistanceNormalized() const;

// Можно ли взаимодействовать
UFUNCTION(BlueprintPure, Category = "Interaction Animation")
bool CanInteractWithFocusedActor() const;
```

### AInteractableActor

```cpp
// Проверить, ст объект в фокусе
UFUNCTION(BlueprintPure, Category = "Interaction Animation")
bool IsFocused() const;

// Проверить, идёт ли обработка взаимодействия
UFUNCTION(BlueprintPure, Category = "Interaction Animation")
bool IsBeingInteractedWith() const;

// Анимация при начале взаимодействия
UFUNCTION(BlueprintImplementableEvent, Category = "Interaction Animation")
void OnInteractionStarted();

// Анимация при конце взаимодействия
UFUNCTION(BlueprintImplementableEvent, Category = "Interaction Animation")
void OnInteractionEnded();
```

## Last Updated
- **Date:** 2025-12-04
- **Version:** 0.0.22
