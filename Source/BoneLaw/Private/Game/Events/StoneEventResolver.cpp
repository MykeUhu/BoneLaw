#include "Game/Events/StoneEventResolver.h"

#include "AbilitySystemComponent.h"
#include "Core/StoneGameplayTags.h"
#include "Data/StoneEventData.h"

bool UStoneEventResolver::EvaluateRequirement(const FStoneRequirement& Req, const UAbilitySystemComponent* ASC, const FGameplayTagContainer& Tags) const
{
	if (!Req.BlockedTagsAny.IsEmpty() && Tags.HasAny(Req.BlockedTagsAny))
	{
		return false;
	}

	if (!Req.RequiredTagsAll.IsEmpty() && !Tags.HasAll(Req.RequiredTagsAll))
	{
		return false;
	}

	if (!Req.MustMatchQuery.IsEmpty())
	{
		FGameplayTagContainer Copy = Tags;
		if (!Req.MustMatchQuery.Matches(Copy))
		{
			return false;
		}
	}

	if (ASC)
	{
		for (const auto& Min : Req.MinAttributes)
		{
			const float V = ASC->GetNumericAttribute(Min.Attribute);
			if (V < Min.MinValue)
			{
				return false;
			}
		}
	}

	return true;
}

void UStoneEventResolver::ResolveChoices(const UStoneEventData* Event, const UAbilitySystemComponent* ASC, const FGameplayTagContainer& Tags, TArray<FStoneChoiceResolved>& Out) const
{
	Out.Reset();
	if (!Event) return;

	Out.SetNum(Event->Choices.Num());

	for (int32 i = 0; i < Event->Choices.Num(); ++i)
	{
		const FStoneChoiceData& C = Event->Choices[i];
		FStoneChoiceResolved R;

		const bool bReqOk = EvaluateRequirement(C.Requirement, ASC, Tags);

		if (!bReqOk)
		{
			switch (C.LockMode)
			{
			case EStoneChoiceLockMode::Hidden:
				R.bVisible = false;
				break;
			case EStoneChoiceLockMode::Disabled:
				R.bEnabled = false;
				break;
			case EStoneChoiceLockMode::SoftFail:
				R.bSoftFail = true; // still enabled
				break;
			}
		}

		Out[i] = R;
	}
}

int32 UStoneEventResolver::ComputeFinalWeight(const UStoneEventData* Event, const FStoneSnapshot& Snapshot) const
{
	if (!Event) return 0;

	const FStoneGameplayTags& Tags = FStoneGameplayTags::Get();

	int32 Weight = FMath::Max(0, Event->BaseWeight);

	// Crisis weighting (Beispiel)
	if (Snapshot.Food < 25.f && Event->EventTags.HasTag(Tags.Event_Hunt))
	{
		Weight *= 2;
	}

	// Focus weighting (Focus -> Event category mapping)
	if (Snapshot.FocusTag.IsValid())
	{
		if (Snapshot.FocusTag == Tags.Focus_Hunt && Event->EventTags.HasTag(Tags.Event_Hunt))
		{
			Weight *= 2;
		}
		else if (Snapshot.FocusTag == Tags.Focus_Shelter && Event->EventTags.HasTag(Tags.Event_Shelter))
		{
			Weight *= 2;
		}
		else if (Snapshot.FocusTag == Tags.Focus_Water && Event->EventTags.HasTag(Tags.Event_Water))
		{
			Weight *= 2;
		}
		else if (Snapshot.FocusTag == Tags.Focus_Fire && Event->EventTags.HasTag(Tags.Event_Fire))
		{
			Weight *= 2;
		}
		else if (Snapshot.FocusTag == Tags.Focus_Forage && Event->EventTags.HasTag(Tags.Event_Forage))
		{
			Weight *= 2;
		}
	}

	// Night weighting
	if (Snapshot.Time.bIsNight && Event->EventTags.HasTag(Tags.Event_Night))
	{
		Weight *= 2;
	}

	return Weight;
}
