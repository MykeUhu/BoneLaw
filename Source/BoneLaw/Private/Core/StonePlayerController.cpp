#include "Core/StonePlayerController.h"

// Project
#include "Core/StoneGameplayTags.h"
#include "Core/StoneGameMode.h"
#include "Runtime/StoneRunSubsystem.h"
#include "Runtime/StoneActionSubsystem.h"
#include "AbilitySystem/StoneAbilitySystemComponent.h"
#include "Data/StoneActionDefinitionData.h"

// Engine
#include "GameFramework/Pawn.h"

// GAS / Engine helpers
#include "AbilitySystemBlueprintLibrary.h"

// Enhanced Input (Plugin)
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Core/StonePlayerState.h"
#include "UI/HUD/StoneHUD.h"

AStonePlayerController::AStonePlayerController()
{
	bReplicates = true;
	
	// Initialize to FreeCam mode
	CurrentCameraState = FStoneGameplayTags::Get().State_Camera_Free;
}

void AStonePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Default camera state is FreeCam
	SetCameraState(FStoneGameplayTags::Get().State_Camera_Free);

	// Start in FreeCam input mode (FPS-like mouse look)
	SetInputModeFreeCam();

	// Add IMC_Cam (always active in gameplay)
	if (IMC_Cam)
	{
		AddInputMappingContext(IMC_Cam, 0);
	}
	UE_LOG(LogTemp, Log, TEXT("[StonePC] PC=%s Pawn=%s"), *GetNameSafe(this), *GetNameSafe(GetPawn()));
	
	TryInitOverlay();
	
}

void AStonePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	UpdateCameraState(DeltaTime);
}

void AStonePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	// Bind Camera Input Actions
	if (IA_Move)
	{
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AStonePlayerController::Input_Move);
	}
	if (IA_Look)
	{
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AStonePlayerController::Input_Look);
	}
	if (IA_Ascend)
	{
		EnhancedInputComponent->BindAction(IA_Ascend, ETriggerEvent::Triggered, this, &AStonePlayerController::Input_Ascend);
	}
	if (IA_Descend)
	{
		EnhancedInputComponent->BindAction(IA_Descend, ETriggerEvent::Triggered, this, &AStonePlayerController::Input_Descend);
	}
	if (IA_ToggleBuildMenu)
	{
		EnhancedInputComponent->BindAction(IA_ToggleBuildMenu, ETriggerEvent::Started, this, &AStonePlayerController::Input_ToggleBuildMenu);
	}
	if (IA_Escape)
	{
		EnhancedInputComponent->BindAction(IA_Escape, ETriggerEvent::Started, this, &AStonePlayerController::Input_Escape);
	}

	// Build Input Actions (only active when IMC_Build is added)
	if (IA_RotateGhost)
	{
		EnhancedInputComponent->BindAction(IA_RotateGhost, ETriggerEvent::Triggered, this, &AStonePlayerController::Input_RotateGhost);
	}
	if (IA_AdjustGhostHeight)
	{
		EnhancedInputComponent->BindAction(IA_AdjustGhostHeight, ETriggerEvent::Triggered, this, &AStonePlayerController::Input_AdjustGhostHeight);
	}
	if (IA_Place)
	{
		EnhancedInputComponent->BindAction(IA_Place, ETriggerEvent::Started, this, &AStonePlayerController::Input_Place);
	}
	if (IA_Cancel)
	{
		EnhancedInputComponent->BindAction(IA_Cancel, ETriggerEvent::Started, this, &AStonePlayerController::Input_Cancel);
	}

	UE_LOG(LogTemp, Log, TEXT("[StonePC] Enhanced Input Setup Complete"));
}

void AStonePlayerController::StartStoneRun()
{
	UStoneRunSubsystem* RunSS = GetGameInstance() ? GetGameInstance()->GetSubsystem<UStoneRunSubsystem>() : nullptr;
	checkf(RunSS, TEXT("StoneRunSubsystem missing"));

	FStoneRunConfig Config;

	// SSOT: Prefer explicit PC override, otherwise take the GameMode default.
	FName StartPackToUse = DefaultStartPack;
	if (StartPackToUse.IsNone())
	{
		if (const AStoneGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AStoneGameMode>() : nullptr)
		{
			StartPackToUse = GM->DefaultStartPack;
		}
	}
	if (!StartPackToUse.IsNone())
	{
		Config.StartingPackIds.Add(StartPackToUse);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[StonePC] StartStoneRun: No start pack configured (PC.DefaultStartPack and GameMode.DefaultStartPack are None)."));
	}

	Config.bEnableAutoPackUnlocks = true;

	UE_LOG(LogTemp, Warning, TEXT("[StonePC] StartStoneRun: Pack=%s"), *StartPackToUse.ToString());
	RunSS->StartNewRun(Config);
}

