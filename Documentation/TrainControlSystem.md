# Train Control System - Blueprint Implementation Guide

## Overview

This system allows players to sit in a train driver's seat and control the train using only Blueprint. It integrates with the existing Interaction System to provide a seamless experience.

## Features

✅ **Seat Interaction** - Use existing interaction system to sit/exit  
✅ **Camera Transition** - Smooth camera movement when sitting  
✅ **Control Mode** - Toggle between sitting and driving  
✅ **Speed Control** - Accelerate, decelerate, and reverse  
✅ **Visual Feedback** - UI indicators for speed and mode  
✅ **Blueprint Only** - No C++ code required  

## Architecture

### Components

1. **BP_TrainDriverSeat** - Interactable seat actor
2. **BP_TrainController** - Main train control logic
3. **WBP_TrainHUD** - Driver UI widget
4. **Enhanced Input Actions** - Control bindings

### Data Flow

```
Player → Interact with Seat → Attach Player to Seat → Switch Camera → Enable Controls
Player → Exit Control → Detach Player → Restore Camera → Disable Controls
```

## Implementation Guide

### Step 1: Setup Input Actions

Create the following Input Actions in `Content/Input/`:

#### IA_ExitSeat
- **Value Type**: Digital (bool)
- **Mapping**: Key `E` (same as interact)

#### IA_TrainThrottle
- **Value Type**: Axis1D (float)
- **Mapping**: Keys `W` (+1.0) and `S` (-1.0)

#### IA_TrainBrake
- **Value Type**: Digital (bool)
- **Mapping**: Key `Space`

#### IA_TrainReverse
- **Value Type**: Digital (bool)
- **Mapping**: Key `R`

Add these to your Input Mapping Context (`IMC_Default`):
- `IA_ExitSeat` → E
- `IA_TrainThrottle` → W (scale: 1.0), S (scale: -1.0)
- `IA_TrainBrake` → Space
- `IA_TrainReverse` → R

### Step 2: Create Train Driver Seat Blueprint

**Blueprint**: `Content/Train/BP_TrainDriverSeat`  
**Parent Class**: `InteractableActor`

#### Components

Add these components:
- **SeatMesh** (Static Mesh) - Visual representation of the seat
- **SitPosition** (Scene Component) - Player attachment point
- **CameraPosition** (Scene Component) - Camera target when seated

#### Variables

```
Name                    Type                    Default     Description
------------------------------------------------------------------------------------
OwningTrain             BP_TrainController      None        Reference to the train
SeatedPlayer            AEpochRailsCharacter    None        Currently seated player
bIsOccupied             Boolean                 false       Is someone sitting?
SeatInteractionName     String                  "Driver Seat"
SitActionText           String                  "Sit"
ExitActionText          String                  "Exit"
```

#### Properties Setup

In the Details panel:
- **Interaction Name**: "Driver Seat"
- **Interaction Action**: "Sit"
- **Max Interaction Distance**: 150.0

#### Blueprint Events

##### Event BeginPlay
```
Event BeginPlay
  ├─ Get Owner
  ├─ Cast to BP_TrainController
  ├─ Set Owning Train (reference)
  └─ [If cast fails, log warning]
```

##### Can Interact (Override)
```
Can Interact
  ├─ Branch: Is Occupied?
  │  ├─ True:
  │  │   └─ Return false
  │  └─ False:
  │      └─ Return true
```

##### Get Interaction Action (Override)
```
Get Interaction Action
  ├─ Branch: Is Occupied?
  │  ├─ True:
  │  │   └─ Return "Occupied"
  │  └─ False:
  │      └─ Return "Sit"
```

##### Event On Interact
```
Event On Interact
  ├─ Branch: Is Occupied?
  │  ├─ True:
  │  │   └─ Return false
  │  └─ False:
  │      ├─ Get Player Character (from Interactor)
  │      ├─ Call: Sit Player (Player Character)
  │      └─ Return true
```

