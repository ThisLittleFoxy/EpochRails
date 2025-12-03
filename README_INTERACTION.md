# Interaction System - Quick Start Guide

## What is it?

A flexible Blueprint-friendly system that allows players to interact with objects in the world by looking at them and pressing a button.

## Features

✅ **Easy to use** - Create interactable objects in Blueprint without writing C++ code  
✅ **Visual feedback** - Automatic detection when player looks at objects  
✅ **Customizable** - Each object can have its own interaction distance, name, and behavior  
✅ **Blueprint events** - Simple events for focus and interaction  
✅ **Debugging tools** - Built-in visualization and logging  

## Quick Setup (3 Steps)

### Step 1: Add Input Action

1. Create new Input Action: `Content/Input/IA_Interact`
2. Set Value Type: `Digital (bool)`
3. Open your Input Mapping Context (e.g., `IMC_Default`)
4. Add mapping: `IA_Interact` → Key `E`
5. Open your player character Blueprint
6. Set `Interact Action` property to `IA_Interact`

### Step 2: Create Interactable Object

1. Create new Blueprint: `Content/Blueprints/BP_MyInteractable`
2. Parent Class: `InteractableActor`
3. Add a Static Mesh component
4. Set properties:
   - Interaction Name: `"Treasure Chest"`
   - Interaction Action: `"Open"`
5. Implement `On Interact` event:
   ```
   Event On Interact
     ├─ Print String: "Chest opened!"
     └─ Return: true
   ```

### Step 3: Place in Level

1. Drag `BP_MyInteractable` into your level
2. Play in Editor (PIE)
3. Look at the object and press `E`
4. Done! 🎉

## Basic Example: Door

**Blueprint: BP_Door (extends InteractableActor)**

**Variables:**
- `bIsOpen` (bool) = false

**Properties:**
- Interaction Name = "Door"
- Interaction Action = "Open" (changes based on state)

**Events:**

```
Event On Focus Begin
  └─ Set Material Parameter (Highlight = 1.0)

Event On Focus End
  └─ Set Material Parameter (Highlight = 0.0)

Event On Interact
  ├─ Branch: Is Open?
  │  ├─ True:
  │  │   ├─ Play Animation: "Door_Close"
  │  │   ├─ Set Is Open = false
  │  │   └─ Set Interaction Action = "Open"
  │  └─ False:
  │      ├─ Play Animation: "Door_Open"
  │      ├─ Set Is Open = true
  │      └─ Set Interaction Action = "Close"
  └─ Return: true
```

## Blueprint Events Explained

### On Focus Begin
Called when player looks at the object. Use for:
- Highlighting the object
- Playing hover sounds
- Showing UI indicators

### On Focus End
Called when player looks away. Use for:
- Removing highlights
- Stopping sounds
- Hiding UI indicators

### On Interact
Called when player presses interact button. Use for:
- Opening doors
- Picking up items
- Starting dialogues
- Activating mechanisms

**Must return `true` if interaction was successful!**

## Useful Blueprint Nodes

From **Player Character**:
- `Get Interaction Component` → Access the interaction system
- `Has Focused Actor` → Check if looking at something
- `Get Focused Actor Name` → Get object's display name
- `Can Interact With Focused Actor` → Check if interaction is possible

From **Interactable Actor**:
- `Get Interaction Name` → Override to change name dynamically
- `Get Interaction Action` → Override to change action text
- `Can Interact` → Override to add conditions (requires key, etc.)
- `Get Interaction Distance` → Override to change detection range

## Common Patterns

### Pattern 1: Item Pickup
```
Event On Interact
  ├─ Add to Inventory (Item)
  ├─ Play Sound (Pickup)
  ├─ Destroy Actor
  └─ Return: true
```

### Pattern 2: Toggle Switch
```
Event On Interact
  ├─ Toggle: bIsActive
  ├─ Branch: Is Active?
  │  ├─ True: Play Animation "On"
  │  └─ False: Play Animation "Off"
  └─ Return: true
```

### Pattern 3: Locked Door
```
Can Interact (override)
  ├─ Branch: Is Locked?
  │  ├─ True: Player Has Key?
  │  └─ False: Return true
  └─ Return result

Event On Interact
  ├─ Branch: Is Locked?
  │  ├─ True:
  │  │   ├─ Use Key
  │  │   └─ Set Is Locked = false
  │  └─ False:
  │      └─ Open Door
  └─ Return: true
```

## Settings You Can Adjust

### On Player Character (Interaction Component):
- **Default Interaction Distance**: How far player can interact (default: 300 cm)
- **Interaction Check Frequency**: How often to check for objects (default: 0.1s)
- **Show Debug Trace**: Enable visual debugging

### On Each Interactable Object:
- **Interaction Name**: Display name (e.g., "Wooden Door")
- **Interaction Action**: What to do (e.g., "Open")
- **Max Interaction Distance**: Override default distance for this object
- **Can Interact**: Enable/disable interaction
- **Enable Debug Log**: Log events for this object

## Creating UI Prompt

**Widget Blueprint: WBP_InteractionPrompt**

1. Add Text Block: "ObjectName"
2. Add Text Block: "ActionText"
3. Add to Viewport on BeginPlay

**Event Tick:**
```
Get Player Character
├─ Get Interaction Component
├─ Has Focused Actor?
│  ├─ True:
│  │   ├─ Get Focused Actor Name → Set ObjectName
│  │   ├─ Get Focused Actor Action → Set ActionText ("Press E to {0}")
│  │   └─ Set Visibility: Visible
│  └─ False:
│      └─ Set Visibility: Hidden
└─ End
```

## Debugging Tips

**Enable Debug Visualization:**
1. Select player character in level
2. Find Interaction Component
3. Check "Show Debug Trace"
4. Green line = hitting object, Red line = no hit

**Enable Object Logging:**
1. Select interactable object
2. Check "Enable Debug Log"
3. Watch Output Log for events

**Common Issues:**
- Object not detected? Check collision on Visibility channel
- Wrong distance? Adjust Max Interaction Distance
- Input not working? Verify Interact Action is assigned

## Performance

✅ **Optimized**: Line trace runs only 10 times per second  
✅ **Efficient**: Focus events only fire on changes  
✅ **Scalable**: Distance culling built-in  

## Next Steps

1. Read full documentation: `Docs/InteractionSystem.md`
2. Check examples: `Content/Examples/Interaction/`
3. Create your first interactable object!
4. Add UI prompt for better UX
5. Experiment with different interaction types

## Need Help?

- Check `Docs/InteractionSystem.md` for detailed guide
- Enable debug visualization
- Check Output Log for errors
- Verify Input Actions are set up correctly

---

**Happy Interacting! 🎮**
