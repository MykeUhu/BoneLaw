// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "Data/StoneCharacterClassInfo.h"
#include "UObject/Interface.h"
#include "PlayerInterface.generated.h"

class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnASCRegistered, UAbilitySystemComponent*)

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

class BONELAW_API IPlayerInterface
{
	GENERATED_BODY()
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
	
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	AActor* GetAvatar();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	EStoneCharacterClass  GetCharacterClass();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SaveProgress(const FName& CheckpointTag);
	
	virtual FOnASCRegistered& GetOnASCRegisteredDelegate() = 0;
};
