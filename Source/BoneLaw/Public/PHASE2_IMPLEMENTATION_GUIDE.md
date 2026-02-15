# BoneLaw Phase 2: Settler Roster & Assignments - Implementation Guide

**Date:** 2026-02-11  
**Engine:** Unreal Engine 5.7  
**Architecture:** ID-driven, Data-only persistence, SSOT/DRY

---

## Overview

Phase 2 extends BoneLaw from single-player to **multiple settlers** with individual attributes, assignments, and persistent identity. This is implemented as a **Commander-style system** where:

- Player controls a FreeCam/Spectator and issues commands via UI
- Multiple AI settlers exist, each with their own GAS (Ability System Component)
- Settlers can be assigned to tasks (world actors like gathering spots, crafting stations)
- All state is **ID-driven** (FGuid), never pointer-based
- SaveGame stores **data-only** (NO UObject pointers, NO ASC references)

---

## Architecture Principles (SSOT/DRY)

### 1. ID-Driven Design

**Every persistent entity has a stable FGuid:**
- **Settlers:** `FGuid SettlerId` (via `UStoneWorldIdComponent` or saved data)
- **Task Actors:** `FGuid WorldObjectId` (via `UStoneWorldIdComponent`)
- **Assignments:** `FGuid AssignmentId`

**Why?** Pointers are invalid after save/load. IDs remain stable across sessions.

### 2. Data-Only Persistence

**SaveGame stores DATA, never runtime references:**
- ❌ **NEVER** save `AActor*`, `UAbilitySystemComponent*`, `UUserWidget*`
- ✅ **ALWAYS** save `FGuid`, `FGameplayTag`, primitive types (float, int, FString)

**Attributes are saved as:**
```cpp
TArray<FSavedAttribute>  // { FGameplayTag AttributeTag; float Value; }
```

**Why?** Attributes are dynamically registered via GameplayTags. Hardcoding names breaks DRY.

### 3. Reconstruction on Load

**On load, we rebuild runtime state from saved data:**
1. Spawn settler pawns
2. Initialize ASC + ActorInfo
3. Apply saved attributes (Max → Vitals → derived secondaries)
4. Apply saved tags
5. Grant saved abilities
6. Restore assignments (if any)
7. Reopen interrupted events (if any)

**Why?** ASC is runtime-only. GAS cannot be serialized directly.

---

## Core Systems

### A) UStoneWorldIdComponent

**Purpose:** Persistent identity for world actors and settlers.

**Usage:**
```cpp
// Add to any actor that needs stable identity
UStoneWorldIdComponent* IdComp = Actor->FindComponentByClass<UStoneWorldIdComponent>();
FGuid ActorId = IdComp->GetWorldId();
```

**Location:**
- Header: `BoneLaw/Public/Core/Components/StoneWorldIdComponent.h`
- Source: `BoneLaw/Private/Core/Components/StoneWorldIdComponent.cpp`

**Key Points:**
- Auto-generates ID in `BeginPlay()` if not set
- SaveGame-enabled to persist across sessions
- Blueprint accessible for editor workflows

---

### B) UStoneRosterSubsystem

**Purpose:** Authoritative owner of settler roster + assignments.

**Responsibilities:**
- Manage settler roster (spawn, despawn, lookup by ID)
- Manage assignments (assign task, track progress, handle completion)
- Provide runtime APIs for UI
- Persist/restore settler state

**Location:**
- Header: `BoneLaw/Public/Runtime/StoneRosterSubsystem.h`
- Source: `BoneLaw/Private/Runtime/StoneRosterSubsystem.cpp`

**Key APIs:**

```cpp
// Roster Management
void InitializeRoster(const TArray<FSavedSettler>& SavedSettlers);
AStoneBaseChar* GetOrSpawnSettlerPawn(const FGuid& SettlerId);
FSavedSettler GetSettlerInfo(const FGuid& SettlerId);
TArray<FGuid> GetAvailableSettlerIds();

// Assignment Management
bool AssignSettlerToTask(FGuid SettlerId, FGuid TaskActorId, FGameplayTag TaskTag, double DurationSeconds);
void CompleteAssignment(const FGuid& SettlerId, bool bSuccess);
void CancelAssignment(const FGuid& SettlerId);

// Persistence
TArray<FSavedSettler> GatherRosterState();
void ApplyRosterState(const TArray<FSavedSettler>& SavedSettlers);

// Runtime Lookups (UI)
UStoneAbilitySystemComponent* GetSettlerASC(const FGuid& SettlerId);
TArray<FSavedAttribute> GetSettlerAttributesSnapshot(const FGuid& SettlerId);
```

**Integration:**
```cpp
// In Blueprint or C++:
UStoneRosterSubsystem* Roster = GetWorld()->GetSubsystem<UStoneRosterSubsystem>();
TArray<FGuid> Available = Roster->GetAvailableSettlerIds();
Roster->AssignSettlerToTask(SettlerIds[0], TaskActorId, TaskTag, 300.0);
```

