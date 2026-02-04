// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "StoneUIThemeDataAsset.h"
#include "UI/Widget/StoneUserWidget.h"
#include "StoneCustomTextBlock.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class BONELAW_API UStoneCustomTextBlock : public UStoneUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Text|Style", meta=(ExposeOnSpawn=true))
	EStoneTextStyle TextStyle = EStoneTextStyle::Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Text|Style")
	EStoneUIFontSize FontSize = EStoneUIFontSize::Base;

	UFUNCTION(BlueprintCallable, Category="Text")
	void SetText(const FText& InText);

	UFUNCTION(BlueprintCallable, Category="Text|Style")
	void SetFontSize(float InFontSize);

	UFUNCTION(BlueprintCallable, Category="Text|Style")
	void SetTextJustification(TEnumAsByte<ETextJustify::Type> InJustification);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Text|Style", meta=(ClampMin="8.0", ClampMax="72.0"))
	float OverrideFontSize = 0.f; // 0 means use theme default

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Text|Style")
	TEnumAsByte<ETextJustify::Type> TextJustification = ETextJustify::Left;

protected:
	virtual void NativePreConstruct() override;
	virtual void SynchronizeProperties() override;

	// Theme hook
	virtual void ThemeSet_Implementation() override;

	void ApplyTheme();

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Main;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Text")
	FText Text;
};
