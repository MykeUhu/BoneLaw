# Settler Screen GUID-Based Mapping Guide

## Problem gelöst

**Vorher:**
- Slot-Widgets wurden über `SlotIndex` identifiziert
- Bei Grid-Rebuilds ging die Zuordnung Widget ↔ ViewModel ↔ Settler verloren
- Keine eindeutige Identifikation zwischen UI und Daten

**Jetzt:**
- Jedes SlotViewModel wird über **Settler-GUID** identifiziert (Primary Key)
- `SlotIndex` ist nur noch UI-Position, keine ID
- Klare Zuordnung: Widget ↔ GUID ↔ ViewModel ↔ Settler

---

## C++ Architektur (bereits implementiert)

### MVVM_SettlerScreen

```cpp
// GUID-basiertes Mapping (intern)
TMap<FGuid, TObjectPtr<UMVVM_SettlerSlot>> SlotViewModelsByGuid;

// Blueprint API
UFUNCTION(BlueprintPure)
UMVVM_SettlerSlot* GetSlotViewModelBySettlerGuid(const FGuid& SettlerId) const;

// Event: OnRequestShowDetails(FGuid SettlerGuid, AStoneBaseChar* Actor)
UPROPERTY(BlueprintAssignable)
FOnSettlerRequestShowDetails OnRequestShowDetails;
```

### MVVM_SettlerSlot

```cpp
// GUID als FGuid parsen
UFUNCTION(BlueprintPure)
FGuid GetSettlerGuidAsGuid() const;

// Status-Check
UFUNCTION(BlueprintPure)
bool IsOccupied() const;

// Interner String für MVVM-Binding
UPROPERTY(FieldNotify)
FString SettlerGUID;
```

---

## Blueprint Integration - Phase 1: Widget Setup

### WBP_SettlerSlot (einzelnes Slot-Widget)

1. **Variable hinzufügen:**
   ```
   Variable: CachedSettlerGuid (Type: Guid)
   ```

2. **Initialize Slot Function:**
   ```blueprint
   Function: InitializeSlot(SlotViewModel: MVVM_SettlerSlot)
   
   [Input] SlotViewModel
     |
     +--> [Get Settler Guid As Guid]
     |      |
     |      +--> [SET: CachedSettlerGuid]
     |
     +--> [Bind to SlotViewModel (MVVM)]
   ```

3. **Button OnClicked:**
   ```blueprint
   [OnClicked]
     |
     +--> [Call: RequestShowDetails on SlotViewModel]
   ```

---

## Blueprint Integration - Phase 2: Screen Widget

### WBP_SettlerScreen (Grid + Screen)

1. **Event: OnSettlerListRebuilt**
   ```blueprint
   [OnSettlerListRebuilt]
     |
     +--> [Clear UniformGrid children]
     |
     +--> [Get Num Settler Slots from SettlerScreenVM]
     |      |
     |      +--> [ForEachLoop: 0 to NumSlots-1]
     |            |
     |            +--> [Get Settler Slot ViewModel By Index(i)]
     |            |      |
     |            |      +--> [IsValid?]
     |            |            |
     |            |            +--> [Create WBP_SettlerSlot Widget]
     |            |                  |
     |            |                  +--> [Call InitializeSlot(SlotViewModel)]
     |            |                  |
     |            |                  +--> [Add to UniformGrid]
   ```

2. **Event: OnRequestShowDetails (Screen-Level)**
   ```blueprint
   [OnRequestShowDetails]  <-- Bindet EINMAL auf Screen-Level
     |
     |  Inputs: SettlerGuid (Guid), SettlerActor (StoneBaseChar)
     |
     +--> [Open Details Panel / Animate / etc.]
     |
     +--> [Details ViewModel ist bereits gebunden via C++]
   ```

---

## Blueprint Integration - Phase 3: GUID-basierte Lookups (Optional)

Falls du später **per GUID** auf ein Slot-Widget zugreifen willst:

### Beispiel: "Highlight Settler by GUID"

