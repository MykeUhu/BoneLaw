// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "Core/Character/StoneBaseChar.h"
#include "Core/Components/StoneSettlerActionComponent.h"
#include "Data/StoneTypes.h"
#include "GameplayEffectTypes.h"
#include "StoneSettlerChar.generated.h"

class AStoneAIController;
class UBehaviorTree;
class UStoneAbilitySystemComponent;
class UStoneAttributeSet;

/**
 * Settler character using Pawn-owned ASC (AuraEnemy style).
 * Player remains PlayerState-ASC. This class is only for AI/Settlers.
 *
 * RESPONSIBILITY:
 * - Owns and initializes its ASC (Owner=this, Avatar=this)
 * - Applies default attributes + startup abilities (authority only)
 * - Optional: applies saved state (tags/attributes/abilities) when provided
 */
UCLASS()
class BONELAW_API AStoneSettlerChar : public AStoneBaseChar
{
	GENERATED_BODY()

public:
	AStoneSettlerChar();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;

	/** Apply save-driven state to this pawn (data-only). Safe to call multiple times. */
	UFUNCTION(BlueprintCallable, Category="Stone|Roster")
	void ApplySavedState(const FSavedSettler& SettlerData);
	
	UFUNCTION(BlueprintPure, Category="Stone|Action")
	UStoneSettlerActionComponent* GetActionComponent() const { return ActionComponent; }
	
	// -------------------------
	// GAS State via GameplayEffects (SSOT: SettlerChar)
	// -------------------------

	UFUNCTION(BlueprintCallable, Category="Stone|GAS|State")
	void ClearStateEffects();

	UFUNCTION(BlueprintCallable, Category="Stone|GAS|State")
	void SetState_Idle();
	
	UFUNCTION(BlueprintCallable, Category="Stone|GAS|State")
	void SetState_TravelToActionStart();

	UFUNCTION(BlueprintCallable, Category="Stone|GAS|State")
	void SetState_ActionRunning();

	UFUNCTION(BlueprintCallable, Category="Stone|GAS|State")
	void SetState_TravelReturning();

protected:
	virtual void BeginPlay() override;

	/** AStoneBaseChar contract */
	virtual void InitAbilityActorInfo() override;
	virtual void InitializeDefaultAttributes() const override;

	/** Startup abilities (authority only) */
	void GiveStartupAbilities() const;

	/** Ensures we only do default init once. */
	bool bDidDefaultInit = false;

	/** Ensures we only apply saved state once per spawn unless explicitly called again. */
	bool bDidApplySavedState = false;

	/** Optional: Level used for default-attribute init */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stone|GAS")
	int32 Level = 1;

	// -------------------------
	// AI
	// -------------------------
	UPROPERTY(EditAnywhere, Category="AI")
	TObjectPtr<UBehaviorTree> BehaviorTree = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AStoneAIController> StoneAIController = nullptr;
	
	// -------------------------
	// Actions (per-settler)
	// -------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stone|Action", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStoneSettlerActionComponent> ActionComponent = nullptr;
	
	// --- State GameplayEffects (set in BP defaults of the Settler BP) ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stone|GAS|State", meta=(AllowPrivateAccess="true"))
	TSubclassOf<UGameplayEffect> GE_State_Idle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stone|GAS|State", meta=(AllowPrivateAccess="true"))
	TSubclassOf<UGameplayEffect> GE_State_TravelToActionStart;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stone|GAS|State", meta=(AllowPrivateAccess="true"))
	TSubclassOf<UGameplayEffect> GE_State_ActionRunning;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stone|GAS|State", meta=(AllowPrivateAccess="true"))
	TSubclassOf<UGameplayEffect> GE_State_TravelReturning;

	// --- Active handles (runtime) ---
	UPROPERTY(Transient)
	FActiveGameplayEffectHandle Handle_State_Idle;

	UPROPERTY(Transient)
	FActiveGameplayEffectHandle Handle_State_TravelToActionStart;

	UPROPERTY(Transient)
	FActiveGameplayEffectHandle Handle_State_ActionRunning;

	UPROPERTY(Transient)
	FActiveGameplayEffectHandle Handle_State_TravelReturning;

	// Internal helpers
	FActiveGameplayEffectHandle ApplyStateEffect(TSubclassOf<UGameplayEffect> EffectClass, float EffectLevel = 1.f);
	void RemoveStateEffect(FActiveGameplayEffectHandle& Handle);
	
};
