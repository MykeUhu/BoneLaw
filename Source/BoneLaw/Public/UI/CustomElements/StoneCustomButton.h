// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "StoneUIThemeDataAsset.h"
#include "UI/Widget/StoneUserWidget.h"
#include "StoneCustomButton.generated.h"


class UButton;
class UTextBlock;

/**
 * 
 */
UCLASS()
class BONELAW_API UStoneCustomButton : public UStoneUserWidget
{
	GENERATED_BODY()
public:	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStoneButtonClicked);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Button|Style", meta=(ExposeOnSpawn=true))
	EStoneButtonStyle ButtonStyle = EStoneButtonStyle::Primary;
	
	UPROPERTY(BlueprintAssignable, Category="Button|Event")
	FOnStoneButtonClicked OnClicked;

	UFUNCTION(BlueprintCallable, Category="Button")
	void SetButtonText(const FText& InText);

	// For "segmented/radio" look (Technique tabs)
	UFUNCTION(BlueprintCallable, Category="Button")
	void SetIsSelected(bool bInIsSelected);

	UFUNCTION(BlueprintPure, Category="Button")
	bool IsSelected() const { return bIsSelected; }

	UFUNCTION(BlueprintCallable, Category="Button|Style")
	void SetFontSize(float InFontSize);

	UFUNCTION(BlueprintCallable, Category="Button|Style")
	void SetTextJustification(TEnumAsByte<ETextJustify::Type> InJustification);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Button|Style", meta=(ClampMin="8.0", ClampMax="72.0"))
	float OverrideFontSize = 0.f; // 0 means use theme default

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Button|Style")
	TEnumAsByte<ETextJustify::Type> TextJustification = ETextJustify::Center;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void SynchronizeProperties() override;

	// Theme hook from UStoneUserWidget
	virtual void ThemeSet_Implementation() override;

	UFUNCTION()
	void HandleClicked();

	void ApplyTheme();

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UButton> Button_Main;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Label;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Button")
	FText ButtonText;

private:
	UPROPERTY()
	bool bIsSelected = false;
};
