# Refactoring: Action Tags & Choice Outcome Preview

**Version:** Demo 0.0.2 Alpha  
**Date:** 2026-02-09  
**Status:** ✅ Complete - No Legacy/Deprecated Code

---

## Overview

This refactoring introduces three major improvements to the BoneLaw event and action systems:

1. **Choice Button Enhancement**: Outcome preview with attribute changes, icons, and return-forced indicators
2. **Action-based GameplayTags**: Separation of Action.* tags from Event.* tags for cleaner architecture
3. **Multiplayer Ready**: Authority checks for tag/pack modifications in ActionSubsystem

---

## 1. Choice Button Enhancement

### Changes Made

**`FStoneChoiceData` (StoneEventData.h)**
- Added `TSoftObjectPtr<UTexture2D> ChoiceIcon` - Optional icon for choice buttons
- Added `bool bForcesReturn` - Indicates if choice forces a dangerous return/end

**`UStoneChoiceButtonWidget`**
- New parameters in `InitChoice()`:
  - `const TArray<FStoneOutcome>& InOutcomes` - Possible outcomes for preview
  - `UTexture2D* InIcon` - Loaded icon texture
  - `bool bInForcesReturn` - Return-forced indicator
- New getters: `GetOutcomes()`, `GetChoiceIcon()`, `ForcesReturn()`
- New Blueprint event: `BP_OnOutcomesUpdated()` - Called after outcomes are set

**`UStoneEventPanelWidget::RebuildChoices()`**
- Now loads icon from `ChoiceData.ChoiceIcon` (synchronous load)
- Passes outcomes, icon, and bForcesReturn to button's `InitChoice()`

### Blueprint Implementation Required

In your WBP_ChoiceButton Blueprint child:
1. Add UI widgets for outcome preview (e.g., attribute deltas, icons)
2. Implement `BP_OnOutcomesUpdated()` event to populate preview widgets
3. Access data via getters: `GetOutcomes()`, `GetChoiceIcon()`, `ForcesReturn()`

**Example Blueprint Logic:**
```
BP_OnOutcomesUpdated()
├─ Get Outcomes → For Each Outcome
│  ├─ If Type == AttributeDelta
│  │  └─ Show AttributeTag.Name + Magnitude (e.g., "+5 Food")
│  ├─ If Type == AddTags
│  │  └─ Show Tag names as icons/text
│  └─ ...
├─ Get Choice Icon → Set Image Brush
└─ If Forces Return
   └─ Show warning indicator (e.g., red border, "⚠️")
```

---

## 2. Action-based GameplayTags

### Problem
Previously, actions used generic `Event.Travel.*` tags, mixing event-driven flows with action-driven flows.

### Solution
New tag hierarchy: **`Action.<ActionType>.<Phase>`**

### New Tags Added

**Travel Actions:**
- `Action.Travel.Outbound`
- `Action.Travel.Arrival`
- `Action.Travel.Return`
- `Action.Travel.ReturnHome`

**Gather Actions:**
- `Action.Gather.Outbound`
- `Action.Gather.Arrival`
- `Action.Gather.Return`
- `Action.Gather.ReturnHome`

**Explore Actions:**
- `Action.Explore.Outbound`
- `Action.Explore.Arrival`
- `Action.Explore.Return`
- `Action.Explore.ReturnHome`

### Tag Usage Pattern

**ActionSubsystem now selects tags based on `ActionType`:**

| ActionType | Outbound Tag | Arrival Tag | Return Tag | ReturnHome Tag |
|------------|--------------|-------------|------------|----------------|
| Travel | `Action.Travel.Outbound` | `Action.Travel.Arrival` | `Action.Travel.Return` | `Action.Travel.ReturnHome` |
| Gather | `Action.Gather.Outbound` | `Action.Gather.Arrival` | `Action.Gather.Return` | `Action.Gather.ReturnHome` |
| Explore | `Action.Explore.Outbound` | `Action.Explore.Arrival` | `Action.Explore.Return` | `Action.Explore.ReturnHome` |
| Custom | `Event.Travel.Outbound` | `Event.Travel.Arrival` | `Event.Travel.Return` | `Event.Travel.ReturnHome` |

**Backward Compatibility:** Custom actions still use `Event.Travel.*` tags.

### Event Pack Configuration

Update your Event Packs in Unreal Editor:

**Old (Generic):**
```
Event Requirements:
  RequiredTagsAll: [Event.Travel.Outbound]
```

**New (Action-Specific):**
```
Event Requirements:
  RequiredTagsAll: [Action.Travel.Outbound]  // For travel actions only
  RequiredTagsAll: [Action.Gather.Arrival]   // For gather actions only
```

