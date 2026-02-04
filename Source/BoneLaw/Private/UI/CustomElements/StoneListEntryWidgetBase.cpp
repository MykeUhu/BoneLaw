#include "UI/CustomElements/StoneListEntryWidgetBase.h"

#include "UI/CustomElements/StoneCustomPanel.h"

void UStoneListEntryWidgetBase::NativePreConstruct()
{
	Super::NativePreConstruct();
	ApplyEntryStateToPanel();
}

void UStoneListEntryWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	ApplyEntryStateToPanel();
}

void UStoneListEntryWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UStoneListEntryWidgetBase::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	ApplyEntryStateToPanel();
}

void UStoneListEntryWidgetBase::ThemeSet_Implementation()
{
	Super::ThemeSet_Implementation();

	// When theme changes, re-apply visual state
	ApplyEntryStateToPanel();
}

void UStoneListEntryWidgetBase::NativeOnListItemObjectSet(UObject* InListItemObject)
{
	ListItemObject = InListItemObject;

	// Give BP child the object so it can populate UI
	BP_OnListItemObjectSet(InListItemObject);

	// Ensure state visuals are correct after data changes
	ApplyEntryStateToPanel();
}

void UStoneListEntryWidgetBase::NativeOnItemSelectionChanged(bool bIsSelectedIn)
{
	bSelected = bIsSelectedIn;

	BP_OnSelectionChanged(bSelected);
	ApplyEntryStateToPanel();
}

void UStoneListEntryWidgetBase::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	bHovered = true;
	BP_OnHoverChanged(true);
	ApplyEntryStateToPanel();
}

void UStoneListEntryWidgetBase::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	bHovered = false;
	BP_OnHoverChanged(false);
	ApplyEntryStateToPanel();
}

void UStoneListEntryWidgetBase::SetEntrySelected(bool bInSelected)
{
	bSelected = bInSelected;
	BP_OnSelectionChanged(bSelected);
	ApplyEntryStateToPanel();
}

void UStoneListEntryWidgetBase::SetEntryHovered(bool bInHovered)
{
	bHovered = bInHovered;
	BP_OnHoverChanged(bHovered);
	ApplyEntryStateToPanel();
}

void UStoneListEntryWidgetBase::ApplyEntryStateToPanel()
{
	if (!Panel_Root)
	{
		return;
	}

	// Keep a unified behaviour across all list entries:
	// - Hover changes panel appearance
	// - Selected changes panel appearance
	Panel_Root->SetHovered(bHovered);
	Panel_Root->SetSelected(bSelected);

	// Optional: if you want selected to accent laser line
	// Panel_Root->bLaserLineAccentWhenSelected = true; // only if you prefer global behaviour
}
