// Copyright by MykeUhu

// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "StonePlayerState.generated.h"

class UAttributeSet;
class UStoneAbilitySystemComponent;
class UStoneAttributeSet;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChanged, int32 /*StatValue*/)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLevelChanged, int32 /*StatValue*/, bool /*bLevelUp*/)

/**
 * AStonePlayerState - GAS Owner for BoneLaw
 * 
 * This is the authoritative owner of the AbilitySystemComponent and AttributeSet.
 * All GAS operations go through this class.
 * 
 * Blueprint Usage:
 *   - Get via PlayerController->GetPlayerState<AStonePlayerState>()
 *   - Access ASC: GetAbilitySystemComponent()
 *   - Access Attributes: GetStoneAttributeSet()
 */
UCLASS()
class BONELAW_API AStonePlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AStonePlayerState();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// === IAbilitySystemInterface ===
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	// === Typed Accessors (for C++ and Blueprint) ===
	UFUNCTION(BlueprintPure, Category="Stone|GAS")
	UStoneAbilitySystemComponent* GetStoneAbilitySystemComponent() const;
	
	UFUNCTION(BlueprintPure, Category="Stone|GAS")
	UStoneAttributeSet* GetStoneAttributeSet() const;
	
	// Legacy accessor (returns base type)
	UAttributeSet* GetAttributeSet() const;
	
	// === Level System ===
	FOnLevelChanged OnLevelChangedDelegate;
	
	FORCEINLINE int32 GetPlayerLevel() const { return Level; }
	void AddToLevel(int32 InLevel);
	void SetLevel(int32 InLevel);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stone|GAS")
	TObjectPtr<UStoneAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stone|GAS")
	TObjectPtr<UStoneAttributeSet> AttributeSet;
	
private:
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_Level)
	int32 Level = 1;
	
	UFUNCTION()
	void OnRep_Level(int32 OldLevel);
};
