# Train Control System - Quick Start Guide

## What is it?

A Blueprint-based system that allows players to sit in a train driver's seat and control the train. Built on top of the existing [Interaction System](README_INTERACTION.md).

## Features

✅ **Sit in driver's seat** - Interact with seat to sit down  
✅ **Control the train** - Accelerate, brake, and reverse  
✅ **Smooth camera transition** - Professional camera blend when sitting  
✅ **Driver HUD** - Speed indicator and controls display  
✅ **Easy exit** - Press E to leave the seat  
✅ **Blueprint only** - No C++ code required  

## Quick Setup (4 Steps)

### Step 1: Create Input Actions

Create these Input Actions in `Content/Input/`:

1. **IA_ExitSeat** (Digital bool) → Key: E
2. **IA_TrainThrottle** (Axis1D float) → Keys: W (+1.0), S (-1.0)
3. **IA_TrainBrake** (Digital bool) → Key: Space
4. **IA_TrainReverse** (Digital bool) → Key: R

Add to your `IMC_Default` Input Mapping Context.

### Step 2: Create Driver Seat Blueprint

**Blueprint**: `Content/Train/BP_TrainDriverSeat`  
**Parent**: `InteractableActor`

**Add Components:**
- Static Mesh (seat visual)
- Scene Component "SitPosition" (player attach point)
- Scene Component "CameraPosition" (camera target)

**Variables:**
- `Owning Train` (BP_TrainController reference)
- `Seated Player` (Character reference)
- `bIsOccupied` (Boolean)

**Key Functions:**
- `Sit Player` - Attach player, switch camera, enable controls
- `Exit Player` - Detach player, restore camera, disable controls

**Events:**
- `On Interact` - Call Sit Player
- `IA_ExitSeat` input - Call Exit Player

### Step 3: Create Train Controller Blueprint

**Blueprint**: `Content/Train/BP_TrainController`  
**Parent**: `Actor`

**Add Components:**
- Static Mesh (train visual)
- Child Actor Component (BP_TrainDriverSeat instance)

**Variables:**
- `Current Speed` (Float) = 0.0
- `Target Speed` (Float) = 0.0
- `Max Speed` (Float) = 2000.0
- `Acceleration` (Float) = 100.0
- `bIsReversing` (Boolean) = false

**Event Tick:**
```
1. Interpolate Current Speed to Target Speed
2. Move train: Add Actor World Offset (Forward Vector × Speed × Delta Time)
3. Update debug display
```

**Input Events:**
- `IA_TrainThrottle` - Set target speed based on input
- `IA_TrainBrake` - Set target speed to 0, apply brake force
- `IA_TrainReverse` - Toggle reverse mode

### Step 4: Place Train in Level

1. Drag `BP_TrainController` into your level
2. Position it on tracks
3. Play in Editor
4. Walk to driver seat and press E to sit
5. Use W/S to accelerate, Space to brake
6. Press E again to exit

## Controls

| Key | Action |
|-----|--------|
| **E** | Sit in seat / Exit seat |
| **W** | Accelerate forward |
| **S** | Accelerate backward / Reverse |
| **Space** | Emergency brake |
| **R** | Toggle reverse mode (only when slow) |

## Blueprint Event Examples

### Example 1: Sit Player (in BP_TrainDriverSeat)

```
Sit Player (Player Character)
  ├─ Set Seated Player = Player
  ├─ Set Is Occupied = true
  ├─ Disable Player Input
  ├─ Attach Actor to Component (Player → Sit Position)
  ├─ Set View Target with Blend (Target: Self, Time: 0.5s)
  ├─ Delay (0.5s)
  └─ Enable train controls (add IMC_TrainControl)
```

### Example 2: Exit Player (in BP_TrainDriverSeat)

```
Exit Player
  ├─ Disable train controls (remove IMC_TrainControl)
  ├─ Set View Target with Blend (Target: Player, Time: 0.5s)
  ├─ Delay (0.5s)
  ├─ Detach Actor (Keep World Position: true)
  ├─ Move player forward 100 units
  ├─ Enable Player Input
  ├─ Set Seated Player = None
  └─ Set Is Occupied = false
```

### Example 3: Train Movement (in BP_TrainController)

```
Event Tick
  ├─ FInterp To (Current Speed → Target Speed)
  ├─ Get Forward Vector
  ├─ Multiply: Forward × Current Speed × Delta Time
  └─ Add Actor World Offset (Delta, Sweep: true)
```

### Example 4: Throttle Input (in BP_TrainController)

```
IA_TrainThrottle (Triggered)
  ├─ Get Action Value (float)
  ├─ Branch: Value > 0.1?
  │  ├─ True: Set Target Speed = Max Speed × Value
  │  └─ False: Branch: Value < -0.1?
  │     ├─ True: Set Target Speed = Max Reverse Speed × Abs(Value)
  │     └─ False: Set Target Speed = 0 (coast)
```

## Optional: Create Driver HUD

**Widget**: `Content/Train/UI/WBP_TrainHUD`

**Add to widget:**
- Text: Current speed in km/h
- Progress Bar: Visual speed indicator
- Text: Mode (FORWARD / REVERSE)
- Text: Controls help

**Update in Event Tick (0.1s timer):**
```
Update HUD
  ├─ Get Current Speed from train
  ├─ Convert to km/h (divide by 27.78)
  ├─ Set Speed Text
  ├─ Set Speed Bar Percent
  └─ Update Mode Text (FORWARD/REVERSE)
```

**Show HUD when sitting:**
- In `Sit Player` function: Create Widget and Add to Viewport
- In `Exit Player` function: Remove Widget from Parent

## Advanced Features

### Feature 1: Multiple Speeds

