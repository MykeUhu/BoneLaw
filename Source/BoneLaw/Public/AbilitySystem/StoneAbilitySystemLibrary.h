// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StoneAbilitySystemLibrary.generated.h"

class UAbilitySystemComponent;
class UStoneSaveGame;
class UAbilityInfo;
class ULoadScreenSaveGame;

class AStoneHUD;
class UStoneOverlayWidgetController;
struct FStoneWidgetControllerParams;

/**
 * BoneLaw GAS helper library.
 * - Fetch overlay widget controller via HUD
 * - Initialize default attributes (no Diablo-level dependencies)
 * - Optional: access AbilityInfo asset
 */
UCLASS()
class BONELAW_API UStoneAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// -----------------------------
	// Widget Controller
	// -----------------------------

	UFUNCTION(BlueprintPure, Category="Stone|AbilitySystem|WidgetController", meta=(DefaultToSelf="WorldContextObject"))
	static bool MakeWidgetControllerParams(const UObject* WorldContextObject, FStoneWidgetControllerParams& OutParams, AStoneHUD*& OutStoneHUD);

	UFUNCTION(BlueprintPure, Category="Stone|AbilitySystem|WidgetController", meta=(DefaultToSelf="WorldContextObject"))
	static UStoneOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	// -----------------------------
	// Defaults / Save init
	// -----------------------------

	UFUNCTION(BlueprintCallable, Category="Stone|AbilitySystem|Defaults", meta=(DefaultToSelf="WorldContextObject"))
	static void InitializeDefaultAttributesFromSaveData(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, UStoneSaveGame* SaveGame);

	UFUNCTION(BlueprintCallable, Category="Stone|AbilitySystem|Defaults", meta=(DefaultToSelf="WorldContextObject"))
	static UAbilityInfo* GetAbilityInfo(const UObject* WorldContextObject);
};