---

### C) SaveGame Extension (ULoadScreenSaveGame)

**New Fields (Phase 2):**

```cpp
// Versioning
int32 SaveVersion = 2;  // Increment on format changes

// Roster
TArray<FSavedSettler> SavedSettlers;

// Event context (for interrupted events)
FSavedOpenEventContext OpenEventContext;

// Run state
TArray<FName> ActivePackIds;
TSet<FName> SeenEventIds;
```

**Migration from SaveVersion 1:**

When loading an old save (SaveVersion < 2):
1. Create a default settler entry from legacy fields:
   - `Strength`, `Food`, `Health`, etc. → `SavedSettlers[0].Attributes`
   - `SavedAbilities` → `SavedSettlers[0].GrantedAbilities`
2. Generate new `SettlerId` for legacy player
3. Mark legacy fields as unused (don't write them on next save)

**Migration Code (Example):**

```cpp
void MigrateSaveFromV1(ULoadScreenSaveGame* SaveGame)
{
    if (SaveGame->SaveVersion >= 2)
        return;  // Already migrated
    
    // Create default settler from legacy fields
    FSavedSettler DefaultSettler;
    DefaultSettler.SettlerId = FGuid::NewGuid();
    DefaultSettler.DisplayName = SaveGame->PlayerName;
    
    // Migrate attributes
    const FStoneGameplayTags& Tags = FStoneGameplayTags::Get();
    DefaultSettler.Attributes.Add(FSavedAttribute(Tags.Attributes_Vital_Health, SaveGame->Health));
    DefaultSettler.Attributes.Add(FSavedAttribute(Tags.Attributes_Vital_Food, SaveGame->Food));
    // ... etc for all attributes
    
    // Migrate abilities
    DefaultSettler.GrantedAbilities = SaveGame->SavedAbilities;
    
    SaveGame->SavedSettlers.Add(DefaultSettler);
    SaveGame->SaveVersion = 2;
    
    UE_LOG(LogTemp, Log, TEXT("[SaveMigration] Migrated save from V1 to V2"));
}
```

---

## Data Structures

### FSavedAttribute

```cpp
USTRUCT(BlueprintType)
struct FSavedAttribute
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag AttributeTag;  // e.g. Attributes.Vital.Food
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Value;
};
```

**Why GameplayTag?** Attributes are dynamically registered. Using tags keeps it DRY.

---

### FSavedAssignment

```cpp
USTRUCT(BlueprintType)
struct FSavedAssignment
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid AssignmentId;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag TaskTag;  // e.g. Task.Woodcutting
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid TaskActorId;  // World ID of task actor
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    double StartTimeSeconds;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    double DurationSeconds;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsAwayWorking;  // True if settler is "away" (despawned/hidden)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform ReturnTransform;  // Where settler reappears
};
```

---

### FSavedSettler

```cpp
USTRUCT(BlueprintType)
struct FSavedSettler
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid SettlerId;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform LastKnownTransform;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer SettlerTags;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FSavedAbility> GrantedAbilities;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FSavedAttribute> Attributes;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FSavedAssignment CurrentAssignment;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasAssignment;
};
```

---

### FSavedOpenEventContext

```cpp
USTRUCT(BlueprintType)
struct FSavedOpenEventContext
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag EventTag;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EventId;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid SettlerId;  // Which settler is involved
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid AssignmentId;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid TaskActorId;
};
```

**Usage:** If an event is open during save, store context here to reopen on load.

---

## Load Order (Critical!)

**Correct sequence for loading:**

1. **Load SaveGame** from disk
2. **Migrate** if `SaveVersion < 2`
3. **Initialize RosterSubsystem:** `Roster->InitializeRoster(SaveGame->SavedSettlers)`
4. **For each settler:**
   - Spawn pawn (if needed)
   - Initialize ASC + ActorInfo
   - Apply **Max** attributes first (MaxHealth, MaxFood, etc.)
   - Apply **Vitals** (Health, Food, etc.)
   - Apply **Persistent Tags**
   - Grant **Abilities**
   - Restore **Assignment** (if `bHasAssignment == true`)
5. **Reopen interrupted event** (if `OpenEventContext.IsValid()`)
6. **Resume assignments** (start timers, etc.)

**Why this order?** Max attributes must exist before vitals. Tags/abilities may modify derived attributes.

---

## Event System Integration

**Events now carry settler context:**

When an event occurs during an assignment:
- Event must know **which settler** is involved
- UI resolves choices for that specific settler
- After choice, assignment continues or completes

**Implementation:**

```cpp
// In event resolver or outcome executor:
FSavedOpenEventContext Context;
Context.EventTag = CurrentEventTag;
Context.SettlerId = AssignmentOwnerId;
Context.AssignmentId = AssignmentId;
Context.TaskActorId = TaskActorId;

// Store in SaveGame if player saves during event
SaveGame->OpenEventContext = Context;
```

**On load:**

```cpp
if (SaveGame->OpenEventContext.IsValid())
{
    // Queue event for reopening
    RunSubsystem->QueueEventByTag(SaveGame->OpenEventContext.EventTag, true);
}
```

---

## Blueprint Integration

### UI: Assign Settler to Task

```blueprint
// Get available settlers
UStoneRosterSubsystem* Roster = GetWorld()->GetSubsystem<UStoneRosterSubsystem>();
TArray<FGuid> AvailableSettlers = Roster->GetAvailableSettlerIds();

// Display in UI list
for (FGuid SettlerId : AvailableSettlers)
{
    FSavedSettler Info = Roster->GetSettlerInfo(SettlerId);
    AddSettlerEntryToList(Info.DisplayName, SettlerId);
}

// On settler selected:
void OnSettlerSelected(FGuid SelectedSettlerId, FGuid TaskActorId)
{
    Roster->AssignSettlerToTask(SelectedSettlerId, TaskActorId, TaskTag, 300.0);
}
```

### Task Actor: Store World ID

In Blueprint:
1. Add `UStoneWorldIdComponent` to Task Actor
2. Component auto-generates ID on first spawn
3. Reference task by ID in assignments

---

## Testing Checklist

### Phase 2 Requirements:

- [x] **A1:** Persistent Settler Identity (UStoneWorldIdComponent)
- [x] **A2:** SaveGame stores roster (FSavedSettler array)
- [x] **A3:** Rebuild settlers on load (no saved ASC pointers)
- [x] **B1:** Persistent ID for Task Actors (UStoneWorldIdComponent)
- [x] **B2:** Data-only saved assignment (FSavedAssignment)
- [x] **B3:** Runtime ownership (UStoneRosterSubsystem)
- [x] **C1:** Event context carries settler ID
- [x] **C2:** Persistence of interrupted events (FSavedOpenEventContext)
- [x] **D1:** SaveVersion + migration from V1

### Manual Testing:

1. **Create 2 settlers** via RosterSubsystem
2. **Assign settler to task** (woodcutting, etc.)
3. **Save game** while assignment is active
4. **Load game** → settlers restored with correct attributes + assignment
5. **Complete assignment** → settler returns to home transform
6. **Trigger event during assignment** → context preserved
7. **Save during open event** → event reopens on load

---

## Future Enhancements (Post-Phase 2)

### Navigation & Pathfinding

Currently stubbed in `UStoneRosterSubsystem::StartSettlerNavigation()`.

**TODO:**
- Integrate with UE5 Navigation System
- Move settler to task location
- On arrival: call `OnSettlerArrival()` → mark as "away working"

### Action Timeline Integration

Link with `UStoneActionSubsystem` for:
- Real-time action tracking (outbound → arrival → return)
- Random events during travel
- UI progress bars

### Multi-Settler Events

Events that affect multiple settlers:
- Store `TArray<FGuid> InvolvedSettlers` in event context
- Resolve choices for each settler
- Apply outcomes based on individual attributes

---

## Files Changed

### New Files:

1. `BoneLaw/Public/Core/Components/StoneWorldIdComponent.h`
2. `BoneLaw/Private/Core/Components/StoneWorldIdComponent.cpp`
3. `BoneLaw/Public/Runtime/StoneRosterSubsystem.h`
4. `BoneLaw/Private/Runtime/StoneRosterSubsystem.cpp`

### Modified Files:

1. `BoneLaw/Public/Data/StoneTypes.h`
   - Added: `FSavedAttribute`, `FSavedAssignment`, `FSavedSettler`, `FSavedOpenEventContext`

2. `BoneLaw/Public/Core/LoadScreenSaveGame.h`
   - Added: `SaveVersion`, `SavedSettlers`, `OpenEventContext`, `ActivePackIds`, `SeenEventIds`

---

## Compile & Validation

### Build Instructions:

1. Regenerate project files (right-click `.uproject` → Generate Visual Studio project files)
2. Build in Visual Studio (Development Editor configuration)
3. Launch editor

### Expected Warnings: **NONE**

If you see warnings about:
- Missing includes → Check `#include` paths
- Unresolved symbols → Verify `BoneLaw.Build.cs` has all modules
- Deprecated APIs → **Report immediately** (no legacy APIs allowed)

### Runtime Validation:

1. Open Output Log
2. Look for `[StoneRoster]` and `[StoneWorldId]` logs
3. Verify no errors on:
   - Roster initialization
   - Settler spawn
   - Assignment creation
   - Save/Load cycle

---

## Contact & Support

**Implementation by:** v0 AI Assistant  
**Reviewed by:** MykeUhu  
**Date:** 2026-02-11

For issues or questions:
1. Check this document first
2. Review Project Bible: `BoneLaw/Public/BoneLaw_Project_Bible.md`
3. Check GAS Architecture: `BoneLaw/Public/GAS_ARCHITECTURE_AND_BEST_PRACTICES.md`

---

**END OF PHASE 2 IMPLEMENTATION GUIDE**
