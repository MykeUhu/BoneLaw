#include "UI/CustomElements/StoneCustomTextBlock.h"

#include "Components/TextBlock.h"
#include "UI/CustomElements/StoneUIThemeDataAsset.h"

void UStoneCustomTextBlock::SetText(const FText& InText)
{
	Text = InText;

	if (TextBlock_Main)
	{
		TextBlock_Main->SetText(Text);
	}
}

void UStoneCustomTextBlock::SetFontSize(float InFontSize)
{
	OverrideFontSize = InFontSize;
	ApplyTheme();
}

void UStoneCustomTextBlock::SetTextJustification(TEnumAsByte<ETextJustify::Type> InJustification)
{
	TextJustification = InJustification;

	if (TextBlock_Main)
	{
		TextBlock_Main->SetJustification(TextJustification);
	}
}

void UStoneCustomTextBlock::NativePreConstruct()
{
	Super::NativePreConstruct();

	ApplyTheme();

	if (TextBlock_Main)
	{
		TextBlock_Main->SetText(Text);
		TextBlock_Main->SetJustification(TextJustification);
	}
}

void UStoneCustomTextBlock::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	ApplyTheme();

	if (TextBlock_Main)
	{
		TextBlock_Main->SetText(Text);
		TextBlock_Main->SetJustification(TextJustification);
	}
}

void UStoneCustomTextBlock::ThemeSet_Implementation()
{
	Super::ThemeSet_Implementation();
	ApplyTheme();
}

void UStoneCustomTextBlock::ApplyTheme()
{
	if (!Theme || !TextBlock_Main) return;

	const FStoneTextBlockTheme& T = Theme->GetTextTheme(TextStyle);

	FSlateFontInfo FontInfo = Theme->GetFont(FontSize);
	if (OverrideFontSize > 0.f)
	{
		FontInfo.Size = OverrideFontSize;
	}

	TextBlock_Main->SetFont(FontInfo);
	TextBlock_Main->SetColorAndOpacity(FSlateColor(T.Color));
	TextBlock_Main->SetShadowOffset(T.ShadowOffset);
	TextBlock_Main->SetShadowColorAndOpacity(T.ShadowColor);
	TextBlock_Main->SetJustification(TextJustification);
}
