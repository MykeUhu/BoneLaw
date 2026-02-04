#include "UI/CustomElements/StoneCustomButton.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Styling/SlateTypes.h"

void UStoneCustomButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	ApplyTheme();

	if (TextBlock_Label)
	{
		TextBlock_Label->SetText(ButtonText);
		TextBlock_Label->SetJustification(TextJustification);
	}
}

void UStoneCustomButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Main)
	{
		Button_Main->OnClicked.RemoveDynamic(this, &UStoneCustomButton::HandleClicked);
		Button_Main->OnClicked.AddDynamic(this, &UStoneCustomButton::HandleClicked);
	}
}

void UStoneCustomButton::NativeDestruct()
{
	if (Button_Main)
	{
		Button_Main->OnClicked.RemoveDynamic(this, &UStoneCustomButton::HandleClicked);
	}

	Super::NativeDestruct();
}

void UStoneCustomButton::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (TextBlock_Label)
	{
		TextBlock_Label->SetText(ButtonText);
		TextBlock_Label->SetJustification(TextJustification);
	}

	ApplyTheme();
}

void UStoneCustomButton::ThemeSet_Implementation()
{
	Super::ThemeSet_Implementation();
	ApplyTheme();
}

void UStoneCustomButton::SetButtonText(const FText& InText)
{
	ButtonText = InText;

	if (TextBlock_Label)
	{
		TextBlock_Label->SetText(ButtonText);
	}
}

void UStoneCustomButton::SetIsSelected(bool bInIsSelected)
{
	bIsSelected = bInIsSelected;
	ApplyTheme();
}

void UStoneCustomButton::SetFontSize(float InFontSize)
{
	OverrideFontSize = InFontSize;
	ApplyTheme();
}

void UStoneCustomButton::SetTextJustification(TEnumAsByte<ETextJustify::Type> InJustification)
{
	TextJustification = InJustification;

	if (TextBlock_Label)
	{
		TextBlock_Label->SetJustification(TextJustification);
	}
}

void UStoneCustomButton::HandleClicked()
{
	OnClicked.Broadcast();
}

void UStoneCustomButton::ApplyTheme()
{
	if (!Theme || !Button_Main) return;

	const FStoneButtonTheme& BtnTheme = Theme->GetButtonTheme(ButtonStyle);
	FButtonStyle FinalStyle = BtnTheme.Style;

	// Selected button: Tab buttons behave like "pressed/selected"
	if (bIsSelected)
	{
		FinalStyle.SetNormal(FinalStyle.Pressed);
		FinalStyle.SetNormalForeground(FinalStyle.PressedForeground);
	}

	Button_Main->SetStyle(FinalStyle);

	if (TextBlock_Label)
	{
		FSlateFontInfo FontInfo = BtnTheme.Font;
		if (OverrideFontSize > 0.f)
		{
			FontInfo.Size = OverrideFontSize;
		}

		TextBlock_Label->SetFont(FontInfo);
		TextBlock_Label->SetJustification(TextJustification);
	}
}