##### Function: Sit Player
```
Sit Player (Input: Player Character)
  ├─ Set Seated Player = Player Character
  ├─ Set Is Occupied = true
  │
  ├─ Disable Player Input
  ├─ Attach Player to Sit Position
  │  └─ Attach Actor to Component (Keep World Position: false)
  │
  ├─ Get Player Camera Manager
  ├─ Set View Target with Blend
  │  ├─ Target: Self
  │  ├─ Blend Time: 0.5s
  │  └─ Blend Function: Cubic
  │
  ├─ Delay (0.5 seconds)
  │
  ├─ Enable Control Mode
  │  ├─ Add Input Mapping (IMC_TrainControl)
  │  └─ Bind Input Actions
  │
  └─ Update Interaction Text = "Exit (E)"
```

##### Function: Exit Player
```
Exit Player
  ├─ Branch: Has Seated Player?
  │  └─ False: Return
  │
  ├─ Disable Control Mode
  │  ├─ Remove Input Mapping (IMC_TrainControl)
  │  └─ Unbind Input Actions
  │
  ├─ Get Player Camera Manager
  ├─ Set View Target with Blend
  │  ├─ Target: Seated Player
  │  ├─ Blend Time: 0.5s
  │  └─ Blend Function: Cubic
  │
  ├─ Delay (0.5 seconds)
  │
  ├─ Detach Player
  │  └─ Detach from Actor (Keep World Position: true)
  │
  ├─ Move Player Forward (100 units)
  │  └─ Set Actor Location (Offset by Forward Vector)
  │
  ├─ Enable Player Input
  │
  ├─ Set Seated Player = None
  ├─ Set Is Occupied = false
  └─ Update Interaction Text = "Sit"
```

##### Input: Exit Seat (IA_ExitSeat)
```
Enhanced Input Action: IA_ExitSeat (Started)
  ├─ Call: Exit Player
  └─ Stop Train (set target speed to 0)
```

### Step 3: Create Train Controller Blueprint

**Blueprint**: `Content/Train/BP_TrainController`  
**Parent Class**: `Actor`

#### Components

- **TrainMesh** (Static Mesh) - Visual representation
- **DriverSeat** (Child Actor Component) - Instance of BP_TrainDriverSeat
- **Speedometer** (Text Render) - Debug speed display

#### Variables

```
Name                    Type        Default     Description
-----------------------------------------------------------------------------
CurrentSpeed            Float       0.0         Current speed (cm/s)
TargetSpeed             Float       0.0         Desired speed
MaxSpeed                Float       2000.0      Maximum forward speed
MaxReverseSpeed         Float       -500.0      Maximum reverse speed
Acceleration            Float       100.0       Speed increase rate
Deceleration            Float       200.0       Speed decrease rate
BrakeForce              Float       500.0       Emergency brake rate
bIsReversing            Boolean     false       Reverse mode enabled
DriverSeat              BP_TrainDriverSeat  None    Seat reference
```

#### Blueprint Events

##### Event BeginPlay
```
Event BeginPlay
  ├─ Get Child Actor (DriverSeat Component)
  ├─ Cast to BP_TrainDriverSeat
  └─ Set Driver Seat (reference)
```

##### Event Tick
```
Event Tick
  ├─ Update Speed
  │  ├─ Branch: Current Speed != Target Speed
  │  │  ├─ True:
  │  │  │   ├─ Calculate Interpolation Speed
  │  │  │   │  └─ Select: Accelerating? Use Acceleration : Use Deceleration
  │  │  │   ├─ FInterp To (Current → Target, Delta Time, Interp Speed)
  │  │  │   └─ Set Current Speed
  │  │  └─ False: Continue
  │
  ├─ Apply Movement
  │  ├─ Get Forward Vector
  │  ├─ Multiply: Forward Vector × Current Speed × Delta Time
  │  ├─ Add Actor World Offset (Delta Movement)
  │  └─ Set Sweep: true (for collision)
  │
  └─ Update Debug Display
     └─ Set Speedometer Text (format: "{0} km/h", Current Speed / 27.78)
```

##### Input: Train Throttle (IA_TrainThrottle)
```
Enhanced Input Action: IA_TrainThrottle (Triggered)
  ├─ Get Action Value (float)
  │
  ├─ Branch: Value > 0.1
  │  ├─ True: Accelerate Forward
  │  │   ├─ Branch: Is Reversing?
  │  │   │  ├─ True: Set Is Reversing = false
  │  │   │  └─ False: Continue
  │  │   └─ Set Target Speed = Max Speed × Value
  │  │
  │  ├─ Else Branch: Value < -0.1
  │  │   ├─ True: Accelerate Backward
  │  │   │   ├─ Set Is Reversing = true
  │  │   │   └─ Set Target Speed = Max Reverse Speed × Abs(Value)
  │  │   └─ False: Coast (no change)
  │  │
  │  └─ Else: Neutral
  │      └─ Set Target Speed = 0
```

