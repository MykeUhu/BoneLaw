#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/StoneUserWidget.h"
#include "GameplayTagContainer.h"
#include "StoneStatsPanelWidget.generated.h"

class UListView;
class UStoneWidgetController;
class UStoneStatsWidgetController;
class UStoneStatEntryObject;
struct FStoneAttributeInfo;

UENUM()
enum class EStoneStatGroup : uint8
{
	Primary,
	Secondary,
	Culture,
	Knowledge,
	Worldline,
	Vital,
	Other
};

UCLASS()
class BONELAW_API UStoneStatsPanelWidget : public UStoneUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category="Stone|UI")
	void SetStatsController(UStoneWidgetController* InController);

protected:
	// Bind these in your WBP with EXACT names
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UListView> ListView_Primary;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UListView> ListView_Secondary;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UListView> ListView_Culture;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UListView> ListView_Knowledge;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UListView> ListView_Worldline;

	// Optional: show vitals here too (probably not needed)
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UListView> ListView_Vital;

private:
	void BindAll();
	void UnbindAll();

	UFUNCTION()
	void HandleAttributeInfo(const FStoneAttributeInfo& Info);

	EStoneStatGroup ResolveGroupFromTag(const FGameplayTag& Tag) const;
	UListView* GetListForGroup(EStoneStatGroup Group) const;

	UStoneStatsWidgetController* GetStatsController() const;

	void UpsertEntry(EStoneStatGroup Group, const FStoneAttributeInfo& Info);

private:
	UPROPERTY()
	TObjectPtr<UStoneWidgetController> StatsControllerBase;

	// Per-group caches (Tag -> EntryObject)
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UStoneStatEntryObject>> PrimaryMap;

	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UStoneStatEntryObject>> SecondaryMap;

	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UStoneStatEntryObject>> CultureMap;

	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UStoneStatEntryObject>> KnowledgeMap;

	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UStoneStatEntryObject>> WorldlineMap;

	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UStoneStatEntryObject>> VitalMap;
};
