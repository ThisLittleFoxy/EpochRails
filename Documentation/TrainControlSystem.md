# Train Control System - Complete Blueprint Guide

## Overview

This system integrates a driveable seat with your **existing RailsTrain class** using only Blueprint. The player can sit in the driver's seat and control the train's speed using the train's existing C++ API.

**Key Point**: This system does NOT replace or duplicate RailsTrain. It only adds player interaction capabilities through Blueprint.

## Architecture

### System Components

```
[Player Character]
       |
       | (Interacts with E)
       v
[BP_TrainDriverSeat] (InteractableActor)
       |
       | (Calls C++ API)
       v
[ARailsTrain] (Your existing C++ class)
       |
       | (Moves along)
       v
[ARailsSplinePath] (Your existing spline system)
```

### Integration Points

**From Blueprint to C++:**
- `BP_TrainDriverSeat` calls `RailsTrain->SetSpeed(float)`
- Seat calls `RailsTrain->StartTrain()` / `StopTrain()`
- Seat reads `RailsTrain->GetCurrentSpeed()`

**No C++ Changes Needed:**
- RailsTrain.h - unchanged
- RailsTrain.cpp - unchanged
- All existing functionality preserved

## Implementation Guide

### Phase 1: Input System Setup

#### Step 1.1: Create Input Actions

Create these Enhanced Input Actions in `Content/Input/`:

**IA_ExitSeat**
- Type: Input Action
- Value Type: Digital (bool)
- Description: Exit driver seat

**IA_TrainThrottle**
- Type: Input Action  
- Value Type: Axis1D (float)
- Description: Control train speed

**IA_TrainBrake**
- Type: Input Action
- Value Type: Digital (bool)
- Description: Emergency brake

#### Step 1.2: Create Input Mapping Context

**IMC_TrainControl**
- Type: Input Mapping Context

Add Mappings:
1. **IA_ExitSeat**
   - Key: E (Keyboard)
   - Modifiers: None

2. **IA_TrainThrottle**
   - Key: W (Keyboard)
   - Modifiers: Scale (1.0)
   - Key: S (Keyboard)  
   - Modifiers: Scale (-1.0)

3. **IA_TrainBrake**
   - Key: Space (Keyboard)
   - Modifiers: None

### Phase 2: Driver Seat Blueprint

#### Step 2.1: Create Blueprint

**File**: `Content/Train/BP_TrainDriverSeat`

**Settings:**
- Parent Class: `InteractableActor`
- Class Settings:
  - Tick: Disabled (not needed)
  - Replicates: False (single player for now)

#### Step 2.2: Add Components

**Component Hierarchy:**
```
BP_TrainDriverSeat (InteractableActor)
├─ DefaultSceneRoot (inherited)
├─ SeatMesh (StaticMeshComponent)
│  ├─ Collision: BlockAll or QueryOnly
│  └─ Visibility Collision: Block (for interaction raycast)
│
├─ SitPosition (SceneComponent)
│  └─ Location: Center of seat, slightly above surface
│
└─ CameraPosition (SceneComponent)
   ├─ Location: Behind and above seat
   └─ Rotation: Angled slightly downward
```

**SeatMesh Setup:**
- Static Mesh: Choose seat mesh
- Material: Your seat material
- Collision Preset: Custom
  - Collision Enabled: Query Only
  - Object Type: WorldStatic
  - Visibility: Block (required for interaction)
  - Camera: Ignore

**SitPosition Setup:**
- Location: (0, 0, 50) relative to seat
- Rotation: (0, 0, 0)
- This is where player will be attached

**CameraPosition Setup:**  
- Location: (-200, 0, 150) relative to seat
- Rotation: (-15, 0, 0)
- This gives nice over-shoulder view

#### Step 2.3: Create Variables

**Blueprint Variables:**

