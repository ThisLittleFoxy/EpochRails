# Interaction System - Train Interaction & Seating

## Overview

This is a Blueprint-only interaction system that allows the player to:
- Interact with the train
- Sit in the driver's seat
- Control the train (forward/backward/stop)
- Exit the train

## Components

### 1. BP_InteractableInterface (Blueprint Interface)

**Location:** `Content/Interaction/Interfaces/BP_InteractableInterface`

**Functions:**
- `OnInteract` (Input: Actor Player) - Called when player interacts
- `GetInteractionPrompt` (Output: Text) - Returns interaction text (e.g., "Press E to enter")
- `CanInteract` (Input: Actor Player, Output: Boolean) - Checks if interaction is possible

---

### 2. BP_TrainSeat (Actor Component)

**Location:** `Content/Interaction/Components/BP_TrainSeat`

**Purpose:** Manages the seat in the train cabin

**Variables:**
- `SeatMesh` (Static Mesh Component) - Reference to seat mesh
- `SeatLocation` (Scene Component) - Exact position where player sits
- `IsOccupied` (Boolean) - Is seat currently occupied
- `SeatedCharacter` (Actor) - Reference to seated character
- `SittingAnimation` (Animation Sequence) - Sitting animation

**Functions:**
- `SitCharacter` (Input: Actor Character) - Seats the character
  - Disables character movement
  - Attaches character to SeatLocation
  - Plays sitting animation
  - Sets IsOccupied to true
  
- `ExitSeat` - Character exits seat
  - Detaches character
  - Enables character movement
  - Spawns character near train
  - Sets IsOccupied to false

---

### 3. BP_TrainInteraction (Actor Component)

**Location:** `Content/Interaction/Components/BP_TrainInteraction`

**Purpose:** Manages train interaction logic

**Implements:** `BP_InteractableInterface`

**Variables:**
- `OwnerTrain` (Actor) - Reference to parent train actor
- `TrainSeat` (BP_TrainSeat Component) - Reference to seat component
- `InteractionPrompt` (Text) - Default: "Press E to Enter Train"
- `ExitPrompt` (Text) - Default: "Press E to Exit Train"
- `PlayerInside` (Boolean) - Is player currently inside train
- `InteractionRange` (Float) - Default: 200.0

**Functions:**
- `OnInteract` (from Interface)
  - If PlayerInside = false: Call TrainSeat->SitCharacter
  - If PlayerInside = true: Call TrainSeat->ExitSeat
  
- `GetInteractionPrompt` (from Interface)
  - Returns InteractionPrompt if not inside
  - Returns ExitPrompt if inside
  
- `CanInteract` (from Interface)
  - Check if player is in range
  - Check if seat is not occupied (when entering)
  - Return true/false

---

### 4. BP_TrainController (Actor Component)

**Location:** `Content/Interaction/Components/BP_TrainController`

**Purpose:** Handles train movement control when player is seated

**Variables:**
- `OwnerTrain` (Actor) - Reference to parent train actor
- `ForwardSpeed` (Float) - Default: 500.0
- `BackwardSpeed` (Float) - Default: -300.0
- `IsControlEnabled` (Boolean) - Can player control train

**Functions:**
- `EnableControl` - Activates train control
  - Binds input actions (Forward, Backward, Stop)
  - Sets IsControlEnabled = true
  
- `DisableControl` - Deactivates train control
  - Unbinds input actions
  - Stops train
  - Sets IsControlEnabled = false
  
- `MoveForward` - Sets train speed to ForwardSpeed
- `MoveBackward` - Sets train speed to BackwardSpeed  
- `StopTrain` - Sets train speed to 0

---

## Setup Instructions

### Step 1: Create Blueprint Interface

1. Create folder: `Content/Interaction/Interfaces/`
2. Create Blueprint Interface: `BP_InteractableInterface`
3. Add functions:
   - `OnInteract(Actor Player)`
   - `GetInteractionPrompt() -> Text`
   - `CanInteract(Actor Player) -> Boolean`

### Step 2: Create Components

1. Create folder: `Content/Interaction/Components/`
2. Create Actor Component: `BP_TrainSeat`
   - Add variables and functions from specification above
   - Implement SitCharacter logic:
     ```
     Event Graph:
     - Get Player Character
     - Cast to ARailsPlayerCharacter
     - Set Actor Enable Input (true)
     - Attach Actor to Component (SeatLocation)
     - Play Animation Montage (SittingAnimation)
     - Set IsOccupied = true
     - Set SeatedCharacter = Player
     ```
   
3. Create Actor Component: `BP_TrainInteraction`
   - Implement Interface: `BP_InteractableInterface`
   - Add all interface functions
   - OnInteract implementation:
     ```
     If PlayerInside == false:
       - Get TrainSeat Component
       - Call SitCharacter(Player)
       - Set PlayerInside = true
       - Enable Train Controller
     Else:
       - Get TrainSeat Component  
       - Call ExitSeat
       - Set PlayerInside = false
       - Disable Train Controller
     ```

