// Copyright (c) 2025 MykeUhu. All rights reserved.

#include "UI/CustomElements/StoneUIThemeDataAsset.h"

#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/TextBlock.h"
#include "UObject/ConstructorHelpers.h"

static FLinearColor Hex(const TCHAR* InHex)
{
	return FColor::FromHex(InHex).ReinterpretAsLinear();
}

static FSlateBrush MakeBoxBrush(const FLinearColor& C, float Alpha = 1.f)
{
	FSlateBrush B;
	B.DrawAs = ESlateBrushDrawType::Box;
	FLinearColor CC = C; CC.A = Alpha;
	B.TintColor = FSlateColor(CC);
	B.Margin = FMargin(0.f);
	return B;
}

UStoneUIThemeDataAsset::UStoneUIThemeDataAsset()
{
	// ========= GLOBAL FONT =========
	{
		static ConstructorHelpers::FObjectFinder<UObject> ZenDotsFont(TEXT("/Game/Blueprints/Import/Font/ZenDots-Regular_Font"));
		if (ZenDotsFont.Succeeded())
		{
			GlobalFont = ZenDotsFont.Object;
		}
	}

	// ========= MASTER PALETTE =========
	UI_Background   = Hex(TEXT("05070A"));
	UI_Panel        = Hex(TEXT("0D1117"));
	UI_PanelHover   = Hex(TEXT("161B22"));
	UI_PanelRaised  = Hex(TEXT("21262D"));
	UI_BorderLight  = Hex(TEXT("30363D"));
	UI_BorderStrong = Hex(TEXT("484F58"));

	TextPrimary     = Hex(TEXT("F0F6FC"));
	TextSecondary   = Hex(TEXT("8B949E"));
	TextDisabled    = Hex(TEXT("6E7681"));

	AccentLime      = Hex(TEXT("D4FF33"));
	AccentMint      = Hex(TEXT("00FFC8"));
	AccentBlue      = Hex(TEXT("58A6FF"));

	SuccessGreen    = Hex(TEXT("2EA043"));
	WarningOrange   = Hex(TEXT("FF8A00"));
	DangerRed       = Hex(TEXT("FF4D4D"));

	// ========= BASE FONTS =========
	const FSlateFontInfo FontSmallInfo = GetFont(EStoneUIFontSize::Small);
	const FSlateFontInfo FontBaseInfo  = GetFont(EStoneUIFontSize::Base);
	const FSlateFontInfo FontTitleInfo = GetFont(EStoneUIFontSize::Title);

	// ========= BRUSHES =========
	const FSlateBrush BrushPanel       = MakeBoxBrush(UI_Panel,       1.f);
	const FSlateBrush BrushPanelHover  = MakeBoxBrush(UI_PanelHover,  1.f);
	const FSlateBrush BrushRaised      = MakeBoxBrush(UI_PanelRaised, 1.f);
	const FSlateBrush BrushBorderLight = MakeBoxBrush(UI_BorderLight, 1.f);
	const FSlateBrush BrushAccentLime  = MakeBoxBrush(AccentLime,     1.f);
	const FSlateBrush BrushDanger      = MakeBoxBrush(DangerRed,      1.f);

	// ========= BUTTON THEMES =========
	// Primary: understated, hover bright edge, pressed flashes lime
	{
		FButtonStyle S = FButtonStyle()
			.SetNormal(BrushRaised)
			.SetHovered(BrushPanelHover)
			.SetPressed(BrushAccentLime)
			.SetDisabled(BrushPanel)
			.SetNormalPadding(FMargin(14.f, 8.f))
			.SetPressedPadding(FMargin(14.f, 10.f, 14.f, 6.f))
			.SetNormalForeground(FSlateColor(TextSecondary))
			.SetHoveredForeground(FSlateColor(TextPrimary))
			.SetPressedForeground(FSlateColor(UI_Background))
			.SetDisabledForeground(FSlateColor(TextDisabled));

		ButtonPrimary.Style = S;
		ButtonPrimary.Font  = FontBaseInfo;
	}

	// Secondary: flatter, less accent
	{
		FButtonStyle S = FButtonStyle()
			.SetNormal(BrushPanel)
			.SetHovered(BrushRaised)
			.SetPressed(BrushPanelHover)
			.SetDisabled(BrushPanel)
			.SetNormalPadding(FMargin(12.f, 7.f))
			.SetPressedPadding(FMargin(12.f, 9.f, 12.f, 5.f))
			.SetNormalForeground(FSlateColor(TextSecondary))
			.SetHoveredForeground(FSlateColor(TextPrimary))
			.SetPressedForeground(FSlateColor(TextPrimary))
			.SetDisabledForeground(FSlateColor(TextDisabled));

		ButtonSecondary.Style = S;
		ButtonSecondary.Font  = FontBaseInfo;
	}

	// Tab: transparent feel, only text + subtle hover
	{
		FButtonStyle S = FButtonStyle()
			.SetNormal(MakeBoxBrush(UI_Background, 0.0f)) // invisible
			.SetHovered(MakeBoxBrush(UI_PanelHover, 0.6f))
			.SetPressed(MakeBoxBrush(AccentLime, 0.35f))
			.SetDisabled(MakeBoxBrush(UI_Background, 0.0f))
			.SetNormalPadding(FMargin(10.f, 6.f))
			.SetPressedPadding(FMargin(10.f, 6.f))
			.SetNormalForeground(FSlateColor(TextSecondary))
			.SetHoveredForeground(FSlateColor(TextPrimary))
			.SetPressedForeground(FSlateColor(AccentLime))
			.SetDisabledForeground(FSlateColor(TextDisabled));

		ButtonTab.Style = S;
		ButtonTab.Font  = FontSmallInfo;
	}

	// Danger: red confirm actions
	{
		FButtonStyle S = FButtonStyle()
			.SetNormal(MakeBoxBrush(DangerRed, 0.65f))
			.SetHovered(BrushDanger)
			.SetPressed(BrushDanger)
			.SetDisabled(BrushPanel)
			.SetNormalPadding(FMargin(14.f, 8.f))
			.SetPressedPadding(FMargin(14.f, 10.f, 14.f, 6.f))
			.SetNormalForeground(FSlateColor(TextPrimary))
			.SetHoveredForeground(FSlateColor(UI_Background))
			.SetPressedForeground(FSlateColor(UI_Background))
			.SetDisabledForeground(FSlateColor(TextDisabled));

		ButtonDanger.Style = S;
		ButtonDanger.Font  = FontBaseInfo;
	}

	// ========= SLIDER =========
	{
		FSlateBrush ThumbN = MakeBoxBrush(AccentMint, 0.9f);
		ThumbN.ImageSize = FVector2D(8.f, 18.f);

		FSlateBrush ThumbH = MakeBoxBrush(AccentLime, 1.f);
		ThumbH.ImageSize = FVector2D(10.f, 20.f);

		SliderTheme.Style = FSliderStyle()
			.SetNormalBarImage(MakeBoxBrush(UI_PanelRaised, 1.f))
			.SetHoveredBarImage(MakeBoxBrush(UI_PanelRaised, 1.f))
			.SetDisabledBarImage(MakeBoxBrush(UI_Panel, 1.f))
			.SetNormalThumbImage(ThumbN)
			.SetHoveredThumbImage(ThumbH)
			.SetDisabledThumbImage(MakeBoxBrush(TextDisabled, 0.35f))
			.SetBarThickness(3.f);
	}

	// ========= COMBO BOX (DARK, PROPER) =========
	{
		// closed field brush
		FSlateBrush FieldNormal  = MakeBoxBrush(UI_PanelRaised, 1.f);
		FSlateBrush FieldHover   = MakeBoxBrush(UI_PanelHover, 1.f);
		FSlateBrush FieldPressed = MakeBoxBrush(AccentLime, 0.25f);

		// popup background
		FSlateBrush MenuBg = MakeBoxBrush(UI_Panel, 1.f);

		FButtonStyle ComboBtn = FButtonStyle()
			.SetNormal(FieldNormal)
			.SetHovered(FieldHover)
			.SetPressed(FieldPressed)
			.SetDisabled(MakeBoxBrush(UI_Panel, 1.f))
			.SetNormalPadding(FMargin(12.f, 7.f))
			.SetPressedPadding(FMargin(12.f, 9.f, 12.f, 5.f))
			.SetNormalForeground(FSlateColor(TextPrimary))
			.SetHoveredForeground(FSlateColor(TextPrimary))
			.SetPressedForeground(FSlateColor(TextPrimary))
			.SetDisabledForeground(FSlateColor(TextDisabled));

		FComboButtonStyle CBS = FComboButtonStyle()
			.SetButtonStyle(ComboBtn)
			.SetDownArrowImage(BrushBorderLight)
			.SetMenuBorderBrush(MenuBg)
			.SetMenuBorderPadding(FMargin(2.f));

		ComboBoxTheme.Style = FComboBoxStyle()
			.SetComboButtonStyle(CBS);

		// row style inside dropdown
		FSlateBrush RowNormal   = MakeBoxBrush(UI_Panel, 1.f);
		FSlateBrush RowHovered  = MakeBoxBrush(UI_PanelHover, 1.f);
		FSlateBrush RowSelected = MakeBoxBrush(AccentLime, 0.20f);

		ComboBoxTheme.ItemStyle = FTableRowStyle()
			.SetTextColor(FSlateColor(TextPrimary))
			.SetSelectedTextColor(FSlateColor(AccentLime))
			.SetEvenRowBackgroundBrush(RowNormal)
			.SetOddRowBackgroundBrush(RowNormal)
			.SetEvenRowBackgroundHoveredBrush(RowHovered)
			.SetOddRowBackgroundHoveredBrush(RowHovered)
			.SetActiveBrush(RowSelected)
			.SetActiveHoveredBrush(MakeBoxBrush(AccentLime, 0.30f))
			.SetSelectorFocusedBrush(MakeBoxBrush(AccentMint, 0.25f));

		ComboBoxTheme.Font = FontSmallInfo;
	}

	// ========= CHECK BOX (LED LOOK) =========
	{
		const FVector2D Size(18.f, 18.f);
		FSlateBrush Off = MakeBoxBrush(UI_PanelRaised, 1.f); Off.ImageSize = Size;
		FSlateBrush On  = MakeBoxBrush(AccentLime, 1.f);     On.ImageSize  = Size;

		CheckBoxTheme.Style = FCheckBoxStyle()
			.SetUncheckedImage(Off)
			.SetUncheckedHoveredImage(MakeBoxBrush(UI_PanelHover, 1.f))
			.SetUncheckedPressedImage(MakeBoxBrush(UI_PanelHover, 1.f))
			.SetCheckedImage(On)
			.SetCheckedHoveredImage(MakeBoxBrush(AccentLime, 1.f))
			.SetCheckedPressedImage(MakeBoxBrush(AccentLime, 1.f))
			.SetForegroundColor(FSlateColor(TextPrimary));
	}

	// ========= EDITABLE TEXT BOX (INSET FIELD) =========
	{
		EditableTextBoxTheme.Style = FEditableTextBoxStyle()
			.SetBackgroundImageNormal(MakeBoxBrush(UI_Panel, 1.f))
			.SetBackgroundImageHovered(MakeBoxBrush(UI_PanelHover, 1.f))
			.SetBackgroundImageFocused(MakeBoxBrush(UI_PanelRaised, 1.f))
			.SetBackgroundImageReadOnly(MakeBoxBrush(UI_Panel, 0.65f))
			.SetForegroundColor(FSlateColor(TextPrimary))
			.SetReadOnlyForegroundColor(FSlateColor(TextSecondary));

		EditableTextBoxTheme.Font = FontSmallInfo;
		EditableTextBoxTheme.Style.TextStyle.Font = EditableTextBoxTheme.Font;
		EditableTextBoxTheme.Style.TextStyle.ColorAndOpacity = FSlateColor(TextPrimary);
	}

	// ========= SCROLL BAR (UE5.7 MINIMAL) =========
	{
		FScrollBarStyle S;

		// Thumb (the draggable bar)
		S.SetNormalThumbImage(MakeBoxBrush(UI_BorderStrong, 0.65f));
		S.SetHoveredThumbImage(MakeBoxBrush(AccentMint, 0.75f));
		S.SetDraggedThumbImage(MakeBoxBrush(AccentMint, 0.95f));

		// Background/Track (in UE5.7: use Horizontal/Vertical background images)
		S.SetHorizontalBackgroundImage(MakeBoxBrush(UI_Background, 0.0f)); // invisible
		S.SetVerticalBackgroundImage(MakeBoxBrush(UI_Background, 0.0f));   // invisible

		// Thickness exists in UE5.7 (if it doesn't compile for you, remove this line
		// and set thickness on the ScrollBar widget instead)
		S.SetThickness(4.f);

		ScrollBarTheme.Style = S;
	}

	// ========= LIST ROW (ListView Entry Background) =========
	{
		FSlateBrush RowNormal   = MakeBoxBrush(UI_Panel, 1.f);
		FSlateBrush RowHover    = MakeBoxBrush(UI_PanelHover, 1.f);
		FSlateBrush RowSelected = MakeBoxBrush(AccentLime, 0.18f);

		ListRowTheme.Style = FTableRowStyle()
			.SetTextColor(FSlateColor(TextPrimary))
			.SetSelectedTextColor(FSlateColor(AccentLime))
			.SetEvenRowBackgroundBrush(RowNormal)
			.SetOddRowBackgroundBrush(RowNormal)
			.SetEvenRowBackgroundHoveredBrush(RowHover)
			.SetOddRowBackgroundHoveredBrush(RowHover)
			.SetActiveBrush(RowSelected)
			.SetActiveHoveredBrush(MakeBoxBrush(AccentLime, 0.28f))
			.SetSelectorFocusedBrush(MakeBoxBrush(AccentMint, 0.22f));
	}

	// ========= TEXT THEMES =========
	TextBody.Font = FontBaseInfo;
	TextBody.Color = TextPrimary;

	TextTitle.Font = FontTitleInfo;
	TextTitle.Color = TextPrimary;

	TextMuted.Font = FontBaseInfo;
	TextMuted.Color = TextSecondary;

	TextAccent.Font = FontBaseInfo;
	TextAccent.Color = AccentLime;

	TextDanger.Font = FontBaseInfo;
	TextDanger.Color = DangerRed;
}

