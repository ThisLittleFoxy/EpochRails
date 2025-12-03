# Train Control System - Quick Start Guide

## What is it?

A Blueprint-based system that allows players to sit in a train driver's seat and control the **existing RailsTrain** class. Built on top of the [Interaction System](README_INTERACTION.md) and integrates seamlessly with your current train implementation.

## Features

✅ **Sit in driver's seat** - Interact with seat to sit down  
✅ **Control existing RailsTrain** - Use your existing train logic  
✅ **Smooth camera transition** - Professional camera blend when sitting  
✅ **Driver HUD** - Speed indicator and controls display  
✅ **Easy exit** - Press E to leave the seat  
✅ **Blueprint only** - No C++ modifications required  

## How It Works

The system adds a **BP_TrainDriverSeat** Blueprint that:
- Extends `InteractableActor` (your existing interaction system)
- Provides player attachment and camera management
- Calls public API of your existing `ARailsTrain` class:
  - `StartTrain()` / `StopTrain()`
  - `SetSpeed(float)`
  - `GetCurrentSpeed()`

**No modifications to RailsTrain.h/cpp needed!** Everything works through Blueprint.

## Quick Setup (4 Steps)

### Step 1: Create Input Actions

Create these Input Actions in `Content/Input/`:

1. **IA_ExitSeat** (Digital bool) → Key: E
2. **IA_TrainThrottle** (Axis1D float) → Keys: W (+1.0), S (-1.0)
3. **IA_TrainBrake** (Digital bool) → Key: Space

Add to your `IMC_Default` Input Mapping Context.

### Step 2: Create Driver Seat Blueprint

**Blueprint**: `Content/Train/BP_TrainDriverSeat`  
**Parent**: `InteractableActor`

**Add Components:**
- Static Mesh (seat visual)
- Scene Component "SitPosition" (player attach point)
- Scene Component "CameraPosition" (camera target)

**Variables:**
- `Owning Train` (RailsTrain Object Reference)
- `Seated Player` (EpochRailsCharacter Object Reference)
- `bIsOccupied` (Boolean) = false
- `Target Speed` (Float) = 0.0
- `Max Control Speed` (Float) = 2000.0

**Properties (Details Panel):**
- Interaction Name = "Driver Seat"
- Interaction Action = "Sit"
- Max Interaction Distance = 150.0

### Step 3: Implement Seat Functions

#### Event Graph - Core Events

**Event BeginPlay:**
```
Event BeginPlay
  ├─ Get Owner
  ├─ Cast to RailsTrain
  ├─ Branch: Cast Success?
  │  ├─ True: Set Owning Train (save reference)
  │  └─ False: Print Warning ("Seat must be child of RailsTrain")
```

**Can Interact (Override):**
```
Can Interact
  ├─ Branch: Is Occupied?
  │  ├─ True: Return false
  │  └─ False: Return true
```

**Get Interaction Action (Override):**
```
Get Interaction Action
  ├─ Branch: Is Occupied?
  │  ├─ True: Return "Occupied"
  │  └─ False: Return "Sit"
```

**Event On Interact:**
```
Event On Interact
  ├─ Branch: Is Occupied?
  │  ├─ True: Return false
  │  └─ False:
  │     ├─ Get Interacting Player Character
  │     ├─ Call: Sit Player (Player Character)
  │     └─ Return true
```

#### Function: Sit Player

**Input**: Player (EpochRailsCharacter)

```
Sit Player
  ├─ Set Seated Player = Player
  ├─ Set Is Occupied = true
  │
  ├─ Disable Player Input
  │  └─ Call: Disable Input (Get Player Controller)
  │
  ├─ Attach Player to Seat
  │  ├─ Attach Actor to Component
  │  │  ├─ Actor: Seated Player
  │  │  ├─ Parent: Sit Position
  │  │  ├─ Location Rule: Snap to Target
  │  │  ├─ Rotation Rule: Snap to Target
  │  │  └─ Scale Rule: Keep World
  │
  ├─ Switch Camera
  │  ├─ Get Player Camera Manager
  │  ├─ Set View Target with Blend
  │  │  ├─ New View Target: Self
  │  │  ├─ Blend Time: 0.5
  │  │  └─ Blend Func: EaseInOut (Cubic)
  │
  ├─ Delay: 0.5 seconds
  │
  ├─ Enable Train Controls
  │  ├─ Get Player Controller
  │  ├─ Get Enhanced Input Subsystem
  │  ├─ Add Mapping Context (IMC_TrainControl, Priority: 1)
  │  └─ Enable Input (Get Player Controller)
  │
  ├─ Start Train
  │  └─ Call: Owning Train → Start Train
  │
  └─ Update Interaction Text = "Exit (E)"
```

