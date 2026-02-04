// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/StoneUserWidget.h"
#include "StoneCustomPanel.generated.h"

class UBorder;
class UNamedSlot;

UENUM(BlueprintType)
enum class EStonePanelStyle : uint8
{
	RootBackground UMETA(DisplayName="RootBackground"),
	Panel          UMETA(DisplayName="Panel"),
	SubPanel       UMETA(DisplayName="SubPanel"),
	Raised         UMETA(DisplayName="Raised"),
	TopBar         UMETA(DisplayName="TopBar"),
	SlotTile       UMETA(DisplayName="SlotTile"),
};

/**
 * 
 */
UCLASS()
class BONELAW_API UStoneCustomPanel : public UStoneUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Panel|Style", meta=(ExposeOnSpawn=true))
	EStonePanelStyle PanelStyle = EStonePanelStyle::Panel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Panel|Style")
	bool bShowLaserLine = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Panel|Style")
	bool bLaserLineAccentWhenSelected = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Panel|Style")
	bool bSelected = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Panel|Style")
	bool bHovered = false;

	UFUNCTION(BlueprintCallable, Category="Panel")
	void SetSelected(bool bInSelected);

	UFUNCTION(BlueprintCallable, Category="Panel")
	void SetHovered(bool bInHovered);

protected:
	virtual void NativePreConstruct() override;
	virtual void SynchronizeProperties() override;

	// Theme hook
	virtual void ThemeSet_Implementation() override;

	void ApplyTheme();

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UBorder> Border_Background;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UBorder> Border_TopLine;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UBorder> Border_LeftLine;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UNamedSlot> Slot_Content;
};