void AStonePlayerController::SetSimSpeed(float NewSpeed)
{
	SimSpeed = FMath::Clamp(NewSpeed, 0.f, 10.f);

	if (UStoneRunSubsystem* RunSS = GetGameInstance() ? GetGameInstance()->GetSubsystem<UStoneRunSubsystem>() : nullptr)
	{
		RunSS->SetSimulationSpeed(SimSpeed);
	}
}

void AStonePlayerController::StartExploreExpedition(float DurationSeconds, float MinEventGapSeconds, float MaxEventGapSeconds, bool bTriggerFirstEventImmediately)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[StonePlayerController] StartExploreExpedition failed: World is null"));
		return;
	}

	UStoneActionSubsystem* ActionSS = World->GetSubsystem<UStoneActionSubsystem>();
	if (!ActionSS)
	{
		UE_LOG(LogTemp, Error, TEXT("[StonePlayerController] StartExploreExpedition failed: StoneActionSubsystem missing"));
		return;
	}

	// SSOT: pack id comes from PC defaults (can be overridden per BP child).
	const FName PackId = DefaultExplorePack;
	if (PackId.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[StonePlayerController] StartExploreExpedition failed: DefaultExplorePack is None"));
		return;
	}

	// Build a transient action definition (no hardcoded content paths; caller provides runtime parameters).
	UStoneActionDefinitionData* Def = NewObject<UStoneActionDefinitionData>(this);
	Def->ActionType = EStoneActionType::Explore;
	Def->DisplayName = FText::FromString(TEXT("Explore"));
	Def->BaseDurationSeconds = FMath::Max(1.f, DurationSeconds);
	Def->BaseDurationSeconds = FMath::Max(1.f, DurationSeconds);

	// Map (Min/Max gap) -> (event count + chance). Keeps BP callsites stable.
	const float MinGap = FMath::Max(0.1f, MinEventGapSeconds);
	const float MaxGap = FMath::Max(MinGap, MaxEventGapSeconds);
	const float AvgGap = 0.5f * (MinGap + MaxGap);
	const float ExpectedEvents = Def->BaseDurationSeconds / FMath::Max(0.1f, AvgGap);

	Def->PackIdsToActivate = { PackId };

	const bool bStarted = ActionSS->StartAction(Def);
	UE_LOG(LogTemp, Log, TEXT("[StonePlayerController] StartExploreExpedition -> %s (Pack=%s, Duration=%.1fs)"),
		bStarted ? TEXT("STARTED") : TEXT("FAILED"),
		*PackId.ToString(),
		Def->BaseDurationSeconds);
}

