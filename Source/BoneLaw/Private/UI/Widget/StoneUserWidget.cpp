#include "UI/Widget/StoneUserWidget.h"

#include "Core/StoneGameInstance.h"

void UStoneUserWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	ResolveThemeIfMissing();
}

void UStoneUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ResolveThemeIfMissing();
}


void UStoneUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	// Native hook (C++ and BP)
	WidgetControllerSet();
}

// ============================================================
// Theme (single source of truth for all custom widgets)
// ============================================================

void UStoneUserWidget::ResolveThemeIfMissing()
{
	if (Theme) return;

	if (const UWorld* World = GetWorld())
	{
		if (UStoneGameInstance* GI = World->GetGameInstance<UStoneGameInstance>())
		{
			if (GI->GetUITheme())
			{
				Theme = GI->GetUITheme();
				ThemeSet();
			}
		}
	}
}

void UStoneUserWidget::SetTheme(UStoneUIThemeDataAsset* InTheme)
{
	Theme = InTheme;

	// Native hook (C++ and BP)
	ThemeSet();
}

void UStoneUserWidget::ThemeSet_Implementation()
{
	// default: nothing
}

