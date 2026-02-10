# FreeCam/Commander Implementation - Phase 1

## Status: ✅ Grundstruktur komplett

### Implementierte Features

#### 1. Camera State System (GameplayTag-based)
- **State.Camera.Free** - FreeCam mode (fliegender Spectator, default)
- **State.Camera.Follow** - Folgt einem ausgewählten Settler
- **State.Camera.BuildPlacement** - Build Ghost Platzierung aktiv
- **State.Camera.UI** - UI Interaktion (Radial Menu offen)

#### 2. StoneCameraPawn
- Erbt von `ASpectatorPawn`
- Fliegende Kamera mit FPS-Style Mouse Look
- Konfigurierbare Geschwindigkeiten:
  - BaseMoveSpeed (WASD Bewegung)
  - VerticalSpeed (Q/E auf/ab)
  - LookSensitivity (Maus-Look)

#### 3. Enhanced Input System
- **Input Mapping Contexts:**
  - `IMC_Cam` - Immer aktiv für Kamera-Steuerung
  - `IMC_Build` - Nur aktiv während Build Placement

- **Input Actions:**
  - `IA_Move` (Axis2D) - WASD Bewegung
  - `IA_Look` (Axis2D) - Mouse Look
  - `IA_Ascend` (Axis1D) - E (hoch)
  - `IA_Descend` (Axis1D) - Q (runter)
  - `IA_ToggleBuildMenu` (Bool) - Tab
  - `IA_RotateGhost` (Axis1D) - MouseWheel
  - `IA_AdjustGhostHeight` (Axis1D) - Shift+MouseWheel
  - `IA_Place` (Bool) - LMB
  - `IA_Cancel` (Bool) - RMB
  - `IA_Escape` (Bool) - Esc

#### 4. PlayerController Erweiterungen
- Enhanced Input Setup
- Camera State Management
- Input Mode Switching:
  - FreeCam: Mouse captured, Cursor hidden, FPS-Look enabled
  - UI: Cursor visible, UI aktiv, FPS-Look disabled
  - BuildPlacement: Wie FreeCam
  
- Follow System (Basic Implementation)
- Build Placement System (Grundstruktur)
- Center-Screen Trace für Ghost Placement

#### 5. StoneInputConfig DataAsset
- Struktur für Input Action + GameplayTag Bindings
- Ermöglicht DataAsset-basierte Input-Konfiguration

### Dateien hinzugefügt/geändert

**Neue Dateien:**
- `BoneLaw/Public/Core/Character/StoneCameraPawn.h`
- `BoneLaw/Private/Core/Character/StoneCameraPawn.cpp`
- `BoneLaw/Public/Input/StoneInputConfig.h`
- `BoneLaw/Private/Input/StoneInputConfig.cpp`

**Geänderte Dateien:**
- `BoneLaw/Public/Core/StoneGameplayTags.h` (Camera State Tags hinzugefügt)
- `BoneLaw/Private/Core/StoneGameplayTags.cpp` (Tags registriert)
- `BoneLaw/Public/Core/StonePlayerController.h` (Enhanced Input + Camera Management)
- `BoneLaw/Private/Core/StonePlayerController.cpp` (Vollständige Implementierung)
- `BoneLaw/BoneLaw.Build.cs` (EnhancedInput Dependency hinzugefügt)

## Nächste Schritte (Phase 2)

### Blueprint Assets erstellen
Die folgenden Assets müssen in Unreal Editor erstellt werden:

1. **Input Mapping Contexts** (als DataAssets)
   - `IMC_Cam` - Mapping für WASD, Mouse, Q/E, Tab, Esc
   - `IMC_Build` - Mapping für MouseWheel, Shift+MouseWheel, LMB, RMB

2. **Input Actions** (als DataAssets)
   - Alle IA_* Actions aus PlayerController header

3. **Input Config DataAsset**
   - `DA_StoneInputConfig` (UStoneInputConfig)
   - Optional: Tagged inputs für Abilities

4. **GameMode Konfiguration**
   - DefaultPawnClass auf `BP_StoneCameraPawn` (Blueprint basierend auf AStoneCameraPawn)
   - PlayerControllerClass bleibt `AStonePlayerController`

5. **Blueprint Radial Menu Integration**
   - `OpenRadialBuildMenu` Event in PlayerController Blueprint implementieren
   - `CloseRadialBuildMenu` Event implementieren
   - Event `OnRadialRecipeSelected` aufrufen wenn Rezept gewählt

### Ghost Placement System
- Ghost Actor spawnen/zerstören
- Visual Feedback (valid/invalid placement)
- Collision Detection
- Building Rezepte DataAsset

### Follow Camera Verbesserungen
- Spring Arm Component für smoothere Kamera
- Zoom in/out
- Orbit um Settler

### Crosshair/HUD
- Minimaler Center-Dot für Platzierung
- Ghost validity Feedback
- Build Kosten anzeigen

## Testing Checklist

- [ ] GameMode spawnt StoneCameraPawn
- [ ] WASD Bewegung funktioniert
- [ ] Mouse Look funktioniert (ohne RMB)
- [ ] Q/E für Auf/Ab funktioniert
- [ ] Tab öffnet Radial Menu (Cursor erscheint, Mouse Look aus)
- [ ] Esc schließt Menu
- [ ] Nach Build-Auswahl: IMC_Build aktiv, Ghost folgt center-screen trace
- [ ] MouseWheel rotiert Ghost
- [ ] Shift+MouseWheel verschiebt Ghost hoch/runter
- [ ] LMB platziert (oder Fehler wenn ungültig)
- [ ] RMB/Esc bricht Placement ab
- [ ] Nach Cancel: Zurück zu FreeCam, IMC_Build entfernt

## Known Limitations / TODOs

- Ghost Actor spawning noch nicht implementiert (Placeholder)
- Follow Camera ist basic (keine Spring Arm)
- Kein Visual Feedback für valid/invalid placement
- Kein Crosshair Widget
- Keine Building Kosten/Validierung
- Keine Settler AI Integration
- GameMode DefaultPawn muss in Blueprint gesetzt werden

## Architektur Notizen

### SSOT Prinzip
- Camera State: GameplayTag (nicht Enum)
- Input: Enhanced Input System (keine Legacy Input)
- Alle Input Bindings in PlayerController (nicht in Pawn)

### Input Mode States
- **GameOnly**: Cursor hidden, Mouse captured, FPS-Look → FreeCam & BuildPlacement
- **GameAndUI**: Cursor visible, UI active, FPS-Look disabled → UI Mode

### IMC Priority
- IMC_Cam: Priority 0 (base layer, immer aktiv)
- IMC_Build: Priority 1 (override, nur bei BuildPlacement)

### Camera Mode Transitions
```
Free ←→ UI (Tab)
Free → Follow (Click Settler)
Follow → Free (Esc)
Free/Follow → BuildPlacement (Select Recipe)
BuildPlacement → Free (Place/Cancel)
```