void AStonePlayerController::StopExpedition(bool bForceReturnEvent)
{
	if (UWorld* World = GetWorld())
	{
		if (UStoneActionSubsystem* ActionSS = World->GetSubsystem<UStoneActionSubsystem>())
		{
			ActionSS->StopCurrentAction(bForceReturnEvent);
			UE_LOG(LogTemp, Log, TEXT("[StonePlayerController] StopExpedition -> StopCurrentAction(ForceReturnHome=%s)"),
				bForceReturnEvent ? TEXT("true") : TEXT("false"));
			return;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("[StonePlayerController] StopExpedition: ActionSubsystem missing (nothing to stop)"));
}

UStoneAbilitySystemComponent* AStonePlayerController::GetASC()
{
	if (StoneAbilitySystemComponent == nullptr)
	{
		StoneAbilitySystemComponent = Cast<UStoneAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return StoneAbilitySystemComponent;
}

void AStonePlayerController::TryInitOverlay()
{
	if (bOverlayInitialized)
	{
		return;
	}

	if (!IsLocalController() || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	AStonePlayerState* StonePS = GetPlayerState<AStonePlayerState>();
	if (!StonePS)
	{
		return; // PS not ready yet
	}

	UAbilitySystemComponent* ASC = StonePS->GetAbilitySystemComponent();
	UAttributeSet* AS = StonePS->GetAttributeSet();
	if (!ASC || !AS)
	{
		return; // ASC/AS not ready yet
	}

	AStoneHUD* StoneHUD = GetHUD<AStoneHUD>();
	if (!StoneHUD)
	{
		return; // HUD not ready yet (rare but possible early)
	}

	StoneHUD->InitOverlay(this, StonePS, ASC, AS);
	bOverlayInitialized = true;
}

// ==========================================
// CAMERA STATE MANAGEMENT
// ==========================================

void AStonePlayerController::SetCameraState(FGameplayTag NewState)
{
	if (CurrentCameraState == NewState) return;

	UE_LOG(LogTemp, Log, TEXT("[StonePC] Camera State: %s -> %s"),
		*CurrentCameraState.ToString(), *NewState.ToString());

	CurrentCameraState = NewState;

	const FStoneGameplayTags& StoneTags = FStoneGameplayTags::Get();

	// Update input mode based on state
	if (NewState.MatchesTagExact(StoneTags.State_Camera_UI))
	{
		SetInputModeUI();
	}
	else if (NewState.MatchesTagExact(StoneTags.State_Camera_BuildPlacement))
	{
		SetInputModeBuildPlacement();
	}
	else // Free or Follow
	{
		SetInputModeFreeCam();
	}
}

void AStonePlayerController::UpdateCameraState(float DeltaTime)
{
	const FStoneGameplayTags& StoneTags = FStoneGameplayTags::Get();

	if (CurrentCameraState.MatchesTagExact(StoneTags.State_Camera_Follow))
	{
		UpdateFollowCamera(DeltaTime);
	}
	else if (CurrentCameraState.MatchesTagExact(StoneTags.State_Camera_BuildPlacement))
	{
		UpdateBuildGhost(DeltaTime);
	}
}

void AStonePlayerController::UpdateFollowCamera(float DeltaTime)
{
	if (!FollowTarget.IsValid())
	{
		// Target destroyed or null -> return to FreeCam
		UE_LOG(LogTemp, Warning, TEXT("[StonePC] Follow target lost, returning to FreeCam"));
		SetCameraState(FStoneGameplayTags::Get().State_Camera_Free);
		return;
	}

	// Simple follow: Position camera behind/above target
	// (This is a basic implementation - can be enhanced with spring arm, etc.)
	APawn* MyPawn = GetPawn();
	if (MyPawn && FollowTarget.IsValid())
	{
		FVector TargetLoc = FollowTarget->GetActorLocation();
		FVector Offset = FVector(-500.f, 0.f, 300.f); // Behind and above
		FVector DesiredLoc = TargetLoc + Offset;
		
		FVector CurrentLoc = MyPawn->GetActorLocation();
		FVector NewLoc = FMath::VInterpTo(CurrentLoc, DesiredLoc, DeltaTime, 5.f);
		MyPawn->SetActorLocation(NewLoc);

		// Look at target
		FRotator LookAtRot = (TargetLoc - NewLoc).Rotation();
		SetControlRotation(LookAtRot);
	}
}

void AStonePlayerController::UpdateBuildGhost(float DeltaTime)
{
	if (!BuildGhostActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StonePC] BuildGhost lost, canceling placement"));
		CancelBuildPlacement();
		return;
	}

	// Trace from center of screen to find placement location
	FVector HitLocation, HitNormal;
	if (TraceBuildLocation(HitLocation, HitNormal))
	{
		// Apply height offset
		FVector FinalLocation = HitLocation + FVector(0.f, 0.f, GhostHeightOffset);
		BuildGhostActor->SetActorLocation(FinalLocation);

		// Apply rotation
		FRotator GhostRot = FRotator(0.f, GhostRotationYaw, 0.f);
		BuildGhostActor->SetActorRotation(GhostRot);
	}
}

bool AStonePlayerController::TraceBuildLocation(FVector& OutLocation, FVector& OutNormal) const
{
	// Center-screen trace (crosshair point)
	int32 ViewportSizeX, ViewportSizeY;
	GetViewportSize(ViewportSizeX, ViewportSizeY);

	FVector2D ScreenCenter(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f);
	
	FVector WorldLocation, WorldDirection;
	if (!DeprojectScreenPositionToWorld(ScreenCenter.X, ScreenCenter.Y, WorldLocation, WorldDirection))
	{
		return false;
	}

	// Trace forward from camera
	FVector TraceEnd = WorldLocation + WorldDirection * 10000.f; // 100m
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetPawn());
	if (BuildGhostActor)
	{
		QueryParams.AddIgnoredActor(BuildGhostActor);
	}

	if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, TraceEnd, ECC_Visibility, QueryParams))
	{
		OutLocation = HitResult.Location;
		OutNormal = HitResult.Normal;
		return true;
	}

	return false;
}

// ==========================================
// BUILD PLACEMENT
// ==========================================

