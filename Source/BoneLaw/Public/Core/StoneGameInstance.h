// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "StoneGameInstance.generated.h"

class UStoneUIThemeDataAsset;
/**
 * 
 */
UCLASS()
class BONELAW_API UStoneGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	// ============================================================
	// UI Theme (set in editor, NOT hardcoded)
	// ============================================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI Theme")
	TObjectPtr<UStoneUIThemeDataAsset> DefaultUITheme = nullptr;

	UFUNCTION(BlueprintPure, Category="UI Theme")
	UStoneUIThemeDataAsset* GetUITheme() const { return DefaultUITheme; }

	virtual void Init() override;
	
	UPROPERTY()
	FName PlayerStartTag = FName();

	UPROPERTY()
	FString LoadSlotName = FString();

	UPROPERTY()
	int32 LoadSlotIndex = 0;
	
private:
	FString GetRunSlotName() const { return TEXT("StoneRun_Current"); }
	int32 GetRunUserIndex() const { return 0; }
};
