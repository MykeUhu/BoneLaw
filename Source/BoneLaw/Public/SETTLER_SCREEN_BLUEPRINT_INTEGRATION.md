# Settler Screen Blueprint Integration Guide

## Overview

`UMVVM_SettlerScreen` ist jetzt ein **reiner Data Provider**. Blueprint erstellt und verwaltet alle ViewModels selbst. C++ bietet nur Roster-Daten und GUID-basierte Lookups.

---

## C++ API Summary

### Data Access

```cpp
// Get number of settlers
int32 GetNumSettlers()

// Get settler GUID by roster index
FGuid GetSettlerGuidByIndex(int32 Index)

// Get settler name by GUID
FString GetSettlerNameByGuid(FGuid SettlerId)

// Get or spawn settler pawn by GUID
AStoneBaseChar* GetSettlerPawnByGuid(FGuid SettlerId)
```

### ViewModel Registration

```cpp
// Register a slot ViewModel (for cross-widget GUID lookups)
void RegisterSlotViewModel(FGuid SettlerGuid, UMVVM_SettlerSlot* SlotVM)

// Unregister slot ViewModel
void UnregisterSlotViewModel(FGuid SettlerGuid)

// Get registered slot ViewModel by GUID
UMVVM_SettlerSlot* GetSlotViewModelBySettlerGuid(FGuid SettlerId)
```

### Events

```cpp
// Fired when roster changes → Blueprint should rebuild grid
UPROPERTY(BlueprintAssignable)
FSettlerListRebuilt OnSettlerListRebuilt

// Fired when details should be shown for a settler
UPROPERTY(BlueprintAssignable)
FOnSettlerRequestShowDetails OnRequestShowDetails

// Broadcast detail request (call from slot widgets)
void RequestShowDetails(FGuid SettlerGuid, AStoneBaseChar* SettlerActor)
```

---

## Blueprint Implementation: WBP_SettlerScreen

### 1. Initialization

```
Event Construct
├─ Create ViewModel (MVVM_SettlerScreen)
├─ Set ViewModel (on widget)
├─ Bind to Roster (on ViewModel)
└─ Bind OnSettlerListRebuilt → RebuildGrid
```

**Important**: ViewModel ist Blueprint Variable, nicht C++ EditDefaultsOnly!

---

### 2. RebuildGrid (Custom Event)

Fires when:
- OnSettlerListRebuilt broadcasts
- Initial construction

Steps:
```
RebuildGrid
├─ Clear Grid (remove all children from UniformGrid)
├─ Unregister all old SlotViewModels from ScreenViewModel
│
├─ FOR each index in 0..GetNumSettlers()-1:
│   ├─ Get SettlerGuid = ScreenViewModel->GetSettlerGuidByIndex(index)
│   ├─ Create Widget (WBP_SettlerSlot)
│   ├─ Create SlotViewModel (MVVM_SettlerSlot)
│   │
│   ├─ Initialize SlotViewModel:
│   │   ├─ Set SettlerGUID (as string, for binding)
│   │   ├─ Get SettlerName = ScreenViewModel->GetSettlerNameByGuid(Guid)
│   │   ├─ Set SettlerName
│   │   ├─ Get SettlerPawn = ScreenViewModel->GetSettlerPawnByGuid(Guid)
│   │   ├─ If Pawn valid: Bind to pawn GAS
│   │   └─ Set SlotStatus = Occupied
│   │
│   ├─ Register SlotViewModel: ScreenViewModel->RegisterSlotViewModel(Guid, SlotVM)
│   ├─ Set ViewModel on SlotWidget (for MVVM binding)
│   └─ Add SlotWidget to UniformGrid (Row = index / columns, Column = index % columns)
│
└─ Force Layout Update
```

---

### 3. Slot Widget: WBP_SettlerSlot

#### Variables:
```
FGuid CachedSettlerGuid (Blueprint variable)
```

#### Construct:
```
Event Construct
├─ Get ViewModel (MVVM_SettlerSlot)
├─ Parse GUID: FGuid::Parse(ViewModel->GetSettlerGUID(), CachedSettlerGuid)
└─ Bind ViewModel properties to UI (MVVM auto-binding)
```

#### Button Click:
```
OnButtonClicked
├─ Get ViewModel
├─ Get SettlerPawn = ViewModel->GetSettlerActor()
├─ Get ScreenViewModel (from parent or GetGameInstance->GetSubsystem)
└─ ScreenViewModel->RequestShowDetails(CachedSettlerGuid, SettlerPawn)
```

---

### 4. Details Panel: WBP_SettlerDetails

#### Receive OnRequestShowDetails:
```
Bind OnRequestShowDetails (from ScreenViewModel)
│
OnRequestShowDetails(FGuid SettlerGuid, AStoneBaseChar* SettlerActor)
├─ Create DetailsViewModel (MVVM_SettlerSlotDetails) if not exists
├─ DetailsViewModel->BindToSettler(SettlerActor)
├─ Set ViewModel on DetailsWidget
└─ Show Details Panel (Set Visibility)
```

---

## Example Blueprint Flow (Pseudo-Code)

### WBP_SettlerScreen