void AStonePlayerController::StartBuildPlacement(FGameplayTag BuildRecipeTag)
{
	UE_LOG(LogTemp, Log, TEXT("[StonePC] StartBuildPlacement: %s"), *BuildRecipeTag.ToString());

	// TODO: Spawn ghost actor based on BuildRecipeTag
	// For now, just log and set state
	
	// Reset accumulators
	GhostRotationYaw = 0.f;
	GhostHeightOffset = 0.f;

	// Add Build IMC
	if (IMC_Build)
	{
		AddInputMappingContext(IMC_Build, 1); // Higher priority than Cam
	}

	// Set state
	SetCameraState(FStoneGameplayTags::Get().State_Camera_BuildPlacement);

	UE_LOG(LogTemp, Warning, TEXT("[StonePC] TODO: Spawn ghost actor for recipe: %s"), *BuildRecipeTag.ToString());
}

void AStonePlayerController::CancelBuildPlacement()
{
	UE_LOG(LogTemp, Log, TEXT("[StonePC] CancelBuildPlacement"));

	// Destroy ghost
	if (BuildGhostActor)
	{
		BuildGhostActor->Destroy();
		BuildGhostActor = nullptr;
	}

	// Remove Build IMC
	if (IMC_Build)
	{
		RemoveInputMappingContext(IMC_Build);
	}

	// Return to FreeCam
	SetCameraState(FStoneGameplayTags::Get().State_Camera_Free);
}

void AStonePlayerController::PlaceBuilding()
{
	UE_LOG(LogTemp, Log, TEXT("[StonePC] PlaceBuilding"));

	if (!BuildGhostActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StonePC] PlaceBuilding: No ghost actor"));
		return;
	}

	// TODO: Convert ghost to real building, consume resources, etc.
	FVector Location = BuildGhostActor->GetActorLocation();
	FRotator Rotation = BuildGhostActor->GetActorRotation();

	UE_LOG(LogTemp, Warning, TEXT("[StonePC] TODO: Place building at %s, rotation %s"),
		*Location.ToString(), *Rotation.ToString());

	// For now, just cancel placement
	CancelBuildPlacement();
}

// ==========================================
// FOLLOW / OBSERVE
// ==========================================

void AStonePlayerController::FollowSettler(AActor* Settler)
{
	if (!Settler)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StonePC] FollowSettler: Settler is null"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[StonePC] FollowSettler: %s"), *Settler->GetName());

	FollowTarget = Settler;
	SetCameraState(FStoneGameplayTags::Get().State_Camera_Follow);
}

void AStonePlayerController::StopFollowing()
{
	UE_LOG(LogTemp, Log, TEXT("[StonePC] StopFollowing"));

	FollowTarget.Reset();
	SetCameraState(FStoneGameplayTags::Get().State_Camera_Free);
}

// ==========================================
// RADIAL MENU (Blueprint Events)
// ==========================================

void AStonePlayerController::OnRadialRecipeSelected(FGameplayTag RecipeTag)
{
	UE_LOG(LogTemp, Log, TEXT("[StonePC] OnRadialRecipeSelected: %s"), *RecipeTag.ToString());

	// Close menu (Blueprint will handle UI)
	CloseRadialBuildMenu();

	// Start build placement
	StartBuildPlacement(RecipeTag);
}

// ==========================================
// INPUT MODE MANAGEMENT
// ==========================================

void AStonePlayerController::SetInputModeFreeCam()
{
	UE_LOG(LogTemp, Log, TEXT("[StonePC] SetInputModeFreeCam"));

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	bShowMouseCursor = false;
	SetIgnoreLookInput(false); // Enable mouse look
}

void AStonePlayerController::SetInputModeUI()
{
	UE_LOG(LogTemp, Log, TEXT("[StonePC] SetInputModeUI"));

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	bShowMouseCursor = true;
	SetIgnoreLookInput(true); // Disable mouse look while UI is open
}

void AStonePlayerController::SetInputModeBuildPlacement()
{
	UE_LOG(LogTemp, Log, TEXT("[StonePC] SetInputModeBuildPlacement"));

	// Same as FreeCam (FPS-like mouse look, cursor hidden)
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	bShowMouseCursor = false;
	SetIgnoreLookInput(false);
}

void AStonePlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	TryInitOverlay();
}

// ==========================================
// INPUT MAPPING CONTEXT MANAGEMENT
// ==========================================

void AStonePlayerController::AddInputMappingContext(UInputMappingContext* IMC, int32 Priority)
{
	if (!IMC) return;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(IMC, Priority);
		UE_LOG(LogTemp, Log, TEXT("[StonePC] Added IMC: %s (Priority: %d)"), *IMC->GetName(), Priority);
	}
}

