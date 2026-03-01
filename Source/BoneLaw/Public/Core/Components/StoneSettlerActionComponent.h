#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Runtime/StoneActionRuntime.h"
#include "Runtime/StoneActionTypes.h"
// Defines FStoneChoiceResolved used by encounter UI APIs
#include "Game/Events/StoneEventResolver.h"
#include "StoneSettlerActionComponent.generated.h"

class UStoneActionDefinitionData;
class UAbilitySystemComponent;
class UStoneEventData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStoneSettlerActionStateChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStoneSettlerActionProgressChanged, float, Progress01);

DECLARE_MULTICAST_DELEGATE_TwoParams(FStoneSettlerActionFinishedNative, const UStoneActionDefinitionData* /*Action*/, bool /*bSuccess*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FStoneSettlerActionFinished, const UStoneActionDefinitionData*, Action, bool, bSuccess);

/**
 * Fired when an encounter (event) is opened for this settler.
 * MVVM screens bind here to drive popups.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStoneSettlerEncounterOpened, const UStoneEventData*, Event);

/**
 * Fired when the currently open encounter for this settler is closed.
 * bAborted is true only when the encounter ended due to Action.Abort outcome tag.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStoneSettlerEncounterClosed, bool, bAborted);

/**
 * Per-Settler Action Component
 * Each settler can have their own active action independently.
 * SSOT for BT state is GAS tags applied on the Settler ASC (not RunSubsystem).
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BONELAW_API UStoneSettlerActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStoneSettlerActionComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Start a new action for this settler. */
	UFUNCTION(BlueprintCallable, Category = "Stone|Action")
	bool StartAction(UStoneActionDefinitionData* ActionDef);

	/** Stops the current action and reports success/failure. (Preferred API) */
	UFUNCTION(BlueprintCallable, Category = "Stone|Action")
	void StopAction(bool bSuccess = true);

	/** Stop the currently running action (legacy). */
	UFUNCTION(BlueprintCallable, Category = "Stone|Action")
	void StopCurrentAction(bool bForceReturnHomeEvent);

	/** Is this settler currently executing an action? */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	bool IsActionRunning() const { return bActionRunning; }

	/** Current phase of the action */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	EStoneActionPhase GetPhase() const { return Phase; }

	/** Overall action progress (0.0 to 1.0) */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	float GetActionProgress01() const;

	/** Current phase progress (0.0 to 1.0) */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	float GetPhaseProgress01() const;

	/** Display title of current action */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	FText GetActionTitleText() const;

	/** Display description of current action */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	FText GetActionDescriptionText() const;

	/** Current phase as text */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	FText GetPhaseText() const;

	/** Remaining time in seconds */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	float GetRemainingSeconds() const;

	/** Get this settler's ASC for attribute lookups */
	UAbilitySystemComponent* GetASC() const;

	/** Get elapsed time in current phase (for save/load) */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	float GetPhaseElapsed() const { return PhaseElapsedBaseSeconds; }

	/** Get total elapsed time (for save/load) */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	float GetTotalElapsed() const { return TotalElapsedBaseSeconds; }

	/** Get current action definition (for save/load) */
	UFUNCTION(BlueprintPure, Category = "Stone|Action")
	UStoneActionDefinitionData* GetCurrentDef() const { return CurrentDef; }

	/** Set phase directly (for save/load resume) */
	UFUNCTION(BlueprintCallable, Category = "Stone|Action")
	void SetPhase(EStoneActionPhase NewPhase) { Phase = NewPhase; }

	/** Set elapsed times directly (for save/load resume) */
	UFUNCTION(BlueprintCallable, Category = "Stone|Action")
	void SetElapsedTimes(float PhaseElapsed, float TotalElapsed)
	{
		PhaseElapsedBaseSeconds = PhaseElapsed;
		TotalElapsedBaseSeconds = TotalElapsed;
	}

	/**
	 * Resolve / close the current encounter. Call from MVVM/UI once the player made a choice.
	 * bAborted should be false in normal UI resolution.
	 */
	UFUNCTION(BlueprintCallable, Category="Stone|Action|Encounter")
	void ResolveCurrentEncounter(bool bAborted);

	/** True while an encounter UI is open for this settler. */
	UFUNCTION(BlueprintPure, Category="Stone|Action|Encounter")
	bool IsEncounterOpen() const { return bEncounterOpen; }

	/** The currently open encounter event (may be null). */
	UFUNCTION(BlueprintPure, Category="Stone|Action|Encounter")
	const UStoneEventData* GetCurrentEncounterEvent() const { return CurrentEncounterEvent; }

	UPROPERTY(BlueprintAssignable, Category = "Stone|Action")
	FStoneSettlerActionStateChanged OnActionStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stone|Action")
	FStoneSettlerActionProgressChanged OnActionProgressChanged;

	/** Fired whenever the action ends (success/fail). */
	UPROPERTY(BlueprintAssignable, Category = "Stone|Action")
	FStoneSettlerActionFinished OnActionFinished;

	/** Native version used by BT tasks (avoids dynamic bind overhead). */
	FStoneSettlerActionFinishedNative OnActionFinishedNative;

	/** Encounter opened for this settler. */
	UPROPERTY(BlueprintAssignable, Category = "Stone|Action|Encounter")
	FStoneSettlerEncounterOpened OnEncounterOpened;

	/** Encounter closed for this settler. */
	UPROPERTY(BlueprintAssignable, Category = "Stone|Action|Encounter")
	FStoneSettlerEncounterClosed OnEncounterClosed;
	
	UPROPERTY(Transient)
	TObjectPtr<UStoneActionRuntime> ActionRuntime;
	
	UPROPERTY(Transient)
	TObjectPtr<UStoneEventData> CurrentEncounterEvent;
	
	/** Returns resolved choices for the currently open encounter (for UI). */
	UFUNCTION(BlueprintCallable, Category="Stone|Action|Encounter")
	void GetCurrentEncounterChoices(TArray<FStoneChoiceResolved>& OutResolved) const;

	/** Applies a choice to the currently open encounter (OutcomeExecutor etc.) */
	UFUNCTION(BlueprintCallable, Category="Stone|Action|Encounter")
	bool ApplyEncounterChoice(int32 ChoiceIndex);

private:
	void StopInternal(bool bSuccess, bool bForceReturnHomeEvent);

	void TickAction();
	void AdvancePhaseTimeline(float AdvanceBaseSeconds);
	void HandlePhaseAdvance();
	void EnterPhase(EStoneActionPhase NewPhase);

	float ResolveActionSpeedMult() const;
	FGameplayTag GetRandomActionTag(EStoneActionPhase InPhase) const;

	/**
	 * Checks if the settler ASC carries an Action.Abort or Action.ReturnImmediately tag.
	 * If found, consumes the tag (removes it) and returns the appropriate result.
	 */
	EStoneActionAbortResult CheckAndConsumeAbortTags();

	/** Open an encounter (event) by tag and broadcast OnEncounterOpened. */
	void OpenEncounterByTag(FGameplayTag EventTag);

	/** Notify listeners that the encounter was closed. */
	void NotifyEncounterClosed(bool bAborted);

private:
	FTimerHandle ActionTickHandle;

	UPROPERTY()
	TObjectPtr<UStoneActionDefinitionData> CurrentDef;

	bool bActionRunning = false;
	bool bReturnHomeQueued = false;
	
	// Optional: Action-scoped tags applied to the Settler ASC for the lifetime of the action.
	// Use only if you still rely on GrantedStateTags in data assets.
	FGameplayTagContainer AppliedStateTags;

	/** True while an encounter is currently open (event unresolved). */
	bool bEncounterOpen = false;

	/** Tag of the currently open encounter (valid only while bEncounterOpen is true). */
	FGameplayTag CurrentEncounterTag;

	/** Set when an encounter/action is aborted via Action.Abort outcome tag. */
	bool bLastEncounterAborted = false;

	
	EStoneActionPhase Phase = EStoneActionPhase::None;

	float BaseDurationSeconds = 0.f;
	float OutboundSeconds = 0.f;
	float ArrivalSeconds = 0.f;
	float ReturnSeconds = 0.f;
	float PhaseElapsedBaseSeconds = 0.f;
	float TotalElapsedBaseSeconds = 0.f;

	TArray<FStonePlannedEncounter> OutboundEncounterSlots;
	TArray<FStonePlannedEncounter> ReturnEncounterSlots;

	FRandomStream RNG;
};