# Demo Start Region Content (Pack_Start)

This recipe creates a deterministic one-shot **region selection** start event for the demo.

## Goals
- Region selection is an **event** (nice text + image), but it must be **one-click** and **never repeat**.
- After selection the run enters **idle (cave)** so the player can start an action.
- No other events may appear from the start content.

## 1) Create Pack Asset
Create **DataAsset → StoneEventPackData**

- **Asset Name:** `Pack_Start`
- **PackId:** `Pack_Start`
- **DisplayName:** `Start (Demo)`
- **RequiredTagsAll:** *(empty)*
- **BlockedTagsAny:** `State.RunStarted` *(optional but recommended)*
- **bAutoUnlockWhenRequirementsMet:** ✅ true
- **bPreloadOnUnlock:** ✅ true
- **Phase:** `0`
- **Events:**
    - Index 0:
        - **EventId:** `WL_Start_SelectRegion`
        - **WeightMultiplier:** `1.0`
        - **EntryTags:** *(empty)*

> Studio rule: `PackId == AssetName` and `EventId == asset name`.

## 2) Create Event Asset
Create **DataAsset → StoneEventData**

- **Asset Name:** `WL_Start_SelectRegion`
- **EventId:** `WL_Start_SelectRegion`
- **Title:** `Choose your starting grounds`
- **Body:** *(paste the text below; MultiLine enabled)*

```
You have one chance to decide where the clan takes shelter.

**The Core Hollow (Demo Region)**
- Water: a shallow spring runs year‑round
- Food: berries in summer, small game trails nearby
- Materials: loose flint and sharp stone at the ridge
- Shelter: a dry cave mouth, but cold winds at night

What this means:
- Early survival is stable, but exploration is tempting.
- Wildlife sightings are common; injuries are possible.
```

- **Image:** *(optional)*
- **EventTags:** `Event.Ambient` *(or leave empty)*
- **Requirement:** *(empty)*
- **BaseWeight:** `100`
- **ArcId:** `None`

### Choices
Add **exactly 1** choice (Index 0):

- **ChoiceText:** `Begin in the Core Hollow`
- **Requirement:** *(empty)*
- **LockMode:** `Disabled` *(default)*

#### Outcomes (success)
Add these outcomes in order:

1) **AddTags**
    - Tags: `State.RunStarted`, `State.RegionSelected`, `State.InCave`, `Region.Core`

2) **SetFocusTag**
    - Tags: `Focus.Explore`
    - (All other outcome fields are ignored for `SetFocusTag`.)

3) **PoolRemoveEvent**
    - EventId: `WL_Start_SelectRegion`

> Why this is deterministic:
> - `State.InCave` makes the run idle between events.
> - `PoolRemoveEvent` ensures the start event can never be re-picked even if something forces a random pick later.

#### FailOutcomes
Leave empty.

#### Schedules
Leave empty.

## 3) Hook it into StartNewRun (SSOT)
Wherever you build `FStoneRunConfig` for a new run (GameMode/PlayerController/UI), set:
- `Config.StartingPackIds = [ Pack_Start, Pack_Core ]` *(or only Pack_Start for ultra-min demo)*

Recommended demo flow:
1) Start with `Pack_Start` only.
2) The region choice adds `State.RunStarted` and idles the run.
3) Your action panel becomes the main interaction.
4) Actions unlock `Pack_Core` / `Pack_Explore` via tags or explicit pack activation.
