#include "UI/Widget/Stats/StoneStatsPanelWidget.h"

#include "Components/ListView.h"
#include "UI/WidgetController/StoneWidgetController.h"
#include "UI/WidgetController/StoneStatsWidgetController.h"
#include "UI/Widget/Stats/StoneStatEntryObject.h"

void UStoneStatsPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UStoneStatsPanelWidget::NativeDestruct()
{
	UnbindAll();
	Super::NativeDestruct();
}

void UStoneStatsPanelWidget::SetStatsController(UStoneWidgetController* InController)
{
	if (StatsControllerBase == InController) return;

	UnbindAll();
	StatsControllerBase = InController;

	UStoneStatsWidgetController* SC = GetStatsController();
	if (!SC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StoneUI][StatsPanel] SetStatsController: not UStoneStatsWidgetController (got %s)"),
			InController ? *InController->GetClass()->GetName() : TEXT("null"));
		return;
	}

	BindAll();

	// Force initial push (Aura pattern)
	SC->BroadcastInitialValues();
}

void UStoneStatsPanelWidget::BindAll()
{
	if (UStoneStatsWidgetController* SC = GetStatsController())
	{
		SC->AttributeInfoDelegate.RemoveDynamic(this, &UStoneStatsPanelWidget::HandleAttributeInfo);
		SC->AttributeInfoDelegate.AddDynamic(this, &UStoneStatsPanelWidget::HandleAttributeInfo);
	}
}

void UStoneStatsPanelWidget::UnbindAll()
{
	if (UStoneStatsWidgetController* SC = GetStatsController())
	{
		SC->AttributeInfoDelegate.RemoveDynamic(this, &UStoneStatsPanelWidget::HandleAttributeInfo);
	}

	StatsControllerBase = nullptr;
}

UStoneStatsWidgetController* UStoneStatsPanelWidget::GetStatsController() const
{
	return Cast<UStoneStatsWidgetController>(StatsControllerBase);
}

void UStoneStatsPanelWidget::HandleAttributeInfo(const FStoneAttributeInfo& Info)
{
	const EStoneStatGroup Group = ResolveGroupFromTag(Info.AttributeTag);

	// Skip vitals if you don’t want duplicates (Overlay already shows them)
	if (Group == EStoneStatGroup::Vital && !ListView_Vital)
	{
		return;
	}

	UpsertEntry(Group, Info);
}

EStoneStatGroup UStoneStatsPanelWidget::ResolveGroupFromTag(const FGameplayTag& Tag) const
{
	const FString S = Tag.ToString();

	if (S.StartsWith(TEXT("Attributes.Primary"))) return EStoneStatGroup::Primary;
	if (S.StartsWith(TEXT("Attributes.Secondary"))) return EStoneStatGroup::Secondary;
	if (S.StartsWith(TEXT("Attributes.Culture"))) return EStoneStatGroup::Culture;
	if (S.StartsWith(TEXT("Attributes.Knowledge"))) return EStoneStatGroup::Knowledge;
	if (S.StartsWith(TEXT("Attributes.Worldline"))) return EStoneStatGroup::Worldline;
	if (S.StartsWith(TEXT("Attributes.Vital"))) return EStoneStatGroup::Vital;

	return EStoneStatGroup::Other;
}

UListView* UStoneStatsPanelWidget::GetListForGroup(EStoneStatGroup Group) const
{
	switch (Group)
	{
	case EStoneStatGroup::Primary:   return ListView_Primary;
	case EStoneStatGroup::Secondary: return ListView_Secondary;
	case EStoneStatGroup::Culture:   return ListView_Culture;
	case EStoneStatGroup::Knowledge: return ListView_Knowledge;
	case EStoneStatGroup::Worldline: return ListView_Worldline;
	case EStoneStatGroup::Vital:     return ListView_Vital;
	default:                         return nullptr;
	}
}

void UStoneStatsPanelWidget::UpsertEntry(EStoneStatGroup Group, const FStoneAttributeInfo& Info)
{
	UListView* List = GetListForGroup(Group);
	if (!List) return;

	TMap<FGameplayTag, TObjectPtr<UStoneStatEntryObject>>* MapPtr = nullptr;
	switch (Group)
	{
	case EStoneStatGroup::Primary:   MapPtr = &PrimaryMap; break;
	case EStoneStatGroup::Secondary: MapPtr = &SecondaryMap; break;
	case EStoneStatGroup::Culture:   MapPtr = &CultureMap; break;
	case EStoneStatGroup::Knowledge: MapPtr = &KnowledgeMap; break;
	case EStoneStatGroup::Worldline: MapPtr = &WorldlineMap; break;
	case EStoneStatGroup::Vital:     MapPtr = &VitalMap; break;
	default: return;
	}

	TObjectPtr<UStoneStatEntryObject>& Entry = (*MapPtr).FindOrAdd(Info.AttributeTag);
	float OldValue = 0.f;

	if (!Entry)
	{
		Entry = NewObject<UStoneStatEntryObject>(this);
		OldValue = Info.AttributeValue; // first time: no delta flash
		Entry->UpdateFromInfo(Info);
		List->AddItem(Entry);
	}
	else
	{
		OldValue = Entry->Value;
		Entry->UpdateFromInfo(Info);

		// Tell ListView to refresh
		List->RequestRefresh();
	}
}