##### Input: Train Brake (IA_TrainBrake)
```
Enhanced Input Action: IA_TrainBrake (Triggered)
  ├─ Set Target Speed = 0
  ├─ Apply Emergency Brake
  │  ├─ Calculate: Current Speed - (Brake Force × Delta Time)
  │  └─ Set Current Speed (clamped to 0)
  └─ Play Brake Sound
```

##### Input: Toggle Reverse (IA_TrainReverse)
```
Enhanced Input Action: IA_TrainReverse (Started)
  ├─ Branch: Current Speed < 50
  │  ├─ True: Can toggle
  │  │   ├─ Toggle: Is Reversing
  │  │   ├─ Set Target Speed = 0
  │  │   └─ Play Toggle Sound
  │  └─ False: Cannot toggle while moving
  │      └─ Display Warning ("Slow down first")
```

##### Function: Set Control Inputs Enabled
```
Set Control Inputs Enabled (Input: Enabled)
  ├─ Branch: Enabled?
  │  ├─ True: Enable
  │  │   ├─ Get Player Controller
  │  │   └─ Enable Input (Player Controller)
  │  └─ False: Disable
  │      ├─ Get Player Controller
  │      └─ Disable Input (Player Controller)
```

### Step 4: Create Driver HUD Widget

**Widget Blueprint**: `Content/Train/UI/WBP_TrainHUD`

#### Widget Structure

```
Canvas Panel
├─ Vertical Box (Driver Info)
│  ├─ Text Block: "Speed"
│  ├─ Progress Bar: Speed Indicator
│  ├─ Text Block: Speed Value (km/h)
│  └─ Text Block: Mode ("FORWARD" / "REVERSE")
│
└─ Vertical Box (Controls Help)
   ├─ Text Block: "W/S - Throttle"
   ├─ Text Block: "Space - Brake"
   ├─ Text Block: "R - Toggle Reverse"
   └─ Text Block: "E - Exit Seat"
```

#### Widget Variables

```
Name                Type                    Description
------------------------------------------------------------------------
OwningTrain         BP_TrainController      Reference to train
SpeedText           Text Block              Speed display
SpeedBar            Progress Bar            Visual speed indicator
ModeText            Text Block              Forward/Reverse indicator
```

#### Widget Events

##### Event Construct
```
Event Construct
  ├─ Bind to Train Controller
  │  └─ Set Owning Train (from player or level)
  └─ Start Update Timer
     └─ Set Timer by Function Name ("UpdateHUD", 0.1s, looping: true)
```

##### Function: Update HUD
```
Update HUD
  ├─ Get Current Speed (from Owning Train)
  │
  ├─ Convert to km/h
  │  └─ Divide: Speed ÷ 27.78
  │
  ├─ Update Speed Text
  │  └─ Set Text: Format("{0} km/h", Speed)
  │
  ├─ Update Speed Bar
  │  ├─ Calculate Percent: Abs(Speed) ÷ Max Speed
  │  └─ Set Percent (0.0 to 1.0)
  │
  └─ Update Mode Text
     ├─ Branch: Is Reversing?
     │  ├─ True: Set Text = "REVERSE" (Red)
     │  └─ False: Set Text = "FORWARD" (Green)
```

### Step 5: Create Input Mapping Context

**Asset**: `Content/Input/IMC_TrainControl`  
**Type**: Input Mapping Context

Add these mappings:
1. **IA_ExitSeat** → Key: E
2. **IA_TrainThrottle** → Keys: W (+1.0), S (-1.0)
3. **IA_TrainBrake** → Key: Space
4. **IA_TrainReverse** → Key: R

### Step 6: Setup Train in Level

1. Place `BP_TrainController` in your level
2. Position the train on rails/tracks
3. The Driver Seat will be automatically created as a child
4. Adjust seat position in the Train blueprint if needed

#### Positioning Seat and Camera

In `BP_TrainController` viewport:
1. Select **DriverSeat** child actor component
2. Position it where the driver should sit
3. Inside the seat, position **SitPosition** (player attach point)
4. Position **CameraPosition** for optimal view