```blueprint
// --- Construct ---
func Construct():
    ScreenViewModel = CreateViewModel(MVVM_SettlerScreen)
    SetViewModel(ScreenViewModel)
    ScreenViewModel.BindToRoster(this)
    ScreenViewModel.OnSettlerListRebuilt.AddDynamic(this, RebuildGrid)

// --- Rebuild Grid ---
func RebuildGrid():
    // Clear old
    UniformGrid.ClearChildren()
    
    // Unregister old SlotViewModels
    for guid in OldGuidsArray:
        ScreenViewModel.UnregisterSlotViewModel(guid)
    
    OldGuidsArray.Empty()
    
    // Build new
    NumSettlers = ScreenViewModel.GetNumSettlers()
    
    for i = 0 to NumSettlers-1:
        // Get GUID
        SettlerGuid = ScreenViewModel.GetSettlerGuidByIndex(i)
        
        // Create slot widget
        SlotWidget = CreateWidget(WBP_SettlerSlot)
        
        // Create slot ViewModel
        SlotViewModel = NewObject(MVVM_SettlerSlot)
        SlotViewModel.SetSettlerGUID(SettlerGuid.ToString())
        SlotViewModel.SetSettlerName(ScreenViewModel.GetSettlerNameByGuid(SettlerGuid))
        
        // Get pawn and bind GAS
        SettlerPawn = ScreenViewModel.GetSettlerPawnByGuid(SettlerGuid)
        if SettlerPawn:
            SlotViewModel.SetOccupied(SettlerGuid.ToString(), SettlerName, SettlerPawn)
        
        // Register for GUID-based lookups
        ScreenViewModel.RegisterSlotViewModel(SettlerGuid, SlotViewModel)
        OldGuidsArray.Add(SettlerGuid)
        
        // Set ViewModel on widget (MVVM binding)
        SlotWidget.SetViewModel(SlotViewModel)
        
        // Add to grid
        Row = i / NumColumns
        Column = i % NumColumns
        UniformGrid.AddChildToUniformGrid(SlotWidget, Row, Column)
```

### WBP_SettlerSlot

```blueprint
// --- Construct ---
func Construct():
    SlotViewModel = GetViewModel() // MVVM_SettlerSlot
    CachedSettlerGuid = ParseGuid(SlotViewModel.GetSettlerGUID())

// --- Button Click ---
func OnSlotButtonClicked():
    SettlerPawn = SlotViewModel.GetSettlerActor()
    ScreenViewModel = GetGameInstance().GetSubsystem(MVVM_SettlerScreen) // or parent reference
    ScreenViewModel.RequestShowDetails(CachedSettlerGuid, SettlerPawn)
```

---

## Key Differences from Old System

| Old (C++ managed ViewModels) | New (Blueprint managed ViewModels) |
|-------------------------------|-------------------------------------|
| C++ creates SlotViewModels    | Blueprint creates SlotViewModels    |
| C++ owns TArray<SlotViewModels> | Blueprint owns widgets + ViewModels |
| SlotIndex-based lookups       | GUID-based lookups                  |
| RefreshSlotsFromRoster fills VMs | RefreshDataFromRoster broadcasts event → Blueprint rebuilds |
| EnsureSlotCount() manages VMs | Blueprint manages widget lifecycle  |

---

## Troubleshooting

### Problem: SlotViewModel not binding to widget
**Solution**: Ensure you call `SlotWidget->SetViewModel(SlotViewModel)` after creating the ViewModel.

### Problem: GUID lookups return null
**Solution**: Ensure you call `ScreenViewModel->RegisterSlotViewModel(Guid, SlotVM)` after creating each SlotViewModel.

### Problem: Grid not rebuilding on roster change
**Solution**: Bind `OnSettlerListRebuilt` to `RebuildGrid` custom event in Widget Blueprint.

### Problem: Details panel not showing
**Solution**: Ensure `RequestShowDetails` is called with valid GUID and SettlerActor, then bind `OnRequestShowDetails` event.

---

## Benefits of New Architecture

1. **Blueprint Ownership**: Full control over widget lifecycle in Blueprint
2. **GUID-based Identity**: Stable identity across grid rebuilds
3. **Clean Separation**: C++ = Data Provider, Blueprint = View Logic
4. **No C++ EditDefaultsOnly**: Easier to test and iterate in Blueprint
5. **Scalable**: Easy to add filtering, sorting, pagination in Blueprint

---

## Migration Checklist

- [x] Remove `SettlerSlotViewModelClass` and `DetailsViewModelClass` from C++
- [x] Remove `TArray<SlotViewModels>` from C++
- [x] Remove `EnsureSlotCount()` and `EnsureDetailsVM()` from C++
- [x] Add Data API: `GetSettlerGuidByIndex`, `GetSettlerNameByGuid`, etc.
- [x] Add Registration API: `RegisterSlotViewModel`, `UnregisterSlotViewModel`
- [ ] Update WBP_SettlerScreen to create ViewModels in Blueprint
- [ ] Update WBP_SettlerSlot to cache GUID and use it for events
- [ ] Update Details Panel to receive GUID-based events
- [ ] Test grid rebuild on roster add/remove
- [ ] Test slot selection and details panel

---

## Next Steps

1. Open WBP_SettlerScreen and implement RebuildGrid custom event
2. Test with adding/removing settlers at runtime
3. Verify GUID-based lookups work across widgets
4. Optimize grid rebuild performance if needed (widget pooling)

Brauchst du Hilfe bei der Blueprint-Implementierung?