```
OwningTrain
  Type: RailsTrain (Object Reference)
  Category: Train Control
  Instance Editable: No
  Tooltip: Reference to the train this seat belongs to
  Default: None

SeatedPlayer
  Type: EpochRailsCharacter (Object Reference)
  Category: Seat State
  Instance Editable: No
  Tooltip: Currently seated player
  Default: None

bIsOccupied
  Type: Boolean
  Category: Seat State
  Instance Editable: No
  Tooltip: Is someone currently sitting?
  Default: false

TargetSpeed
  Type: Float
  Category: Control
  Instance Editable: No
  Tooltip: Target speed set by player input
  Default: 0.0

MaxControlSpeed
  Type: Float
  Category: Control
  Instance Editable: Yes
  Tooltip: Maximum speed player can set
  Default: 2000.0
```

**InteractableActor Properties (Details Panel):**
- Interaction Name: "Driver Seat"
- Interaction Action: "Sit"
- Max Interaction Distance: 150.0
- Can Interact: true
- Enable Debug Log: false (enable for testing)

### Phase 3: Blueprint Graph Implementation

#### Event Graph

##### Event: BeginPlay

**Purpose**: Find and store reference to owning train

**Implementation:**
```
Event BeginPlay
  │
  ├── Get Owner
  │   |
  │   v
  ├── Cast to RailsTrain
  │   |
  │   +--> [Success]
  │   |      |
  │   |      v
  │   |    Set Owning Train (store reference)
  │   |      |
  │   |      v
  │   |    Print String ("Seat initialized for train: {train name}")
  │   |
  │   +--> [Failed]
  │          |
  │          v
  │        Print String ("ERROR: Seat must be attached to RailsTrain!")
  │          |
  │          v
  │        Print String (Color: Red, Duration: 10.0)
```

**Nodes:**
1. Event BeginPlay
2. Get Owner (inherited from Actor)
3. Cast to RailsTrain
4. Branch (implicit in Cast)
5. Set Owning Train (variable)
6. Print String (for debug)

##### Override: Can Interact

**Purpose**: Prevent interaction when seat is occupied

**Implementation:**
```
Can Interact (Override)
  │
  ├── Branch: Is Occupied?
  │   |
  │   +--> True: Return false
  │   |
  │   +--> False: Return true
```

**Return Type**: Boolean

##### Override: Get Interaction Action

**Purpose**: Change interaction text based on state

**Implementation:**
```
Get Interaction Action (Override)
  │
  ├── Branch: Is Occupied?
  │   |
  │   +--> True: Return "Occupied"
  │   |
  │   +--> False: Return "Sit"
```

**Return Type**: String

##### Event: On Interact

**Purpose**: Handle player interaction (E key press)

**Implementation:**
```
Event On Interact
  │
  ├── Branch: Is Occupied?
  │   |
  │   +--> True:
  │   |      |
  │   |      v
  │   |    Return false (cannot sit)
  │   |
  │   +--> False:
  │          |
  │          v
  │        Get Instigator (from event)
  │          |
  │          v
  │        Cast to EpochRailsCharacter
  │          |
  │          v
  │        Call Function: Sit Player (PlayerCharacter)
  │          |
  │          v
  │        Return true (interaction success)
```

**Input**: Instigator (Actor)
**Return Type**: Boolean

#### Function: Sit Player

**Purpose**: Attach player to seat and enable train controls

**Access**: Private  
**Pure**: No  
**Inputs**: 
- Player (EpochRailsCharacter Reference)

