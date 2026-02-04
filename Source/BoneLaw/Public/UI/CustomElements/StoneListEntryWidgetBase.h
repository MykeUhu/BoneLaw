// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "UI/Widget/StoneUserWidget.h"
#include "UObject/Object.h"
#include "StoneListEntryWidgetBase.generated.h"

class UStoneCustomPanel;
/**
 * 
 */
UCLASS()
class BONELAW_API UStoneListEntryWidgetBase : public UStoneUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
public:
	// ============================================================
	// Data access
	// ============================================================
	UFUNCTION(BlueprintPure, Category="Stone|ListEntry")
	UObject* GetListItemObject() const { return ListItemObject; }

	UFUNCTION(BlueprintPure, Category="Stone|ListEntry")
	bool IsEntrySelected() const { return bSelected; }

	UFUNCTION(BlueprintPure, Category="Stone|ListEntry")
	bool IsEntryHovered() const { return bHovered; }

	// Optional: allow forcing states from outside (rarely needed, but useful)
	UFUNCTION(BlueprintCallable, Category="Stone|ListEntry")
	void SetEntrySelected(bool bInSelected);

	UFUNCTION(BlueprintCallable, Category="Stone|ListEntry")
	void SetEntryHovered(bool bInHovered);

protected:
	// ============================================================
	// UUserWidget lifecycle
	// ============================================================
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void SynchronizeProperties() override;

	// Theme hook (from UStoneUserWidget)
	virtual void ThemeSet_Implementation() override;

	// ============================================================
	// List entry hooks
	// ============================================================
	virtual void NativeOnListItemObjectSet(UObject* InListItemObject) override;
	virtual void NativeOnItemSelectionChanged(bool bIsSelected) override;

	// Hover handling (so your panel reacts the same everywhere)
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	// ============================================================
	// Blueprint extension points
	// ============================================================

	/** Called whenever the ListItemObject is set/changed. Implement in BP child to fill texts/images. */
	UFUNCTION(BlueprintImplementableEvent, Category="Stone|ListEntry")
	void BP_OnListItemObjectSet(UObject* InListItemObject);

	/** Called whenever selection changes. Optional override in BP child. */
	UFUNCTION(BlueprintImplementableEvent, Category="Stone|ListEntry")
	void BP_OnSelectionChanged(bool bInSelected);

	/** Called whenever hover changes. Optional override in BP child. */
	UFUNCTION(BlueprintImplementableEvent, Category="Stone|ListEntry")
	void BP_OnHoverChanged(bool bInHovered);

	// ============================================================
	// Shared visuals
	// ============================================================

	/** Optional: if your entry has a UStoneCustomPanel as root, bind it here for consistent highlight behavior. */
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UStoneCustomPanel> Panel_Root;

	// ============================================================
	// Internal state
	// ============================================================
	UPROPERTY(BlueprintReadOnly, Category="Stone|ListEntry")
	TObjectPtr<UObject> ListItemObject;

	UPROPERTY(BlueprintReadOnly, Category="Stone|ListEntry")
	bool bSelected = false;

	UPROPERTY(BlueprintReadOnly, Category="Stone|ListEntry")
	bool bHovered = false;

private:
	void ApplyEntryStateToPanel();
};
