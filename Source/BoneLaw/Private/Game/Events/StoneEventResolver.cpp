#include "Game/Events/StoneEventResolver.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/StoneAttributeSet.h"
#include "Core/StoneGameplayTags.h"
#include "Data/StoneEventData.h"

namespace StoneResolver
{
	static bool ResolveAttributeFromTag(const FGameplayTag& AttributeTag, FGameplayAttribute& OutAttribute)
	{
		OutAttribute = FGameplayAttribute();
		if (!AttributeTag.IsValid())
		{
			return false;
		}

		const UStoneAttributeSet* Defaults = GetDefault<UStoneAttributeSet>();
		if (!Defaults)
		{
			return false;
		}

		const TStaticFuncPtr<FGameplayAttribute()>* Fn = Defaults->TagsToAttributes.Find(AttributeTag);
		if (!Fn)
		{
			return false;
		}

		OutAttribute = (*Fn)();
		return OutAttribute.IsValid();
	}
}

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
		for (const FStoneAttributeMin& Min : Req.MinAttributes)
		{
			FGameplayAttribute Attr;
			if (!StoneResolver::ResolveAttributeFromTag(Min.AttributeTag, Attr))
			{
				UE_LOG(LogTemp, Warning, TEXT("[StoneEventResolver] Requirement references unknown AttributeTag '%s'."), *Min.AttributeTag.ToString());
				return false;
			}

			const float V = ASC->GetNumericAttribute(Attr);
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
				R.bSoftFail = true;
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

	if (Snapshot.Food < 25.f && Event->EventTags.HasTag(Tags.Event_Hunt))
	{
		Weight *= 2;
	}

	if (Snapshot.FocusTag.IsValid())
	{
		if (Snapshot.FocusTag == Tags.Focus_Hunt && Event->EventTags.HasTag(Tags.Event_Hunt)) Weight *= 2;
		else if (Snapshot.FocusTag == Tags.Focus_Shelter && Event->EventTags.HasTag(Tags.Event_Shelter)) Weight *= 2;
		else if (Snapshot.FocusTag == Tags.Focus_Water && Event->EventTags.HasTag(Tags.Event_Water)) Weight *= 2;
		else if (Snapshot.FocusTag == Tags.Focus_Fire && Event->EventTags.HasTag(Tags.Event_Fire)) Weight *= 2;
		else if (Snapshot.FocusTag == Tags.Focus_Forage && Event->EventTags.HasTag(Tags.Event_Forage)) Weight *= 2;
	}

	if (Snapshot.Time.bIsNight && Event->EventTags.HasTag(Tags.Event_Night))
	{
		Weight *= 2;
	}

	return Weight;
}