**Implementation:**
```
Sit Player
  │
  ├── [1] Store Reference
  │   |
  │   v
  │   Set Seated Player = Player
  │   Set Is Occupied = true
  │
  ├── [2] Disable Player Movement
  │   |
  │   v
  │   Player -> Get Controller
  │   |
  │   v
  │   Cast to PlayerController
  │   |
  │   v
  │   Disable Input (Player Controller)
  │
  ├── [3] Attach Player to Seat
  │   |
  │   v
  │   Attach Actor to Component
  │     Inputs:
  │       - Actor: Seated Player
  │       - Parent: Sit Position (component)
  │       - Socket Name: None
  │       - Location Rule: Snap to Target
  │       - Rotation Rule: Snap to Target
  │       - Scale Rule: Keep World
  │       - Weld Simulated Bodies: false
  │
  ├── [4] Switch Camera
  │   |
  │   v
  │   Get Player Controller
  │   |
  │   v
  │   Get Player Camera Manager
  │   |
  │   v
  │   Set View Target with Blend
  │     Inputs:
  │       - New View Target: Self (BP_TrainDriverSeat)
  │       - Blend Time: 0.5
  │       - Blend Function: EViewTargetBlendFunction::VTBlend_Cubic
  │       - Blend Exp: 2.0
  │       - Lock Outgoing: false
  │
  ├── [5] Wait for Camera Transition
  │   |
  │   v
  │   Delay (0.5 seconds)
  │
  ├── [6] Enable Train Controls
  │   |
  │   v
  │   Get Player Controller
  │   |
  │   v
  │   Get Enhanced Input Local Player Subsystem
  │     Input: Player Controller
  │   |
  │   v
  │   Add Mapping Context
  │     Inputs:
  │       - Mapping Context: IMC_TrainControl
  │       - Priority: 1
  │       - Options: (default)
  │   |
  │   v
  │   Enable Input (Player Controller)
  │
  ├── [7] Start Train
  │   |
  │   v
  │   Owning Train -> Start Train
  │   |
  │   v
  │   Print String ("Train started by player")
  │
  └── [8] Update Interaction Text
      |
      v
      Set Interaction Action = "Exit (E)"
```

**Key Nodes:**
- **Attach Actor to Component**: Physically attaches player to seat
- **Set View Target with Blend**: Smooth camera transition
- **Add Mapping Context**: Enables train control inputs
- **Start Train**: Begins train movement via C++ API

#### Function: Exit Player

**Purpose**: Detach player from seat and restore normal controls

**Access**: Private  
**Pure**: No

**Implementation:**
```
Exit Player
  │
  ├── [0] Validation
  │   |
  │   v
  │   Branch: Seated Player Is Valid?
  │   |
  │   +--> False: Return (nothing to exit)
  │
  ├── [1] Stop Train
  │   |
  │   v
  │   Set Target Speed = 0.0
  │   |
  │   v
  │   Owning Train -> Set Speed (0.0)
  │   |
  │   v
  │   Print String ("Train stopped by player")
  │
  ├── [2] Disable Train Controls
  │   |
  │   v
  │   Get Player Controller (from Seated Player)
  │   |
  │   v
  │   Disable Input (Player Controller)
  │   |
  │   v
  │   Get Enhanced Input Local Player Subsystem
  │   |
  │   v
  │   Remove Mapping Context
  │     Input: IMC_TrainControl
  │
  ├── [3] Restore Camera
  │   |
  │   v
  │   Get Player Camera Manager
  │   |
  │   v
  │   Set View Target with Blend
  │     Inputs:
  │       - New View Target: Seated Player
  │       - Blend Time: 0.5
  │       - Blend Function: VTBlend_Cubic
  │
  ├── [4] Wait for Camera Transition
  │   |
  │   v
  │   Delay (0.5 seconds)
  │
  ├── [5] Detach Player
  │   |
  │   v
  │   Detach from Actor
  │     Inputs:
  │       - Location Rule: Keep World
  │       - Rotation Rule: Keep World
  │       - Scale Rule: Keep World
  │
  ├── [6] Position Player Safely
  │   |
  │   v
  │   Get Actor Forward Vector (from Self)
  │   |
  │   v
  │   Multiply (Forward Vector * 100.0)
  │   |
  │   v
  │   Get Actor Location (from Seated Player)
  │   |
  │   v
  │   Add (Location + Offset)
  │   |
  │   v
  │   Set Actor Location (Seated Player, New Location, Sweep: true)
  │
  ├── [7] Restore Player Input
  │   |
  │   v
  │   Get Controller (from Seated Player)
  │   |
  │   v
  │   Cast to PlayerController
  │   |
  │   v
  │   Enable Input (Player Controller)
  │
  ├── [8] Clear State
  │   |
  │   v
  │   Set Seated Player = None
  │   Set Is Occupied = false
  │   Set Target Speed = 0.0
  │   Set Interaction Action = "Sit"
  │   |
  │   v
  │   Print String ("Player exited driver seat")
```

