#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StoneUserWidget.generated.h"

class UStoneUIThemeDataAsset;
class UStoneWidgetController;

UCLASS()
class BONELAW_API UStoneUserWidget : public UUserWidget
{
	GENERATED_BODY()

	// ============================================================
	// Controller
	// ============================================================
public:
	UFUNCTION(BlueprintCallable, Category="Stone|UI")
	void SetWidgetController(UObject* InWidgetController);
	
	UPROPERTY(BlueprintReadOnly, Category="Stone|UI")
	TObjectPtr<UObject> WidgetController;
	
protected:
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerSet();
	
	// ============================================================
	// Theme (single source of truth for all custom widgets)
	// ============================================================
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stone|UI|Theme", meta=(ExposeOnSpawn=true))
	TObjectPtr<UStoneUIThemeDataAsset> Theme;
	
	UFUNCTION(BlueprintCallable, Category="Stone|UI|Theme")
	void SetTheme(UStoneUIThemeDataAsset* InTheme);
	
	UFUNCTION(BlueprintPure, Category="Stone|UI|Theme")
	UStoneUIThemeDataAsset* GetTheme() const { return Theme; }
	
protected:
	void ResolveThemeIfMissing();
	
	/** C++/BP hook – called AFTER Theme is assigned (safe to call from PreConstruct/SynchronizeProperties) */
	UFUNCTION(BlueprintNativeEvent, Category="Stone|UI|Theme")
	void ThemeSet();
	virtual void ThemeSet_Implementation();

	// ============================================================
	// Debug Content Validation
	// ============================================================
public:
	UFUNCTION(BlueprintImplementableEvent, Category="Stone|Dev")
	void BP_RunDevContentValidation();
	
protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	
	/** Optional BP hook */
	UFUNCTION(BlueprintImplementableEvent, Category="Stone|UI")
	void BP_OnWidgetControllerSet();
};
