#include "UI/CustomElements/StoneCustomPanel.h"

#include "Components/Border.h"
#include "UI/CustomElements/StoneUIThemeDataAsset.h"

static FSlateBrush MakeSolidBrush(const FLinearColor& Color)
{
	FSlateBrush B;
	B.DrawAs = ESlateBrushDrawType::Box;
	B.TintColor = FSlateColor(Color);
	B.Margin = FMargin(0.f);
	return B;
}

void UStoneCustomPanel::NativePreConstruct()
{
	Super::NativePreConstruct();
	ApplyTheme();
}

void UStoneCustomPanel::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	ApplyTheme();
}

void UStoneCustomPanel::ThemeSet_Implementation()
{
	Super::ThemeSet_Implementation();
	ApplyTheme();
}

void UStoneCustomPanel::SetSelected(bool bInSelected)
{
	bSelected = bInSelected;
	ApplyTheme();
}

void UStoneCustomPanel::SetHovered(bool bInHovered)
{
	bHovered = bInHovered;
	ApplyTheme();
}

void UStoneCustomPanel::ApplyTheme()
{
	if (!Theme) return;
	if (!Border_Background) return;

	FLinearColor Bg = Theme->UI_Panel;
	FLinearColor Line = Theme->UI_BorderLight;

	switch (PanelStyle)
	{
	case EStonePanelStyle::RootBackground: Bg = Theme->UI_Background;  break;
	case EStonePanelStyle::Panel:          Bg = Theme->UI_Panel;       break;
	case EStonePanelStyle::SubPanel:       Bg = Theme->UI_PanelHover;  break;
	case EStonePanelStyle::Raised:         Bg = Theme->UI_PanelRaised; break;
	case EStonePanelStyle::TopBar:         Bg = Theme->UI_Panel;       break;
	case EStonePanelStyle::SlotTile:       Bg = Theme->UI_PanelHover;  break;
	default:                              Bg = Theme->UI_Panel;       break;
	}

	// Hover: stronger border + "lift" if already hover base
	if (bHovered && !bSelected)
	{
		const bool bBaseIsHover = (Bg == Theme->UI_PanelHover);
		Bg = bBaseIsHover ? Theme->UI_PanelRaised : Theme->UI_PanelHover;
		Line = Theme->UI_BorderStrong;
	}

	// Selected accent
	if (bShowLaserLine && bSelected && bLaserLineAccentWhenSelected)
	{
		Line = Theme->AccentMint;
	}
	else if (bSelected)
	{
		Line = Theme->UI_BorderStrong;
		Bg = Theme->UI_PanelRaised;
	}

	Border_Background->SetBrush(MakeSolidBrush(Bg));

	if (Border_TopLine)
	{
		Border_TopLine->SetVisibility(bShowLaserLine ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		Border_TopLine->SetBrush(MakeSolidBrush(Line));
	}

	if (Border_LeftLine)
	{
		Border_LeftLine->SetVisibility(bShowLaserLine ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		Border_LeftLine->SetBrush(MakeSolidBrush(Line));
	}
}