void AStonePlayerController::RemoveInputMappingContext(UInputMappingContext* IMC)
{
	if (!IMC) return;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->RemoveMappingContext(IMC);
		UE_LOG(LogTemp, Log, TEXT("[StonePC] Removed IMC: %s"), *IMC->GetName());
	}
}

// ==========================================
// INPUT HANDLERS
// ==========================================

void AStonePlayerController::Input_Move(const FInputActionValue& InputActionValue)
{
	const FVector2D MovementVector = InputActionValue.Get<FVector2D>();
	if (APawn* ControlledPawn = GetPawn())
	{
		const FRotator ControlRot = GetControlRotation(); // FULL (Yaw+Pitch)
		const FVector Forward = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::X);
		const FVector Right   = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::Y);

		ControlledPawn->AddMovementInput(Forward, MovementVector.Y);
		ControlledPawn->AddMovementInput(Right,   MovementVector.X);
	}
}


void AStonePlayerController::Input_Look(const FInputActionValue& InputActionValue)
{
	const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();

	// Add yaw/pitch to controller rotation (FPS-like)
	AddYawInput(LookAxisVector.X);
	AddPitchInput(LookAxisVector.Y);
}

void AStonePlayerController::Input_Ascend(const FInputActionValue& InputActionValue)
{
	const float Value = InputActionValue.Get<float>();

	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->AddMovementInput(FVector::UpVector, Value);
	}
}

void AStonePlayerController::Input_Descend(const FInputActionValue& InputActionValue)
{
	const float Value = InputActionValue.Get<float>();

	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->AddMovementInput(FVector::DownVector, Value);
	}
}

void AStonePlayerController::Input_ToggleBuildMenu()
{
	UE_LOG(LogTemp, Log, TEXT("[StonePC] Input_ToggleBuildMenu"));

	const FStoneGameplayTags& StoneTags = FStoneGameplayTags::Get();

	if (CurrentCameraState.MatchesTagExact(StoneTags.State_Camera_UI))
	{
		// Close menu
		CloseRadialBuildMenu();
		SetCameraState(StoneTags.State_Camera_Free);
	}
	else if (CurrentCameraState.MatchesTagExact(StoneTags.State_Camera_Free) || 
	         CurrentCameraState.MatchesTagExact(StoneTags.State_Camera_Follow))
	{
		// Open menu
		OpenRadialBuildMenu();
		SetCameraState(StoneTags.State_Camera_UI);
	}
}

void AStonePlayerController::Input_RotateGhost(const FInputActionValue& InputActionValue)
{
	const float Value = InputActionValue.Get<float>();
	
	// Accumulate yaw rotation (continuous)
	GhostRotationYaw += Value * 2.f; // Sensitivity multiplier

	// Wrap to 0-360
	while (GhostRotationYaw >= 360.f) GhostRotationYaw -= 360.f;
	while (GhostRotationYaw < 0.f) GhostRotationYaw += 360.f;
}

void AStonePlayerController::Input_AdjustGhostHeight(const FInputActionValue& InputActionValue)
{
	const float Value = InputActionValue.Get<float>();

	// Accumulate height offset (Shift + MouseWheel)
	GhostHeightOffset += Value * 10.f; // 10 units per scroll tick

	// Clamp to reasonable range
	GhostHeightOffset = FMath::Clamp(GhostHeightOffset, -500.f, 500.f);
}

void AStonePlayerController::Input_Place()
{
	UE_LOG(LogTemp, Log, TEXT("[StonePC] Input_Place"));

	PlaceBuilding();
}

void AStonePlayerController::Input_Cancel()
{
	UE_LOG(LogTemp, Log, TEXT("[StonePC] Input_Cancel"));

	// Cancel build placement (RMB or Esc in build mode)
	CancelBuildPlacement();
}

void AStonePlayerController::Input_Escape()
{
	UE_LOG(LogTemp, Log, TEXT("[StonePC] Input_Escape"));

	const FStoneGameplayTags& StoneTags = FStoneGameplayTags::Get();

	// Priority: Cancel build > Close UI > Exit Follow
	if (CurrentCameraState.MatchesTagExact(StoneTags.State_Camera_BuildPlacement))
	{
		CancelBuildPlacement();
	}
	else if (CurrentCameraState.MatchesTagExact(StoneTags.State_Camera_UI))
	{
		CloseRadialBuildMenu();
		SetCameraState(StoneTags.State_Camera_Free);
	}
	else if (CurrentCameraState.MatchesTagExact(StoneTags.State_Camera_Follow))
	{
		StopFollowing();
	}
	// If already in FreeCam, do nothing (or could open pause menu)
}