#### Function: Exit Player

```
Exit Player
  ├─ Branch: Seated Player is Valid?
  │  └─ False: Return (nothing to do)
  │
  ├─ Stop Train
  │  ├─ Set Target Speed = 0
  │  └─ Call: Owning Train → Set Speed (0)
  │
  ├─ Disable Train Controls
  │  ├─ Get Player Controller
  │  ├─ Get Enhanced Input Subsystem
  │  ├─ Remove Mapping Context (IMC_TrainControl)
  │  └─ Disable Input (Get Player Controller)
  │
  ├─ Restore Camera
  │  ├─ Get Player Camera Manager
  │  ├─ Set View Target with Blend
  │  │  ├─ New View Target: Seated Player
  │  │  ├─ Blend Time: 0.5
  │  │  └─ Blend Func: EaseInOut (Cubic)
  │
  ├─ Delay: 0.5 seconds
  │
  ├─ Detach Player
  │  ├─ Detach from Actor
  │  │  ├─ Location Rule: Keep World
  │  │  ├─ Rotation Rule: Keep World
  │  │  └─ Scale Rule: Keep World
  │  │
  │  ├─ Get Actor Forward Vector
  │  ├─ Multiply: Forward × 100.0 (move player forward)
  │  ├─ Get Actor Location (Seated Player)
  │  ├─ Add: Location + Offset
  │  └─ Set Actor Location (Seated Player, New Location)
  │
  ├─ Enable Player Input
  │  └─ Call: Enable Input (Get Player Controller)
  │
  ├─ Clear References
  │  ├─ Set Seated Player = None
  │  ├─ Set Is Occupied = false
  │  └─ Set Target Speed = 0.0
  │
  └─ Update Interaction Text = "Sit"
```

#### Input Event: Exit Seat

**Enhanced Input Action**: IA_ExitSeat (Started)

```
IA_ExitSeat (Started)
  └─ Call: Exit Player
```

#### Input Event: Train Throttle

**Enhanced Input Action**: IA_TrainThrottle (Triggered)

```
IA_TrainThrottle (Triggered)
  ├─ Get Action Value (Float)
  ├─ Store in local variable: Throttle Input
  │
  ├─ Branch: Throttle Input > 0.1?
  │  ├─ True: Accelerate Forward
  │  │  ├─ Set Target Speed = Max Control Speed × Throttle Input
  │  │  └─ Call: Owning Train → Set Speed (Target Speed)
  │  │
  │  └─ False: Branch: Throttle Input < -0.1?
  │     ├─ True: Slow Down
  │     │  ├─ Set Target Speed = Max Control Speed × Throttle Input × 0.3
  │     │  └─ Call: Owning Train → Set Speed (Abs(Target Speed))
  │     │
  │     └─ False: Coast (no throttle)
  │        ├─ Get Current Speed (from Owning Train)
  │        ├─ Multiply: Current Speed × 0.95 (gradual slow down)
  │        └─ Call: Owning Train → Set Speed (New Speed)
```

#### Input Event: Train Brake

**Enhanced Input Action**: IA_TrainBrake (Triggered)

```
IA_TrainBrake (Triggered)
  ├─ Set Target Speed = 0
  ├─ Call: Owning Train → Stop Train
  └─ [Optional] Play Brake Sound
```

### Step 4: Add Seat to Train

**Option A: Blueprint Child Actor (Recommended)**

1. Open your existing train Blueprint (child of RailsTrain)
2. Add Component → Child Actor Component
3. Name it "DriverSeat"
4. Set Child Actor Class = BP_TrainDriverSeat
5. Position it where driver should sit
6. Done!

**Option B: Manual Placement**

1. Place BP_TrainDriverSeat in level
2. Attach to train actor in World Outliner (drag and drop)
3. Position seat in viewport

