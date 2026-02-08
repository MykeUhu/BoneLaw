// Copyright by MykeUhu
#pragma once

#include "CoreMinimal.h"
#include "Core/Character/StoneBaseChar.h"
#include "StonePlayerChar.generated.h"

class AStonePlayerState;
class UStoneAbilitySystemComponent;
class UStoneAttributeSet;

UCLASS()
class BONELAW_API AStonePlayerChar : public AStoneBaseChar
{
	GENERATED_BODY()

public:
	AStonePlayerChar();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	
	// Interface
	virtual AActor* GetAvatar_Implementation() override;
	virtual EStoneCharacterClass GetCharacterClass_Implementation() override;
	virtual FOnASCRegistered& GetOnASCRegisteredDelegate() override;
	
	virtual void SaveProgress_Implementation(const FName& CheckpointTag) override;
	// end Interface
	
private:
	void LoadProgress();
	virtual void InitAbilityActorInfo() override;
};