**Key Steps:**
1. Stop train before exit
2. Remove input context
3. Restore camera view
4. Detach player safely
5. Move player forward 100 units
6. Restore normal input

#### Input Event: Exit Seat

**Enhanced Input Action**: IA_ExitSeat

**Event**: Started

```
IA_ExitSeat (Started)
  │
  └── Call Function: Exit Player
```

**Simple**: Just calls Exit Player function

#### Input Event: Train Throttle

**Enhanced Input Action**: IA_TrainThrottle

**Event**: Triggered

```
IA_TrainThrottle (Triggered)
  │
  ├── Get Action Value (float)
  │   |
  │   v
  │   Store in: Throttle Input (local variable)
  │
  ├── Branch: Throttle Input > 0.1?
  │   |
  │   +--> True: [FORWARD ACCELERATION]
  │   |      |
  │   |      v
  │   |    Set Target Speed = Max Control Speed * Throttle Input
  │   |      |
  │   |      v
  │   |    Clamp (Target Speed, 0.0, Max Control Speed)
  │   |      |
  │   |      v
  │   |    Owning Train -> Set Speed (Target Speed)
  │   |      |
  │   |      v
  │   |    Owning Train -> Start Train (if stopped)
  │   |
  │   +--> False: Branch: Throttle Input < -0.1?
  │          |
  │          +--> True: [BACKWARD/SLOW]
  │          |      |
  │          |      v
  │          |    Get Current Speed (from Owning Train)
  │          |      |
  │          |      v
  │          |    Set Target Speed = Current Speed * 0.7
  │          |      |
  │          |      v
  │          |    Clamp (Target Speed, 0.0, Current Speed)
  │          |      |
  │          |      v
  │          |    Owning Train -> Set Speed (Target Speed)
  │          |
  │          +--> False: [COAST/NEUTRAL]
  │                 |
  │                 v
  │               Get Current Speed (from Owning Train)
  │                 |
  │                 v
  │               Set Target Speed = Current Speed * 0.95
  │                 |
  │                 v
  │               Owning Train -> Set Speed (Target Speed)
```

**Logic:**
- **W key (positive)**: Accelerate forward
- **S key (negative)**: Slow down gradually  
- **No input**: Coast (natural deceleration)

#### Input Event: Train Brake

**Enhanced Input Action**: IA_TrainBrake

**Event**: Triggered

```
IA_TrainBrake (Triggered)
  │
  ├── Set Target Speed = 0.0
  │
  ├── Owning Train -> Stop Train
  │   |
  │   v
  │   Print String ("Emergency brake applied!")
  │
  └── [Optional] Play Sound: Brake Sound
```

**Effect**: Immediate stop using RailsTrain's deceleration system

### Phase 4: Camera Setup

#### Camera Component

The seat itself acts as the view target. Add Camera component (optional):

**Option A: Use CameraPosition Scene Component (Recommended)**
- CameraPosition determines view angle
- No Camera component needed
- Player Camera Manager handles view

**Option B: Add Camera Component**
1. Add Component → Camera
2. Attach to CameraPosition
3. Set as default camera

**CameraPosition Location Examples:**

```
// Third Person View (behind seat)
Location: (-250, 0, 180)
Rotation: (-20, 0, 0)

// First Person View (at seat)
Location: (0, 0, 120)
Rotation: (0, 0, 0)

// Overhead View
Location: (0, 0, 400)
Rotation: (-60, 0, 0)
```

### Phase 5: Integration with Train

#### Method A: Child Actor Component (Recommended)

**In your train Blueprint (child of RailsTrain):**