FSlateFontInfo UStoneUIThemeDataAsset::GetFont(EStoneUIFontSize Size) const
{
	float Px = FontBase;
	switch (Size)
	{
		case EStoneUIFontSize::Small: Px = FontSmall; break;
		case EStoneUIFontSize::Base:  Px = FontBase;  break;
		case EStoneUIFontSize::Large: Px = FontLarge; break;
		case EStoneUIFontSize::Title: Px = FontTitle; break;
	}
	Px *= FMath::Max(0.5f, FontScale);
	return FSlateFontInfo(GlobalFont, FMath::RoundToInt(Px));
}

const FStoneButtonTheme& UStoneUIThemeDataAsset::GetButtonTheme(EStoneButtonStyle Style) const
{
	switch (Style)
	{
		case EStoneButtonStyle::Primary:   return ButtonPrimary;
		case EStoneButtonStyle::Secondary: return ButtonSecondary;
		case EStoneButtonStyle::Tab:       return ButtonTab;
		case EStoneButtonStyle::Danger:    return ButtonDanger;
		default:                          return ButtonPrimary;
	}
}

const FStoneTextBlockTheme& UStoneUIThemeDataAsset::GetTextTheme(EStoneTextStyle Style) const
{
	switch (Style)
	{
		case EStoneTextStyle::Body:   return TextBody;
		case EStoneTextStyle::Title:  return TextTitle;
		case EStoneTextStyle::Muted:  return TextMuted;
		case EStoneTextStyle::Accent: return TextAccent;
		case EStoneTextStyle::Danger: return TextDanger;
		default:                     return TextBody;
	}
}

void UStoneUIThemeDataAsset::ApplyTextStyle(UTextBlock* Text, EStoneTextStyle Style, EStoneUIFontSize Size) const
{
	if (!Text) return;

	Text->SetFont(GetFont(Size));

	const FStoneTextBlockTheme& T = GetTextTheme(Style);
	Text->SetColorAndOpacity(FSlateColor(T.Color));
	Text->SetShadowOffset(T.ShadowOffset);
	Text->SetShadowColorAndOpacity(T.ShadowColor);
}

void UStoneUIThemeDataAsset::ApplyComboBoxStyle(UComboBoxString* ComboBox) const
{
	if (!ComboBox) return;

	ComboBox->SetWidgetStyle(ComboBoxTheme.Style);
	ComboBox->SetItemStyle(ComboBoxTheme.ItemStyle);
}

void UStoneUIThemeDataAsset::ApplyCheckBoxStyle(UCheckBox* CheckBox) const
{
	if (!CheckBox) return;

	CheckBox->SetWidgetStyle(CheckBoxTheme.Style);
}
