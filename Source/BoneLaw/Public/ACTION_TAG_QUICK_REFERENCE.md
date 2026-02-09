# Action Tag Quick Reference

**Quick lookup for Action vs Event tags in BoneLaw Demo 0.0.2 Alpha**

---

## Tag Hierarchy

```
Action.*                    // Action-driven flows (ActionSubsystem)
├─ Action.Travel.*          // Travel actions
├─ Action.Gather.*          // Gather actions
└─ Action.Explore.*         // Explore actions

Event.*                     // Event-driven flows (EventResolver)
├─ Event.Travel.*           // Generic travel events (backward compat)
├─ Event.Hunt.*             // Hunt events
├─ Event.Forage.*           // Forage events
└─ ...
```

---

## When to Use Action.* vs Event.*

| Use Case | Use Tag | Example |
|----------|---------|---------|
| Random event during travel action | `Action.Travel.Outbound` | "You find berries while traveling" |
| Random event during gather action | `Action.Gather.Outbound` | "You spot a deer while gathering" |
| Arrival event (any action) | `Action.<Type>.Arrival` | "You arrive at the hunting grounds" |
| Generic event (not tied to action) | `Event.<Category>.*` | "A stranger visits your camp at night" |
| Legacy/Custom action | `Event.Travel.*` | Backward compatibility |

---

## Full Tag List

### Travel Actions
```
Action.Travel.Outbound      // Random events on the way out
Action.Travel.Arrival       // Event at destination (gate event)
Action.Travel.Return        // Random events on the way back
Action.Travel.ReturnHome    // Event when arriving home
```

### Gather Actions
```
Action.Gather.Outbound      // Random events while going to gather site
Action.Gather.Arrival       // Event at gathering location
Action.Gather.Return        // Random events while returning with resources
Action.Gather.ReturnHome    // Event when arriving home with gathered items
```

### Explore Actions
```
Action.Explore.Outbound     // Random events during exploration travel
Action.Explore.Arrival      // Event at exploration site (discovery)
Action.Explore.Return       // Random events while returning from exploration
Action.Explore.ReturnHome   // Event when arriving home from exploration
```

---

## Event Pack Configuration Examples

### Example 1: Event triggers during ANY action outbound phase
```cpp
Event Requirements:
  MustMatchQuery: 
    Any: [Action.Travel.Outbound, Action.Gather.Outbound, Action.Explore.Outbound]
```

### Example 2: Event ONLY for travel actions
```cpp
Event Requirements:
  RequiredTagsAll: [Action.Travel.Outbound]
```

### Example 3: Event at ANY arrival
```cpp
Event Requirements:
  MustMatchQuery:
    Any: [Action.Travel.Arrival, Action.Gather.Arrival, Action.Explore.Arrival]
```

### Example 4: Event ONLY for explore arrival (discovery)
```cpp
Event Requirements:
  RequiredTagsAll: [Action.Explore.Arrival]
  EventTags: [Event.Explore]  // Also tag as exploration event
```

### Example 5: Generic event (not tied to actions)
```cpp
Event Requirements:
  RequiredTagsAll: [State.Night, State.InCave]
  BlockedTagsAny: [State.OnAction]  // Don't trigger during actions
```

---

## Blueprint Tag Checks

### Check if ANY action is running
```blueprint
Get Run Subsystem → Has State Tag → Action.Travel.Outbound
                     OR
Get Action Subsystem → Is Action Running
```

### Check specific action phase
```blueprint
Get Run Subsystem → Has All Tags → [Action.Travel.Arrival]
```

### Check NOT during action
```blueprint
Get Run Subsystem → Has Tag → State.OnAction → NOT
```

---

## Common Patterns

### Pattern: "Event at any destination"
```
RequiredTagsAll: ANY of [Action.*.Arrival]
Use MustMatchQuery with "Any" operator
```

### Pattern: "Event only when NOT traveling"
```
BlockedTagsAny: [State.OnAction, State.OnTravel]
```

### Pattern: "Event specific to travel + night"
```
RequiredTagsAll: [Action.Travel.Outbound, State.Night]
```

### Pattern: "Event at home after exploration"
```
RequiredTagsAll: [Action.Explore.ReturnHome]
EventTags: [Event.Explore]
```

---

## Debugging Tips

1. **Log current tags:** Enable verbose logging in StoneRunSubsystem
2. **Check pack activation:** Verify ActionDef.PackIdsToActivate includes correct packs
3. **Verify requirements:** Use Content Validator to check event requirements
4. **Test phase timing:** Use console command `DebugActionPhase` (if implemented)

---

## Migration from Old System

| Old Tag | New Tag (Travel) | New Tag (Gather) | New Tag (Explore) |
|---------|------------------|------------------|-------------------|
| `Event.Travel.Outbound` | `Action.Travel.Outbound` | `Action.Gather.Outbound` | `Action.Explore.Outbound` |
| `Event.Travel.Arrival` | `Action.Travel.Arrival` | `Action.Gather.Arrival` | `Action.Explore.Arrival` |
| `Event.Travel.Return` | `Action.Travel.Return` | `Action.Gather.Return` | `Action.Explore.Return` |
| `Event.Travel.ReturnHome` | `Action.Travel.ReturnHome` | `Action.Gather.ReturnHome` | `Action.Explore.ReturnHome` |

**Note:** Old `Event.Travel.*` tags still work for Custom actions (backward compatibility).

---

## Version History

- **0.0.2 Alpha** (2026-02-09): Initial Action tag system
- **0.0.1 Alpha**: Legacy Event.Travel.* only

---

**Quick Tip:** Use tag hierarchies in Unreal's GameplayTag editor to visualize the structure!
