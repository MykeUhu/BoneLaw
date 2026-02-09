# GAS Architecture & Best Practices - BoneLaw

## Überblick

BoneLaw ist ein **Event-driven** Story/Survival Game (wie Kingdom Come: Deliverance), KEIN Action-RPG (wie Diablo/Path of Exile). Daher unterscheidet sich unsere GAS-Architektur von typischen Tutorial-Patterns.

---

## 1. Tutorial Pattern: AAuraEffectActor (Action-RPG)

### Was der Tutorial Actor macht:
```cpp
class AAuraEffectActor : public AActor
{
    // Overlap-based: Trifft auf Spieler -> Wendet GameplayEffect an
    void ApplyEffectToTarget(AActor* Target, TSubclassOf<UGameplayEffect> GE);
    void OnOverlap(AActor* Target);
    void OnEndOverlap(AActor* Target);
    
    // Policies: Wann effect applyen/removen
    EEffectApplicationPolicy InstantEffectApplicationPolicy;
    EEffectRemovalPolicy InfiniteEffectRemovalPolicy;
    
    // Tracked handles für removal
    TMap<FActiveGameplayEffectHandle, UAbilitySystemComponent*> ActiveEffectHandles;
};
```

### Wann dieser Pattern gut ist:
- **Action-RPGs**: Spieler läuft durch Welt, kollidiert mit Items
- **Pickup Items**: Health Potions, Mana Flasks, Buffs
- **Environmental Effects**: Fire zones, healing areas, damage traps
- **Active Player Movement**: Spieler steuert Charakter direkt (WASD)

### Warum das für BoneLaw NICHT passt:
1. ❌ **Wir haben keine Overlap-basierte Interaktion** - Spieler bewegt sich nicht frei in der Welt
2. ❌ **Wir haben keine Pickups** - Alles ist Event-driven via Choices
3. ❌ **Kein Active Movement** - Events sind text-basiert, nicht spatial
4. ❌ **AActor Overhead** - Würde leere Actors im Level spawnen nur um Effects zu applyen

---

## 2. BoneLaw Pattern: UStoneOutcomeExecutor (Event-driven)

### Was unser Executor macht:
```cpp
class UStoneOutcomeExecutor : public UObject
{
    // Event-driven: Choice gewählt -> Outcomes ausführen
    void ExecuteOutcomes(const TArray<FStoneOutcome>& Outcomes);
    
    // Verschiedene Outcome-Typen:
    void ApplyAttributeDelta(FGameplayTag AttributeTag, float Delta);
    void ApplyGameplayEffect(TSubclassOf<UGameplayEffect> GE);
    void ModifyTags(FGameplayTagContainer Tags, bool bAdd);
    void ScheduleEvent(FStoneScheduledEvent Scheduled);
    // ... etc
};
```

### Warum dieser Pattern für uns richtig ist:
1. ✅ **Event-driven Flow**: User wählt Choice -> Outcomes werden executed
2. ✅ **Kein Actor Overhead**: Pure UObject, kein Spawning/Cleanup nötig
3. ✅ **Centralized Logic**: Alle Outcome-Execution an einer Stelle
4. ✅ **Trace Support**: Kann direkt ins RunTrace schreiben
5. ✅ **Multiplayer Ready**: Authority Checks an zentraler Stelle

---

## 3. GAS Integration Comparison

### Tutorial (AAuraEffectActor):
```cpp
// Spatial-based
void AAuraEffectActor::ApplyEffectToTarget(AActor* Target, TSubclassOf<UGameplayEffect> GE)
{
    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    Context.AddSourceObject(this); // Actor ist Source
    
    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GE, ActorLevel, Context);
    FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
}
```

### BoneLaw (StoneOutcomeExecutor):
```cpp
// Event-driven
void UStoneOutcomeExecutor::ApplyGameplayEffect(TSubclassOf<UGameplayEffect> GE)
{
    UAbilitySystemComponent* ASC = GetTargetASC(); // Von PlayerState
    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    Context.AddSourceObject(EventDataAsset); // Event ist Source
    
    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GE, 1.0f, Context);
    FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
    
    TraceBuffer->AddOutcome(OutcomeType::GameplayEffect, GE->GetName()); // Logging
}
```

---

## 4. Wann welcher Pattern?

| Feature | Tutorial Pattern (Actor) | BoneLaw Pattern (Executor) |
|---------|-------------------------|---------------------------|
| **Game Type** | Action-RPG, ARPG | Event-driven, Story-based |
| **Player Movement** | Active (WASD) | Passive (Text Choices) |
| **Interaction** | Overlap/Collision | Button Clicks |
| **Item Pickups** | ✅ Gut für Potions/Loot | ❌ Nicht vorhanden |
| **Environmental Effects** | ✅ Fire zones, Buffs | ❌ Nicht vorhanden |
| **Event System** | ❌ Nicht vorhanden | ✅ Kern-Mechanik |
| **Multiplayer** | Needs replication setup | Pure Authority checks |
| **Performance** | Actor spawning overhead | Lightweight UObject |