## Advanced Features

### Feature 1: Collision Detection

In `BP_TrainController` Event Tick:
```
Add Actor World Offset (with Sweep: true)
  └─ Hit Result
     ├─ Branch: Blocking Hit?
     │  ├─ True: Collision detected
     │  │   ├─ Set Current Speed = 0
     │  │   ├─ Set Target Speed = 0
     │  │   └─ Play Crash Sound
     │  └─ False: Continue
```

### Feature 2: Speed Limits

Add speed limit zones:
```
On Overlap Begin (Trigger Volume)
  ├─ Get Other Actor
  ├─ Cast to BP_TrainController
  ├─ Set Max Speed = Zone Speed Limit
  └─ Display Speed Limit Sign
```

### Feature 3: Station Stops

Create `BP_TrainStation` actor:
```
On Train Overlap
  ├─ Check: Is Player Seated?
  │  └─ True:
  │     ├─ Display: "Press E to stop at station"
  │     └─ If E pressed:
  │        ├─ Set Train Target Speed = 0
  │        ├─ Wait for stop
  │        └─ Open doors
```

### Feature 4: Momentum Physics

For more realistic physics:
```
Event Tick (in BP_TrainController)
  ├─ Calculate Momentum
  │  ├─ Apply acceleration based on mass
  │  ├─ Apply friction/resistance
  │  └─ Apply slope gravity
  │
  └─ Update Current Speed (with momentum)
```

### Feature 5: Seat Animations

In `BP_TrainDriverSeat`:
```
Sit Player
  └─ After attach:
     ├─ Play Sitting Animation
     └─ Set Player Mesh Relative Location/Rotation
```

## Debugging

### Enable Visual Debugging

In `BP_TrainController`:
1. Add debug sphere at driver position
2. Draw debug line showing movement direction
3. Print speed to screen

```
Event Tick
  └─ Draw Debug String
     ├─ Text: "Speed: {0} km/h"
     ├─ Location: Above train
     └─ Color: Speed-based (Green → Yellow → Red)
```

### Common Issues

**Issue**: Player falls through seat
- **Fix**: Ensure SitPosition is above seat mesh
- **Fix**: Set collision on seat to BlockAll

**Issue**: Camera clips through train
- **Fix**: Position CameraPosition away from walls
- **Fix**: Adjust camera collision settings

**Issue**: Can't exit seat
- **Fix**: Verify IA_ExitSeat input binding
- **Fix**: Check Input Mapping Context priority

**Issue**: Train doesn't move
- **Fix**: Check Current Speed in debugger
- **Fix**: Verify Tick is enabled
- **Fix**: Check for collision blocking movement

**Issue**: Controls don't respond
- **Fix**: Verify Input Mapping Context is added
- **Fix**: Check Player Controller has input enabled
- **Fix**: Verify Input Actions are bound correctly

## Performance Optimization

### Best Practices

1. **Update Frequency**
   - HUD: Update at 10 Hz (0.1s timer)
   - Physics: Use Event Tick
   - Input: Use Enhanced Input (event-driven)

2. **Collision**
   - Use simple collision on train mesh
   - Sweep only when moving
   - Cache collision results

3. **UI**
   - Update only when values change
   - Use binding sparingly
   - Pool widget instances

## Testing Checklist

- [ ] Can interact with seat when empty
- [ ] Cannot interact when occupied
- [ ] Camera transitions smoothly
- [ ] Controls respond correctly
- [ ] Speed limits are respected
- [ ] Brake stops the train
- [ ] Reverse toggle works only when slow
- [ ] Exit returns player safely
- [ ] HUD displays correct information
- [ ] Collision detection works
- [ ] Multiple trains don't interfere

## Next Steps

1. Add sounds (engine, brake, ambient)
2. Create particle effects (steam, smoke)
3. Implement multiplayer (train conductor role)
4. Add train damage system
5. Create railway signaling system
6. Design conductor's cabin interior
7. Add passenger cars with seats

## Additional Resources

- **Main Interaction System**: `README_INTERACTION.md`
- **Enhanced Input**: Unreal Engine documentation
- **Blueprint Best Practices**: `Documentation/BlueprintGuidelines.md`

---

**Happy Train Driving! 🚂**