**Or use Tag Queries for multiple action types:**
```
Event Requirements:
  MustMatchQuery: 
    Any: [Action.Travel.Outbound, Action.Gather.Outbound, Action.Explore.Outbound]
```

### Benefits
- ✅ Clear separation: Actions use `Action.*`, Events use `Event.*`
- ✅ Fine-grained control: Different events for different action types
- ✅ Scalable: Easy to add new action types (e.g., `Action.Hunt.*`)
- ✅ No breaking changes: Existing `Event.*` tags still work for Custom actions

---

## 3. Multiplayer Ready

### Authority Checks Added

**`UStoneActionSubsystem::ApplyRunSideEffects()`**
```cpp
if (World->GetNetMode() == NM_Client)
{
    UE_LOG(LogTemp, Warning, TEXT("[StoneAction] ApplyRunSideEffects called on client - ignoring"));
    return;
}
```

**`UStoneActionSubsystem::RemoveRunSideEffects()`**
```cpp
if (World->GetNetMode() == NM_Client)
{
    UE_LOG(LogTemp, Warning, TEXT("[StoneAction] RemoveRunSideEffects called on client - ignoring"));
    return;
}
```

### What This Means
- Tag/Pack modifications only happen on **Server** or **Standalone**
- Clients will log warnings if they attempt modifications (helps debugging)
- **No existing code changed** - only new authority guards added

### Future Multiplayer Work
When implementing full multiplayer:
1. Add replication to `UStoneRunSubsystem` (RunTags, ActivePacks, etc.)
2. Add replication to `UStoneActionSubsystem` (Phase, Progress, CurrentDef)
3. Convert subsystem delegates to replicated events
4. Player state already exists (`AStonePlayerState`) - ready for character-specific data

---

## Migration Guide

### For Content Designers

**1. Update Event Pack Requirements:**

Check all Event Packs that have `Event.Travel.*` tags in Requirements:
- If the event should trigger during **any travel action**, keep `Event.Travel.*` (backward compat)
- If the event should trigger during **specific action types**, use new `Action.*` tags

**2. Add Choice Icons (Optional):**

In Event DataAssets:
```
Choices[0]:
  ChoiceIcon: /Game/Textures/Icons/T_Icon_Berries
  bForcesReturn: false  // Set true if dangerous outcome
```

**3. Test Action Phases:**

Start a Travel/Gather/Explore action and verify:
- Random events use new `Action.*` tags
- Arrival events use new `Action.*` tags
- ReturnHome events use new `Action.*` tags

### For Blueprint Developers

**1. Update WBP_ChoiceButton:**

Implement `BP_OnOutcomesUpdated()` to show:
- Attribute deltas (Food +5, Health -10)
- Tag additions (show icons for status effects)
- Return-forced warning (red border, icon, etc.)

**2. Update Event Requirement Checks:**

If you have Blueprint logic checking tags, update:
```
Old: HasTag(Event.Travel.Outbound)
New: HasTag(Action.Travel.Outbound) OR HasMatchingTag(Action.*.Outbound)
```

### For Programmers

**No changes required** unless:
- You manually check `Event.Travel.*` tags in C++ → Update to `Action.*` tags
- You call `ApplyRunSideEffects()` directly → Already has authority checks

---

## Testing Checklist

- [ ] Start Travel action → Random events use `Action.Travel.*` tags
- [ ] Start Gather action → Random events use `Action.Gather.*` tags
- [ ] Start Explore action → Random events use `Action.Explore.*` tags
- [ ] Choice buttons show outcomes (attributes, icons, return indicator)
- [ ] Authority checks log warnings on client (if testing in multiplayer)
- [ ] Existing content still works (backward compat with `Event.Travel.*`)

---

## Known Limitations

1. **Icon Loading:** Currently synchronous (`LoadSynchronous()`). For large projects, consider async loading.
2. **Outcome Preview:** Blueprint must implement `BP_OnOutcomesUpdated()`. No default preview provided.
3. **Multiplayer:** Full replication not implemented yet - only authority checks added.

---

## Future Enhancements

1. **Async Icon Loading:** Avoid frame hitches with large textures
2. **Outcome Probability Display:** Show % chance for outcomes (requires gameplay effect extensions)
3. **Multiplayer Replication:** Full networked action/event system
4. **Action Chaining:** Queue multiple actions (e.g., Travel → Gather → Return)

---

## Questions?

Contact: Senior Dev Team  
Version: Demo 0.0.2 Alpha  
Last Updated: 2026-02-09
