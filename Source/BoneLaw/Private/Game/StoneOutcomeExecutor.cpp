#include "Game/StoneOutcomeExecutor.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/StoneAttributeSet.h"
#include "Game/StoneScheduler.h"
#include "Runtime/StoneRunSubsystem.h"

namespace StoneOutcome
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

void UStoneOutcomeExecutor::ApplyOutcomes(const TArray<FStoneOutcome>& Outcomes, UStoneRunSubsystem* Run, const FStoneOutcomeContext& Ctx)
{
	if (!Run) return;

	for (const FStoneOutcome& O : Outcomes)
	{
		ApplyOutcome(O, Run, Ctx);
	}
}

void UStoneOutcomeExecutor::ApplyOutcome(const FStoneOutcome& O, UStoneRunSubsystem* Run, const FStoneOutcomeContext& Ctx)
{
	if (!Run) return;

	switch (O.Type)
	{
	case EStoneOutcomeType::AttributeDelta:
	{
		if (!Ctx.ASC)
		{
			UE_LOG(LogTemp, Warning, TEXT("[StoneOutcomeExecutor] AttributeDelta requested but ASC is null."));
			break;
		}

		FGameplayAttribute Attr;
		if (!StoneOutcome::ResolveAttributeFromTag(O.AttributeTag, Attr))
		{
			UE_LOG(LogTemp, Warning, TEXT("[StoneOutcomeExecutor] AttributeDelta: unknown AttributeTag '%s'."), *O.AttributeTag.ToString());
			break;
		}

		const float Current = Ctx.ASC->GetNumericAttribute(Attr);
		const float Next = Current + O.Magnitude;

		Ctx.ASC->SetNumericAttributeBase(Attr, Next);

		UE_LOG(LogTemp, Warning, TEXT("[StoneOutcomeExecutor] AttributeDelta %s %+0.2f (%.2f -> %.2f)"),
			*O.AttributeTag.ToString(), O.Magnitude, Current, Next);
		break;
	}

	case EStoneOutcomeType::ApplyGameplayEffect:
	{
		if (Ctx.ASC && *O.GameplayEffectClass)
		{
			FGameplayEffectContextHandle EffectCtx = Ctx.ASC->MakeEffectContext();
			EffectCtx.AddSourceObject(Run);

			const FGameplayEffectSpecHandle Spec = Ctx.ASC->MakeOutgoingSpec(O.GameplayEffectClass, 1.f, EffectCtx);
			if (Spec.IsValid())
			{
				Ctx.ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
				UE_LOG(LogTemp, Warning, TEXT("[StoneOutcomeExecutor] Applied GE %s"), *O.GameplayEffectClass->GetName());
			}
		}
		break;
	}

	case EStoneOutcomeType::AddTags:
		if (Ctx.Tags)
		{
			Ctx.Tags->AppendTags(O.Tags);
			Run->AddStateTags(O.Tags);
		}
		break;

	case EStoneOutcomeType::RemoveTags:
		if (Ctx.Tags)
		{
			Ctx.Tags->RemoveTags(O.Tags);
			Run->RemoveStateTags(O.Tags);
		}
		break;

	case EStoneOutcomeType::ForceNextEvent:
		Run->ForceNextEvent(O.EventId);
		break;

	case EStoneOutcomeType::PoolAddEvent:
		Run->PoolAddEvent(O.EventId);
		break;

	case EStoneOutcomeType::PoolRemoveEvent:
		Run->PoolRemoveEvent(O.EventId);
		break;

	case EStoneOutcomeType::ScheduleEvent:
		if (Ctx.Scheduler && Ctx.Time && O.Scheduled.IsValid())
		{
			Ctx.Scheduler->Enqueue(O.Scheduled, *Ctx.Time);
		}
		break;

	case EStoneOutcomeType::SetFocusTag:
		if (Ctx.FocusTag && O.Tags.Num() > 0)
		{
			*Ctx.FocusTag = O.Tags.First();
			Run->SetFocus(*Ctx.FocusTag);
		}
		break;

	default:
		break;
	}
}