1. Open train Blueprint
2. Add Component → Child Actor
3. Name: "DriverSeat"
4. Details Panel:
   - Child Actor Class: BP_TrainDriverSeat
5. Position in viewport:
   - Location: Where driver sits (e.g., front of train)
   - Rotation: Facing forward
6. Compile and Save

**Benefits:**
- Seat automatically moves with train
- Owning Train reference auto-set in BeginPlay
- Easy to position in editor

#### Method B: Manual Attachment

**In level:**

1. Place BP_TrainDriverSeat in level
2. Place your train (child of RailsTrain)
3. World Outliner:
   - Drag BP_TrainDriverSeat onto train
   - Creates parent-child relationship
4. Position seat in viewport

**Note**: Verify BeginPlay finds Owning Train correctly

### Phase 6: Testing and Debugging

#### Debug Helpers

**Add Debug Display (Optional):**

In BP_TrainDriverSeat, enable Tick and add:

```
Event Tick
  │
  ├── Branch: Is Occupied?
  │   |
  │   +--> True:
  │          |
  │          v
  │        Get Current Speed (from Owning Train)
  │          |
  │          v
  │        Divide by 27.78 (convert to km/h)
  │          |
  │          v
  │        Format Text: "{0} km/h"
  │          |
  │          v
  │        Get Actor Location + (0, 0, 200)
  │          |
  │          v
  │        Draw Debug String
  │          Inputs:
  │            - Text: Speed Text
  │            - Location: Above Seat
  │            - Color: Green
  │            - Duration: 0.0
```

#### Testing Checklist

**Pre-Flight:**
- [ ] BP_TrainDriverSeat parent is InteractableActor
- [ ] Seat has SeatMesh with Visibility collision
- [ ] SitPosition and CameraPosition components exist
- [ ] Input Actions created (3 actions)
- [ ] IMC_TrainControl created with mappings
- [ ] Seat is child of train (Child Actor or attached)

**Basic Functionality:**
- [ ] Can walk to seat
- [ ] Interaction prompt appears ("Sit")
- [ ] E key sits player down
- [ ] Camera transitions smoothly (0.5s)
- [ ] Player is attached to seat
- [ ] Train starts moving

**Controls:**
- [ ] W key increases speed
- [ ] S key decreases speed  
- [ ] Space key stops train
- [ ] No input causes coasting
- [ ] Speed is clamped to MaxControlSpeed

**Exit:**
- [ ] E key exits seat
- [ ] Train stops (speed → 0)
- [ ] Camera returns to player
- [ ] Player detaches from seat
- [ ] Player positioned 100 units forward
- [ ] Normal controls restored

**Edge Cases:**
- [ ] Cannot sit when occupied
- [ ] Interaction text changes ("Occupied")
- [ ] Seat works with train on spline
- [ ] Player stays attached during turns
- [ ] Exit works at any speed

#### Common Issues

**Issue 1: Owning Train is None**

**Symptoms:**
- Print error in BeginPlay
- Controls don't affect train

**Solutions:**
1. Verify seat is child of RailsTrain actor
2. Check Get Owner returns RailsTrain
3. Use Child Actor Component method
4. Check BeginPlay executes (add Print String)

**Issue 2: Camera doesn't move**

**Symptoms:**
- Camera stays on player
- No camera transition

**Solutions:**
1. Verify CameraPosition component exists
2. Check Set View Target is called
3. Ensure blend time > 0
4. Test with immediate blend (time = 0.01)
5. Check Player Camera Manager is valid

**Issue 3: Player falls through train**

**Symptoms:**
- Player detaches during movement
- Player positioned below train

**Solutions:**
1. Check PlatformMesh in RailsTrain has collision
2. Verify SitPosition is above platform surface
3. Check Attach Actor uses "Snap to Target"
4. Ensure RailsTrain PlatformMesh collision is enabled

**Issue 4: Controls don't work**

**Symptoms:**
- W/S/Space do nothing
- Train doesn't respond