```blueprint
Function: HighlightSettlerByGuid(TargetGuid: Guid)

[Input: TargetGuid]
  |
  +--> [SettlerScreenVM: Get Slot ViewModel By Settler Guid(TargetGuid)]
         |
         +--> [IsValid?]
               |
               +--> [Find Widget in UniformGrid by matching CachedSettlerGuid]
                      |
                      +--> [Play Animation: Highlight]
```

---

## Migration von altem Code

### Alte API (noch unterstützt, aber deprecated):

```cpp
// ALTE Delegate-Signatur (entfernen wenn Blueprint umgestellt)
FOnSettlerRequestShowDetails(int32 SlotIndex, AStoneBaseChar* Actor)

// ALTE Slot-Lookup
GetSettlerSlotViewModelByIndex(int32 Index)
```

### Neue API (verwenden):

```cpp
// NEUE Delegate-Signatur (GUID-basiert)
FOnSettlerRequestShowDetails(FGuid SettlerGuid, AStoneBaseChar* Actor)

// NEUER Lookup
GetSlotViewModelBySettlerGuid(FGuid SettlerId)
```

**Blueprint Update:**
- `OnRequestShowDetails` Event neu binden (ändert Input-Pins!)
- SlotIndex-basierte Logik durch GUID-Lookups ersetzen

---

## Debug Helpers (Blueprint)

### Print Slot Mapping

```blueprint
[ForEachLoop: All Slot ViewModels]
  |
  +--> [Get Settler Guid As Guid]
  |      |
  |      +--> [Print String: "Slot {Index}: {GUID} - {SettlerName}"]
```

### Validate Widget-ViewModel Binding

```blueprint
[Event: OnConstruct WBP_SettlerSlot]
  |
  +--> [Branch: CachedSettlerGuid IsValid?]
         |
         +--> False: [Print Warning: "Widget has no GUID assigned!"]
```

---

## Best Practices

1. **Eindeutige Identifikation:**
   - Speichere `CachedSettlerGuid` im Widget (nicht SlotIndex)
   - Nutze GUID für alle Lookups

2. **Grid Rebuilds:**
   - Bei `OnSettlerListRebuilt`: Alte Widgets löschen, neue erstellen
   - `InitializeSlot()` setzt GUID + bindet ViewModel

3. **Event Handling:**
   - Binde `OnRequestShowDetails` EINMAL auf Screen-Level
   - Nicht pro Slot-Widget binden (Memory Leak!)

4. **Details Panel:**
   - C++ handled das Binding automatisch
   - Blueprint nur UI-Animation/Visibility

---

## FAQ

**Q: Muss ich SlotIndex noch irgendwo nutzen?**
A: Nur für UI-Position im Grid (Row/Column Berechnung). Für Daten-Lookups: GUID verwenden.

**Q: Was passiert wenn ein Settler stirbt?**
A: `RefreshSlotsFromRoster()` wird aufgerufen → Grid rebuilt → Slot wird entfernt.

**Q: Wie zeige ich Details eines bestimmten Settlers?**
A: `GetSlotViewModelBySettlerGuid(Guid)` → `RequestShowDetails()` auf dem Slot aufrufen.

**Q: Kann ich mehrere Details-Panels haben?**
A: Aktuell 1 DetailViewModel. Für mehrere: Erweitere `MVVM_SettlerScreen::DetailViewModels` zu TArray.

---

## Implementation Checklist

- [x] C++ GUID-Mapping implementiert
- [x] `GetSlotViewModelBySettlerGuid()` Blueprint-exposed
- [x] `GetSettlerGuidAsGuid()` Blueprint-exposed
- [x] Delegate-Signatur auf GUID umgestellt
- [ ] Blueprint: WBP_SettlerSlot `CachedSettlerGuid` Variable hinzufügen
- [ ] Blueprint: `InitializeSlot()` Function implementieren
- [ ] Blueprint: `OnSettlerListRebuilt` Grid-Rebuild implementieren
- [ ] Blueprint: `OnRequestShowDetails` Event neu binden (GUID statt SlotIndex)
- [ ] Blueprint: Alte SlotIndex-basierte Logik entfernen

---

**Status:** C++ Implementation fertig ✅  
**Next:** Blueprint Integration durchführen
