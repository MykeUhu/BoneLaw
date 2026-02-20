# Settler Screen Widget Creation Fix

## Problem
Widgets werden im Grid erstellt, aber sie haben keine Daten gebunden. Die Loop erstellt leere Widgets ohne ViewModel-Initialisierung.

## Root Cause
Nach `Create WBP Settler Slot Widget` fehlt die ViewModel-Initialisierung und GUID-Übergabe.

---

## Blueprint Fix: WBP_SettlerScreen

### Aktueller Flow (FALSCH):
```
For Loop (0 to NumSettlers)
  ↓
Create WBP Settler Slot Widget
  ↓
Is Valid?
  ↓
Add Child to Uniform Grid Panel (Row=Index/4, Col=Index%4)
  ↓
Append to Slot Widgets Array
```

### Korrigierter Flow (RICHTIG):
```
For Loop (0 to NumSettlers)
  ↓
Get Settler Screen VM (self)
  ↓
Get Settler Guid By Index (LoopIndex) → SettlerGuid
  ↓
Create WBP Settler Slot Widget
  ↓
Is Valid?
  ┣━━ TRUE:
  ┃    ↓
  ┃   SET: Store SettlerGuid im Widget (als Variable)
  ┃    ↓
  ┃   Call: SetupVM(SettlerGuid, ScreenViewModel)
  ┃    ↓
  ┃   Add Child to Uniform Grid Panel (Row, Col)
  ┃    ↓
  ┃   Append to Slot Widgets Array
  ┃
  ┗━━ FALSE: Print String "Widget creation failed"
```

---

## Blueprint Fix: WBP_SettlerSlot

### 1. Add Variable
**Name**: `CachedSettlerGuid`
**Type**: `Guid` (struct)
**Default**: Invalid Guid
**Category**: "Settlers"

---

### 2. Update SetupVM Custom Event

**Inputs**:
- `SettlerGuid` (Guid)
- `ScreenViewModel` (UMVVM_SettlerScreen, Object Reference)

**Flow**:
```
SetupVM (SettlerGuid, ScreenViewModel)
  ↓
SET: CachedSettlerGuid = SettlerGuid
  ↓
Construct BP Settler Slot View Model
  ↓
Add View Model Instance (Context: "BP Settler Slot", VM Instance)
  ↓
Get View Model (BP_SettlerSlot_ViewModel)
  ↓
Branch: Is Valid?
  ┣━━ TRUE:
  ┃    ↓
  ┃   Call: InitializeSlot(SettlerGuid, ScreenViewModel)
  ┃    ↓
  ┃   ScreenViewModel → RegisterSlotViewModel(SettlerGuid, SlotVM)
  ┃    ↓
  ┃   Print String "[Slot] Setup complete for {SettlerGuid}"
  ┃
  ┗━━ FALSE: Print String "[Slot] ViewModel creation failed"
```

---

## Blueprint Implementation Steps

### WBP_SettlerScreen - RebuildGrid Event

**After "Create WBP Settler Slot Widget" node:**

1. **Get SettlerGuid by Index**:
   - Drag from `Get Settler Screen VM` (self)
   - Call: `Get Settler Guid By Index`
   - Connect: `Loop Index` → `Index` pin
   - Store result as `Current Settler Guid` (local variable)

2. **Call SetupVM**:
   - Drag from `Create WBP Settler Slot Widget` return value
   - Call: `SetupVM` (custom event)
   - Connect: `Current Settler Guid` → `SettlerGuid` pin
   - Connect: `Get Settler Screen VM` → `ScreenViewModel` pin

3. **Add to Grid** (existing logic after SetupVM)

---

### WBP_SettlerSlot - SetupVM Event

**Signature**:
```
Inputs:
  - SettlerGuid (Guid)
  - ScreenViewModel (Object Reference - UMVVM_SettlerScreen)
```

**Implementation**:

1. **Store GUID**:
   ```
   SET CachedSettlerGuid = SettlerGuid
   ```

2. **Create ViewModel**:
   ```
   Construct BP Settler Slot View Model
   ```

3. **Add to View Model Collection**:
   ```
   Get Player Controller → Get View Model Collection
   Add View Model Instance:
     - Context Name: "BP Settler Slot"
     - View Model: [from Construct]
   ```

4. **Get ViewModel Reference**:
   ```
   Get View Model (BP_SettlerSlot_ViewModel_1)
   ```

5. **Initialize Slot Data**:
   ```
   Call: InitializeSlot
     - SettlerGuid
     - ScreenViewModel
   ```

6. **Register with Screen**:
   ```
   ScreenViewModel → RegisterSlotViewModel(SettlerGuid, SlotViewModel)
   ```

7. **Debug Output**:
   ```
   Print String: "[Slot] Setup for {SettlerGuid}"
   ```

---

## C++ Side (Already Implemented)

### UMVVM_SettlerSlot

Add initialization method:

```cpp
UFUNCTION(BlueprintCallable, Category="Settlers")
void InitializeSlot(const FGuid& SettlerGuid, UMVVM_SettlerScreen* ScreenVM);
```

Implementation:
```cpp
void UMVVM_SettlerSlot::InitializeSlot(const FGuid& SettlerGuid, UMVVM_SettlerScreen* ScreenVM)
{
    if (!ScreenVM)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SettlerSlot] InitializeSlot failed: ScreenVM is null"));
        return;
    }

    // Get settler data from ScreenVM
    FString SettlerName = ScreenVM->GetSettlerNameByGuid(SettlerGuid);
    AStoneBaseChar* SettlerPawn = ScreenVM->GetSettlerPawnByGuid(SettlerGuid);

    // Set occupied state
    SetOccupied(
        SettlerGuid.ToString(EGuidFormats::DigitsWithHyphens),
        SettlerName,
        SettlerPawn
    );

    UE_LOG(LogTemp, Log, TEXT("[SettlerSlot] Initialized: %s (%s)"), 
        *SettlerName, *SettlerGuid.ToString());
}
```

---

## Testing Checklist

1. **Compile C++** (if InitializeSlot was added)
2. **Open WBP_SettlerScreen** → Fix RebuildGrid loop
3. **Open WBP_SettlerSlot** → Fix SetupVM event
4. **Play in Editor**
5. **Check Output Log** for:
   ```
   [SettlerScreen] Refreshed data from roster. Settlers=X
   [Slot] Setup for {GUID-1}
   [Slot] Setup for {GUID-2}
   ...
   ```
6. **Verify Grid** shows widgets with settler names

---

## Common Issues

### Widgets created but empty
- **Cause**: `SetupVM` not called or GUID not passed
- **Fix**: Ensure `SetupVM` is called in loop with valid GUID

### No widgets appear
- **Cause**: Loop not executing (NumSettlers = 0)
- **Fix**: Check `BindToRoster` was called in Widget's `OnConstruct`

### Bindings show "(null)"
- **Cause**: ViewModel not registered in ViewModelCollection
- **Fix**: Verify `Add View Model Instance` is called in `SetupVM`

---

## Debug Output to Add

**WBP_SettlerScreen - RebuildGrid:**
```
Print String: "[Screen] Rebuilding grid for {NumSettlers} settlers"
```

**WBP_SettlerScreen - Loop:**
```
Print String: "[Screen] Creating widget for index {LoopIndex}, GUID: {SettlerGuid}"
```

**WBP_SettlerSlot - SetupVM:**
```
Print String: "[Slot] SetupVM called with GUID: {SettlerGuid}"
Print String: "[Slot] ViewModel registered successfully"
```
