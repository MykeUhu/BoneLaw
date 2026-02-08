// Copyright by MykeUhu
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Core/Interaction/PlayerInterface.h"
#include "Data/StoneCharacterClassInfo.h"
#include "GameFramework/Character.h"
#include "StoneBaseChar.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;
class UGameplayAbility;

class UStoneAbilitySystemComponent;
class UStoneAttributeSet;

// Aura pattern: Delegate fired when ASC is registered on this character
DECLARE_MULTICAST_DELEGATE_OneParam(FOnASCRegistered, UAbilitySystemComponent*);

UCLASS(Abstract)
class BONELAW_API AStoneBaseChar : public ACharacter, public IAbilitySystemInterface, public IPlayerInterface
{
	GENERATED_BODY()

public:
	AStoneBaseChar();
	
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	// Interface
	virtual AActor* GetAvatar_Implementation() override;
	virtual EStoneCharacterClass GetCharacterClass_Implementation() override;
	virtual FOnASCRegistered& GetOnASCRegisteredDelegate() override;
	// end Interface
	
	FOnASCRegistered OnAscRegistered;
	
	void SetCharacterClass(EStoneCharacterClass InClass) { CharacterClass = InClass; }

protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	virtual void InitAbilityActorInfo();

	virtual void BeginPlay() override;

	// -------- Default Attribute GEs (asset-driven, Aura style) --------
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS|Defaults")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS|Defaults")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS|Defaults")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS|Defaults")
	TSubclassOf<UGameplayEffect> DefaultCultureAttributes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS|Defaults")
	TSubclassOf<UGameplayEffect> DefaultKnowledgeAttributes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS|Defaults")
	TSubclassOf<UGameplayEffect> DefaultWorldlineAttributes;

	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level = 1.f) const;
	virtual void InitializeDefaultAttributes() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Class Defaults")
	EStoneCharacterClass CharacterClass = EStoneCharacterClass::Scout;
};