### Step 5: Test

1. Play in Editor (PIE)
2. Walk to driver seat
3. Look at seat (should highlight if interaction system is set up)
4. Press E to sit
5. Use W/S to control speed
6. Press Space to brake
7. Press E again to exit

## Controls

| Key | Action |
|-----|--------|
| **E** | Sit in seat / Exit seat |
| **W** | Accelerate (increase speed) |
| **S** | Slow down (decrease speed) |
| **Space** | Emergency brake (stop train) |

## Blueprint Implementation Details

### Speed Control Logic

The system controls your existing `RailsTrain` by calling:

```cpp
// Your existing C++ API (no changes needed)
OwningTrain->SetSpeed(NewSpeed);  // From Blueprint
OwningTrain->StartTrain();        // From Blueprint
OwningTrain->StopTrain();         // From Blueprint
float Speed = OwningTrain->GetCurrentSpeed(); // From Blueprint
```

### Camera System

When sitting:
1. Player camera smoothly blends to seat's camera position (0.5s)
2. Seat actor becomes view target
3. Camera follows train movement automatically

When exiting:
1. Camera blends back to player (0.5s)
2. Player detaches from train
3. Normal camera control restored

## Optional: Create Driver HUD

**Widget**: `Content/Train/UI/WBP_TrainHUD`

**Widget Structure:**
```
Canvas Panel
├─ Text: Speed Label ("Speed")
├─ Progress Bar: Speed Indicator
├─ Text: Speed Value ("{Speed} km/h")
└─ Vertical Box: Controls Help
   ├─ Text: "W - Accelerate"
   ├─ Text: "S - Slow Down"
   ├─ Text: "Space - Brake"
   └─ Text: "E - Exit"
```

**Widget Variables:**
- `Owning Train` (RailsTrain Reference)

**Function: Update HUD (called every 0.1s)**
```
Update HUD
  ├─ Get Current Speed (from Owning Train)
  ├─ Convert to km/h: Speed ÷ 27.78
  ├─ Set Speed Text: Format("{0} km/h", Speed)
  ├─ Set Progress Bar: Percent = Speed ÷ Max Speed
  └─ [Optional] Change color based on speed
```

**Show HUD:**
- In `Sit Player`: Create Widget → Add to Viewport
- In `Exit Player`: Remove from Parent

## Advanced Features

### Feature 1: Speed Limits

**Using Trigger Volumes:**

1. Place Trigger Box in level
2. Set overlap events in Level Blueprint:

```
On Actor Begin Overlap (RailsTrain)
  ├─ Cast to RailsTrain
  └─ Set Max Speed = Speed Limit (e.g., 500.0)

On Actor End Overlap (RailsTrain)
  ├─ Cast to RailsTrain
  └─ Set Max Speed = Original Max Speed (e.g., 2000.0)
```

### Feature 2: Station Stops

**Blueprint**: `BP_TrainStation`

```
On Overlap Begin (Trigger)
  ├─ Cast to RailsTrain
  ├─ Check: Is occupied (has seated player)?
  │  └─ Display Widget: "Press F to stop at station"
  │
  └─ On F Key Pressed:
     ├─ Call: Train → Stop Train
     └─ [Optional] Open doors after stop
```

### Feature 3: Emergency Brake Distance

In `BP_TrainDriverSeat`:

```
IA_TrainBrake (Triggered)
  ├─ Get Current Speed (from Owning Train)
  ├─ Calculate Brake Distance = Speed² / (2 × Deceleration)
  ├─ Display Warning if distance is large
  └─ Call: Owning Train → Stop Train
```

### Feature 4: Speed Zones with Spline

Modify your `RailsSplinePath` to include speed data:

1. Add metadata to spline points
2. In `BP_TrainDriverSeat`, query spline for speed limit
3. Clamp `Max Control Speed` based on current position

## Debugging Tips

**Enable Debug Display:**

In `BP_TrainDriverSeat` Event Tick:
```
Draw Debug String
  ├─ Text: Format("Speed: {0} km/h | Target: {1}", Current Speed, Target Speed)
  ├─ Location: Get Actor Location + (0, 0, 200)
  ├─ Color: Green
  └─ Duration: 0.0 (single frame)
```