4. Create Actor Component: `BP_TrainController`
   - Add input binding logic
   - MoveForward:
     ```
     - Get Owner (Cast to BP_Train)
     - Set CurrentSpeed = ForwardSpeed
     ```

### Step 3: Update BP_Train

1. Open your train Blueprint (e.g., `Content/Train/BP_Train/BP_MyTrain`)
2. Add Components:
   - `BP_TrainSeat` (name: TrainSeat)
   - `BP_TrainInteraction` (name: Interaction)
   - `BP_TrainController` (name: Controller)
   
3. Setup SeatLocation:
   - Add Scene Component as child of train mesh
   - Name it "DriverSeatLocation"
   - Position it at driver seat position
   - In BP_TrainSeat, set SeatLocation reference to this component

4. In Construction Script:
   ```
   - Get Interaction Component
   - Set OwnerTrain = Self
   - Get Controller Component  
   - Set OwnerTrain = Self
   ```

### Step 4: Update BP_PlayerCharacter

1. Open player character Blueprint
2. Add Interaction Detection:
   - Create Sphere Trace in Tick or Timer
   - Check if hit actor implements `BP_InteractableInterface`
   - Store reference to InteractableActor
   - Display interaction prompt on screen

3. Add Input Action "Interact" (E key):
   ```
   Event Graph:
   - Check if InteractableActor is valid
   - Call CanInteract on InteractableActor
   - If true: Call OnInteract(Self)
   ```

### Step 5: Setup Input Mapping

1. Open Project Settings -> Input
2. Add Action Mappings:
   - `Interact` -> E key
   - `TrainForward` -> W key (or reuse existing)
   - `TrainBackward` -> S key
   - `TrainStop` -> Space key

---

## Usage Example

### Player Workflow:

1. **Approach Train**
   - Player walks near train
   - Interaction prompt appears: "Press E to Enter Train"
   
2. **Press E**
   - Player character is attached to seat
   - Sitting animation plays
   - Camera switches to train view (optional)
   - Train controls are enabled
   
3. **Control Train**
   - W: Move forward
   - S: Move backward  
   - Space: Stop train
   
4. **Exit Train**
   - Press E again
   - Prompt: "Press E to Exit Train"
   - Character detaches from seat
   - Character spawns near train
   - Train controls disabled

---

## Customization Options

### Visual Feedback

- Add outline/highlight to train when player is in range
- Use Widget Component to show 3D interaction prompt
- Add particle effects when entering/exiting

### Animation

- Create animation montage for sitting down
- Create animation montage for standing up
- Add blend space for sitting idle animations

### Camera

- Add Camera Component to train
- Switch camera when player sits
- Smooth camera transitions with Timeline

### Sound

- Play sound when entering train
- Play sound when exiting train
- Add ambient train cabin sounds

---

## Troubleshooting

### Player doesn't attach to seat
- Check SeatLocation component is valid
- Verify AttachToComponent settings (KeepWorldPosition = false)
- Check collision settings on seat mesh

### Input not working in train
- Verify Input Action mappings exist
- Check EnableControl is called when sitting
- Ensure player controller has input enabled

### Player can't exit train
- Check ExitSeat function is called
- Verify DetachFromActor is working
- Check spawn location is valid (not inside geometry)

---

## Extension Ideas

1. **Multiple Seats**
   - Add array of BP_TrainSeat components
   - Allow passengers to sit in different seats
   - Only driver seat can control train

2. **Doors System**
   - Add door animation before entering
   - Prevent interaction when doors are closed
   - Add door open/close sounds

3. **UI Dashboard**
   - Create UMG widget for train dashboard
   - Show speed, distance, direction
   - Add buttons for lights, horn, etc.

4. **Networked Multiplayer**
   - Replicate seat occupancy
   - Server authority for train control
   - Sync animations across clients

---

## Blueprint Node Summary

### Key Nodes Used:
- `Attach Actor to Component` - Attach player to seat
- `Detach from Actor` - Detach player from seat
- `Implement Interface` - Add interface to component
- `Cast to [ClassName]` - Type casting
- `Sphere Trace by Channel` - Interaction detection
- `Play Animation Montage` - Play sitting animation
- `Set Actor Enable Input` - Enable/disable input
- `Bind Event to On[Input]` - Dynamic input binding

---

## Performance Notes

- Interaction traces should be done on a timer (0.1s) not every tick
- Use collision channels efficiently
- Disable character tick when seated if not needed
- Cache component references in BeginPlay

---

## License

This interaction system is part of the EpochRails project.
All code and documentation are provided as-is for educational purposes.

---

**Created:** 2025-12-03  
**Author:** AI Assistant  
**Version:** 1.0  
**UE Version:** 5.x compatible