Add gear system:
```
Variables:
- Current Gear (Integer) = 1
- Gear Ratios (Array of Floats) = [0.3, 0.6, 1.0]

Calculate Speed:
Target Speed = Max Speed × Gear Ratio[Current Gear] × Throttle Input
```

### Feature 2: Collision Detection

Handle obstacles:
```
Add Actor World Offset (Sweep: true, Hit Result)
  ├─ Branch: Blocking Hit?
  │  ├─ True:
  │  │   ├─ Set Current Speed = 0
  │  │   ├─ Set Target Speed = 0
  │  │   └─ Play Crash Sound
```

### Feature 3: Train Stations

Create `BP_TrainStation` with trigger volume:
```
On Overlap Begin
  ├─ Cast to BP_TrainController
  ├─ Check: Has seated player?
  │  └─ Display: "Press F to stop at station"
  └─ On F pressed: Gradually stop train
```

### Feature 4: Realistic Physics

Add momentum:
```
Variables:
- Train Mass (Float) = 1000.0
- Friction (Float) = 50.0
- Slope Angle (Float) = 0.0

Calculate Force:
Force = (Throttle × Engine Power - Friction - Gravity × Sin(Slope)) / Mass
Speed += Force × Delta Time
```

### Feature 5: Seat Animations

Play sitting animation:
```
Sit Player
  └─ Play Animation Montage ("Sit_Anim")

Exit Player
  └─ Play Animation Montage ("Stand_Anim")
```

## Debugging Tips

**Enable Debug Display:**

In `BP_TrainController` Event Tick:
```
Draw Debug String
  ├─ Text: Format("Speed: {0} km/h", Current Speed / 27.78)
  ├─ Location: Get Actor Location + (0, 0, 200)
  ├─ Color: Lerp (Green to Red based on speed)
  └─ Duration: 0.0 (single frame)
```

**Common Issues:**

1. **Player can't sit**
   - Check seat has InteractableActor parent class
   - Verify IA_Interact is set up in player character
   - Check seat collision on Visibility channel

2. **Camera doesn't move**
   - Verify CameraPosition component exists
   - Check Set View Target is called
   - Ensure blend time is > 0

3. **Controls don't work**
   - Verify Input Mapping Context is added
   - Check Input Actions are bound correctly
   - Ensure player controller has input enabled

4. **Train doesn't move**
   - Check Event Tick is enabled
   - Verify Current Speed is changing (use Print String)
   - Check for collision blocking movement

5. **Can't exit seat**
   - Verify IA_ExitSeat input binding
   - Check Input Mapping Context priority
   - Ensure Exit Player function is called

## Performance Notes

✅ **Optimized**: Uses interpolation instead of instant speed changes  
✅ **Efficient**: HUD updates at 10 Hz, not every frame  
✅ **Scalable**: Sweep only when train is moving  
✅ **Physics-friendly**: Uses Add Actor World Offset for smooth movement  

## Blueprint Best Practices

1. **Use Functions** - Break complex logic into reusable functions
2. **Name Variables Clearly** - Use descriptive names with prefixes (b for bool)
3. **Add Comments** - Document complex Blueprint graphs (English only)
4. **Use Pure Functions** - For getters and calculations
5. **Handle Edge Cases** - Check for None references
6. **Enable Validation** - Use Branch nodes for safety

## Testing Checklist

- [ ] Can interact with empty seat
- [ ] Cannot interact with occupied seat
- [ ] Camera transitions smoothly (0.5s blend)
- [ ] W key accelerates forward
- [ ] S key accelerates backward
- [ ] Space key applies brakes
- [ ] R key toggles reverse mode
- [ ] E key exits seat properly
- [ ] Player is positioned correctly when exiting
- [ ] HUD displays accurate speed
- [ ] Train stops on collision
- [ ] Multiple trains work independently

## File Structure

```
Content/
├─ Input/
│  ├─ IA_ExitSeat
│  ├─ IA_TrainThrottle
│  ├─ IA_TrainBrake
│  ├─ IA_TrainReverse
│  └─ IMC_TrainControl
│
└─ Train/
   ├─ BP_TrainController
   ├─ BP_TrainDriverSeat
   └─ UI/
      └─ WBP_TrainHUD
```

## Next Steps

1. Read full documentation: [Documentation/TrainControlSystem.md](Documentation/TrainControlSystem.md)
2. Create your first train controller
3. Add driver's seat to your train
4. Test sitting and controlling
5. Add HUD for better feedback
6. Customize speed and acceleration values
7. Add sound effects and animations
8. Create train stations and stops

## Additional Resources

- **Full Documentation**: `Documentation/TrainControlSystem.md`
- **Interaction System**: `README_INTERACTION.md`
- **Enhanced Input Guide**: Unreal Engine Documentation
- **Blueprint Communication**: Unreal Engine Documentation

## Example Project Structure

**Minimal Setup (Required):**
1. `BP_TrainController` - Main train logic
2. `BP_TrainDriverSeat` - Seat interaction
3. Input Actions (4 actions)
4. Input Mapping Context

**Complete Setup (Recommended):**
1. All minimal setup files
2. `WBP_TrainHUD` - Driver interface
3. `BP_TrainStation` - Station stops
4. Sound effects
5. Animations
6. Particle effects

## Need Help?

1. Check full documentation in `Documentation/TrainControlSystem.md`
2. Review Interaction System guide: `README_INTERACTION.md`
3. Enable debug visualization
4. Check Output Log for errors
5. Verify all Input Actions are set up correctly
6. Test each component individually

---

**Happy Train Driving! 🚂**

*Built on the [Interaction System](README_INTERACTION.md) • Pure Blueprint • No C++ Required*