**Common Issues:**

1. **Seat not interactable**
   - ✓ Check BP_TrainDriverSeat parent is InteractableActor
   - ✓ Verify seat has collision on Visibility channel
   - ✓ Check player has IA_Interact set up

2. **Train doesn't respond to controls**
   - ✓ Verify Owning Train reference is set (BeginPlay)
   - ✓ Check RailsTrain has SplinePathRef assigned
   - ✓ Use Print String to debug SetSpeed calls

3. **Camera doesn't move**
   - ✓ Check CameraPosition component exists
   - ✓ Verify Set View Target is called
   - ✓ Ensure blend time > 0

4. **Player falls through train**
   - ✓ Check PlatformMesh collision is enabled
   - ✓ Verify player is attached to SitPosition
   - ✓ SitPosition should be above platform surface

5. **Can't exit seat**
   - ✓ Verify IA_ExitSeat is bound correctly
   - ✓ Check IMC_TrainControl is added with correct priority
   - ✓ Test with Print String in Exit Player function

## Performance Notes

✅ **Uses existing RailsTrain** - No duplicate train logic  
✅ **Lightweight Blueprint** - Only handles seat interaction  
✅ **Efficient** - Reuses C++ movement system  
✅ **Scalable** - Works with multiple trains  

## Integration with Existing Systems

### RailsTrain API Used

Your existing `ARailsTrain` provides these Blueprint-callable functions:

```cpp
// Called from BP_TrainDriverSeat
StartTrain()              // Begin movement
StopTrain()               // Begin deceleration
SetSpeed(float)           // Set target speed
GetCurrentSpeed()         // Read current speed
GetTrainState()           // Check state (Moving, Stopped, etc.)
```

**No modifications to RailsTrain.h/cpp required!**

### Character Boarding System

Your existing `ARailsTrain` handles passenger boarding:
- Characters auto-board when entering `BoardingZone`
- Platform movement is inherited automatically
- Seated player is still a "passenger" on the train

### Spline Movement

Your existing `RailsSplinePath` system:
- Continues to work as-is
- Train follows spline automatically
- Seat inherits all train movement (rotation, position)

## Testing Checklist

- [ ] Seat is child of RailsTrain (or attached to train)
- [ ] Can interact with empty seat (E key)
- [ ] Cannot interact when occupied
- [ ] Camera transitions smoothly (0.5s blend)
- [ ] W key increases train speed
- [ ] S key decreases train speed
- [ ] Space key stops train (calls StopTrain)
- [ ] E key exits seat properly
- [ ] Player is positioned safely when exiting (100 units forward)
- [ ] Train continues on spline path
- [ ] Multiple trains work independently
- [ ] Existing boarding system still works

## File Structure

```
Content/
├─ Input/
│  ├─ IA_ExitSeat
│  ├─ IA_TrainThrottle
│  ├─ IA_TrainBrake
│  └─ IMC_TrainControl
│
└─ Train/
   ├─ BP_TrainDriverSeat (NEW)
   ├─ BP_MyTrain (your existing train Blueprint, child of RailsTrain)
   └─ UI/
      └─ WBP_TrainHUD (optional)

Source/EpochRails/Train/
├─ RailsTrain.h (NO CHANGES)
├─ RailsTrain.cpp (NO CHANGES)
├─ RailsSplinePath.h (NO CHANGES)
└─ RailsSplinePath.cpp (NO CHANGES)
```

## Next Steps

1. Create BP_TrainDriverSeat Blueprint
2. Add seat as Child Actor to your train
3. Create Input Actions (3 actions)
4. Implement seat functions (Sit, Exit, Throttle, Brake)
5. Test in PIE
6. [Optional] Add HUD widget
7. [Optional] Add sound effects
8. [Optional] Create station stops

## Additional Resources

- **Full Documentation**: `Documentation/TrainControlSystem.md`
- **Interaction System**: `README_INTERACTION.md`  
- **RailsTrain Source**: `Source/EpochRails/Train/RailsTrain.h`
- **Enhanced Input Guide**: Unreal Engine Documentation

---

**Happy Train Driving! 🚂**

*Integrates with existing RailsTrain • Uses Interaction System • Pure Blueprint • No C++ Changes*