---

## 5. Optional: Hybrid Approach für Zukunft

### Falls wir später NPCs / Familie / Siedlung haben:

**Option A: Bleib bei Executor (Recommended)**
- NPCs haben auch ASC (AStoneNPChar)
- Events können NPCs als Target angeben
- StoneOutcomeExecutor nimmt NPC's ASC statt Player ASC
- ✅ Consistent mit Event-System
- ✅ Kein neuer Code-Pattern

**Option B: EffectActor für NPCs (Only if needed)**
- Erstelle `AStoneEffectActor` nur für räumliche NPC-Interactions
- Z.B. "NPC läuft durch Feuer -> nimmt Schaden"
- Tutorial Pattern wird relevant wenn NPCs frei laufen
- ⚠️ Nur wenn wir wirklich spatial interactions brauchen

---

## 6. Current Architecture (Demo 0.0.2 Alpha)

```
┌─────────────────────────────────────────────┐
│           Player Makes Choice               │
└──────────────────┬──────────────────────────┘
                   │
                   ▼
        ┌──────────────────────┐
        │  StoneRunSubsystem   │ ◄──── Orchestrator
        │  - SelectChoice()    │
        └──────────┬───────────┘
                   │
                   ▼
        ┌──────────────────────┐
        │ StoneOutcomeExecutor │ ◄──── Pure UObject
        │  - ExecuteOutcomes() │
        └──────────┬───────────┘
                   │
        ┌──────────┴────────────────────────────────┐
        │                                           │
        ▼                                           ▼
┌───────────────────┐                   ┌────────────────────┐
│ Gameplay Effects  │                   │  Other Outcomes    │
│ (via ASC)         │                   │  - Tags            │
│ - AttributeDelta  │                   │  - Schedule Events │
│ - Apply GE        │                   │  - Pool Mods       │
└───────────────────┘                   └────────────────────┘
        │                                           │
        └──────────┬────────────────────────────────┘
                   │
                   ▼
        ┌──────────────────────┐
        │   StoneRunTrace      │ ◄──── Logging/Debugging
        │   (Audit Trail)      │
        └──────────────────────┘
```

### Key Points:
- ✅ **StoneOutcomeExecutor ist unser "ApplyEffectToTarget"**
- ✅ **Events sind unsere "Pickup Items"**
- ✅ **Choices sind unsere "Overlap Trigger"**
- ✅ **ASC ist in PlayerState** (standard Unreal best practice)
- ✅ **Multiplayer Ready**: Authority checks im Executor

---

## 7. Best Practices für BoneLaw GAS

### ✅ DO:
1. **Use StoneOutcomeExecutor** für alle GameplayEffect Applications
2. **ASC in PlayerState** behalten (not in Character)
3. **Authority Checks** vor ASC modifications
4. **Trace alle Outcomes** für Debugging/Save System
5. **GameplayTags** für State Management (Action.*, Event.*)

### ❌ DON'T:
1. **Keine AActor für Effect Application** (außer spatial interactions)
2. **Keine GE Specs cachen** - immer fresh erstellen
3. **Keine Client-side ASC Modifications** - nur Server/Standalone
4. **Keine hardcoded Attribute Values** - immer via MMC

---

## 8. Zukünftige Erweiterungen

### Wenn NPCs/Familie kommen:
```cpp
// Option: Executor erweitern für Multi-Target
void UStoneOutcomeExecutor::SetTargetASC(UAbilitySystemComponent* InASC)
{
    TargetASC = InASC; // Kann Player oder NPC sein
}

// Usage:
Executor->SetTargetASC(NPC->GetAbilitySystemComponent());
Executor->ExecuteOutcomes(Outcomes); // Affects NPC statt Player
```

### Wenn räumliche Interactions nötig werden:
```cpp
// Nur dann: AStoneEffectActor erstellen
class AStoneEffectActor : public AActor
{
    // Tutorial Pattern aber Stone-aware
    UPROPERTY()
    TObjectPtr<UStoneOutcomeExecutor> OutcomeExecutor;
    
    void OnOverlap(AActor* Target)
    {
        // Delegate to Executor (reuse logic)
        OutcomeExecutor->SetTargetASC(Target->GetASC());
        OutcomeExecutor->ExecuteOutcomes(Outcomes);
    }
};
```

---

## Fazit

**BoneLaw braucht KEINEN AAuraEffectActor-Pattern** weil:
1. Wir sind Event-driven, nicht Action-driven
2. Wir haben keine räumlichen Pickups/Overlaps
3. StoneOutcomeExecutor macht exakt was wir brauchen
4. Tutorial Pattern ist für Action-RPGs optimiert

**Der Tutorial ist trotzdem wertvoll** weil:
- ✅ Zeigt wie GameplayEffects richtig applyen
- ✅ Zeigt Handle Management für Infinite Effects
- ✅ Zeigt Context/Spec pattern
- ✅ Kann später für NPCs relevant werden

**Unsere Lösung ist Best Practice** für Event-driven Games! 🎯
