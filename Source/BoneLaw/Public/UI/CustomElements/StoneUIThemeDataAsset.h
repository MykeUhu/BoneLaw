// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "StoneUIThemeDataAsset.generated.h"

class UObject;
class UTextBlock;
class UComboBoxString;
class UCheckBox;

// ================================
// Theme Types
// ================================

UENUM(BlueprintType)
enum class EStoneTextStyle : uint8
{
	Body        UMETA(DisplayName="Body"),
	Title       UMETA(DisplayName="Title"),
	Muted       UMETA(DisplayName="Muted"),
	Accent      UMETA(DisplayName="Accent"),
	Danger      UMETA(DisplayName="Danger"),
};

UENUM(BlueprintType)
enum class EStoneButtonStyle : uint8
{
	Primary     UMETA(DisplayName="Primary"),
	Secondary   UMETA(DisplayName="Secondary"),
	Tab         UMETA(DisplayName="Tab"),
	Danger      UMETA(DisplayName="Danger"),
};

// ================================
// Struct Themes
// ================================

USTRUCT(BlueprintType)
struct FStoneButtonTheme
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Style")
	FButtonStyle Style;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Style")
	FSlateFontInfo Font;
};

USTRUCT(BlueprintType)
struct FStoneSliderTheme
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Style")
	FSliderStyle Style;
};

USTRUCT(BlueprintType)
struct FStoneComboBoxTheme
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Style")
	FComboBoxStyle Style;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Style")
	FTableRowStyle ItemStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Style")
	FSlateFontInfo Font;
};

USTRUCT(BlueprintType)
struct FStoneCheckBoxTheme
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Style")
	FCheckBoxStyle Style;
};

USTRUCT(BlueprintType)
struct FStoneEditableTextBoxTheme
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Style")
	FEditableTextBoxStyle Style;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Style")
	FSlateFontInfo Font;
};

USTRUCT(BlueprintType)
struct FStoneTextBlockTheme
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Style")
	FSlateFontInfo Font;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Style")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Style")
	FVector2D ShadowOffset = FVector2D(2.f, 2.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Style")
	FLinearColor ShadowColor = FLinearColor(0.f, 0.f, 0.f, 0.5f);
};

USTRUCT(BlueprintType)
struct FStoneScrollBarTheme
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Style")
	FScrollBarStyle Style;
};

USTRUCT(BlueprintType)
struct FStoneTableRowTheme
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Style")
	FTableRowStyle Style;
};

// ================================
// Font Size Enum
// ================================
UENUM(BlueprintType)
enum class EStoneUIFontSize : uint8
{
	Small  UMETA(DisplayName="Small"),
	Base   UMETA(DisplayName="Base"),
	Large  UMETA(DisplayName="Large"),
	Title  UMETA(DisplayName="Title"),
};

// ================================
// Theme Data Asset
// ================================

UCLASS(BlueprintType, Category="UI|Theme")
class BONELAW_API UStoneUIThemeDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

	public:
	UStoneUIThemeDataAsset();

	// ========= GLOBAL FONT =========
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Font")
	TObjectPtr<UObject> GlobalFont = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Font")
	float FontSmall = 14.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Font")
	float FontBase  = 16.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Font")
	float FontLarge = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Font")
	float FontTitle = 26.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Font")
	float FontScale = 1.f;

	// ========= COLORS (MASTER PALETTE) =========
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Colors")
	FLinearColor UI_Background;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Colors")
	FLinearColor UI_Panel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Colors")
	FLinearColor UI_PanelHover;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Colors")
	FLinearColor UI_PanelRaised;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Colors")
	FLinearColor UI_BorderLight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Colors")
	FLinearColor UI_BorderStrong;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Colors")
	FLinearColor TextPrimary;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Colors")
	FLinearColor TextSecondary;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Colors")
	FLinearColor TextDisabled;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Colors")
	FLinearColor AccentLime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Colors")
	FLinearColor AccentMint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Colors")
	FLinearColor AccentBlue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Colors")
	FLinearColor SuccessGreen;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Colors")
	FLinearColor WarningOrange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Global Colors")
	FLinearColor DangerRed;

	// ========= WIDGET STYLES =========
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Widget Styles")
	FStoneButtonTheme ButtonPrimary;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Widget Styles")
	FStoneButtonTheme ButtonSecondary;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Widget Styles")
	FStoneButtonTheme ButtonTab;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Widget Styles")
	FStoneButtonTheme ButtonDanger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Widget Styles")
	FStoneSliderTheme SliderTheme;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Widget Styles")
	FStoneComboBoxTheme ComboBoxTheme;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Widget Styles")
	FStoneCheckBoxTheme CheckBoxTheme;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Widget Styles")
	FStoneEditableTextBoxTheme EditableTextBoxTheme;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Widget Styles")
	FStoneScrollBarTheme ScrollBarTheme;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Widget Styles")
	FStoneTableRowTheme ListRowTheme;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Widget Styles")
	FStoneTextBlockTheme TextBody;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Widget Styles")
	FStoneTextBlockTheme TextTitle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Widget Styles")
	FStoneTextBlockTheme TextMuted;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Widget Styles")
	FStoneTextBlockTheme TextAccent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Widget Styles")
	FStoneTextBlockTheme TextDanger;

	// ========= HELPERS =========
	UFUNCTION(BlueprintPure, Category="UI|Theme")
	FSlateFontInfo GetFont(EStoneUIFontSize Size) const;

	UFUNCTION(BlueprintPure, Category="UI|Theme")
	const FStoneButtonTheme& GetButtonTheme(EStoneButtonStyle Style) const;

	UFUNCTION(BlueprintPure, Category="UI|Theme")
	const FStoneTextBlockTheme& GetTextTheme(EStoneTextStyle Style) const;

	UFUNCTION(BlueprintCallable, Category="UI|Theme")
	void ApplyTextStyle(UTextBlock* Text, EStoneTextStyle Style, EStoneUIFontSize Size) const;

	UFUNCTION(BlueprintCallable, Category="UI|Theme")
	void ApplyComboBoxStyle(UComboBoxString* ComboBox) const;

	UFUNCTION(BlueprintCallable, Category="UI|Theme")
	void ApplyCheckBoxStyle(UCheckBox* CheckBox) const;
};