**Solutions:**
1. Check IMC_TrainControl is added in Sit Player
2. Verify Input Actions are bound (compile Blueprint)
3. Ensure Enable Input is called
4. Check Input Mapping Context priority (should be 1+)
5. Test with Print String in input events

**Issue 5: Can't exit seat**

**Symptoms:**
- E key doesn't work
- Player stuck in seat

**Solutions:**
1. Verify IA_ExitSeat is in IMC_TrainControl
2. Check Exit Player function is bound
3. Test E key with Print String
4. Ensure input is not disabled
5. Check Seated Player is valid in Exit function

**Issue 6: Train doesn't stop on exit**

**Symptoms:**
- Train continues moving
- Speed doesn't reset

**Solutions:**
1. Check Stop Train is called in Exit Player
2. Verify Set Speed(0) is called
3. Ensure Owning Train reference is valid
4. Test with Print String before API call

## Advanced Features

### Feature: Speed Limit Zones

**Create**: BP_SpeedLimitZone

**Components:**
- Box Trigger

**Variables:**
- Speed Limit (Float) = 500.0

**Events:**
```
On Actor Begin Overlap
  ├── Cast to RailsTrain
  ├── Set Max Speed = Speed Limit
  └── Display Warning: "Speed Limit: {limit} km/h"

On Actor End Overlap
  ├── Cast to RailsTrain  
  └── Set Max Speed = Original Max Speed
```

### Feature: Train Stations

**Create**: BP_TrainStation

**Components:**
- Trigger Box
- Station Mesh

**Variables:**
- Station Name (String)
- Stop Duration (Float) = 10.0

**Events:**
```
On Actor Begin Overlap
  ├── Cast to RailsTrain
  ├── Check: Is Character On Train? (has seated player)
  ├── Display: "Press F to stop at {Station Name}"
  └── Bind F key:
     ├── Call: Stop Train
     ├── Wait: Stop Duration
     └── Allow departure
```

### Feature: HUD Widget

**Create**: WBP_TrainDriverHUD

**Parent**: User Widget

**Widgets:**
- Text: Speed (km/h)
- Progress Bar: Speed indicator  
- Text: Controls help

**Variables:**
- Owning Seat (BP_TrainDriverSeat Reference)

**Functions:**
```
Event Construct
  └── Set Timer by Function Name ("UpdateHUD", 0.1, looping)

UpdateHUD
  ├── Get Owning Seat -> Owning Train
  ├── Get Current Speed
  ├── Convert to km/h
  ├── Update Text
  └── Update Progress Bar
```

**Integration:**
- In Sit Player: Create Widget → Add to Viewport
- In Exit Player: Remove from Parent

## Performance Considerations

### Optimization Tips

1. **Disable Tick**: Seat doesn't need Tick unless debugging
2. **HUD Updates**: Use timer (0.1s) not Tick
3. **Input Events**: Already optimized (event-driven)
4. **Camera Blending**: Native UE function, efficient

### Memory Usage

- **Seat Blueprint**: ~50 KB
- **Input Assets**: ~10 KB each
- **Runtime**: Minimal (single reference, no physics)

## Multiplayer Considerations

**Current Implementation**: Single player only

**For Multiplayer:**

1. **Replicate Seat State:**
   - Mark bIsOccupied as Replicated
   - Replicate Seated Player reference

2. **Server Authority:**
   - Sit/Exit must be Server RPC
   - Speed control via Server RPC

3. **Client Prediction:**
   - Smooth camera on clients
   - Interpolate speed changes

## Conclusion

You now have a complete system to control your existing RailsTrain using Blueprint-only driver seat. The system:

- ✅ Uses existing RailsTrain C++ class
- ✅ No C++ modifications required
- ✅ Integrates with Interaction System
- ✅ Smooth camera transitions
- ✅ Intuitive controls
- ✅ Easy to extend

**Next Steps:**
1. Create BP_TrainDriverSeat
2. Add to your train
3. Create Input Actions
4. Test in PIE
5. Add HUD (optional)
6. Create stations (optional)

---

**Happy Train Driving! 🚂**
