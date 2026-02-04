#include "Game/StoneOutcomeExecutor.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Data/StoneTypes.h"
#include "Game/StoneScheduler.h"

static void ApplyInstantDelta(UAbilitySystemComponent* ASC, const FGameplayAttribute& Attr, float Magnitude)
{
	if (!ASC || !Attr.IsValid()) return;
	const float Cur = ASC->GetNumericAttribute(Attr);
	ASC->SetNumericAttributeBase(Attr, Cur + Magnitude);
}

void UStoneOutcomeExecutor::Execute(const TArray<FStoneOutcome>& Outcomes, FStoneOutcomeContext& Ctx)
{
	for (const FStoneOutcome& O : Outcomes)
	{
		switch (O.Type)
		{
		case EStoneOutcomeType::AttributeDelta:
			ApplyInstantDelta(Ctx.ASC, O.Attribute, O.Magnitude);
			break;

		case EStoneOutcomeType::ApplyGameplayEffect:
			if (Ctx.ASC && O.GameplayEffectClass)
			{
				FGameplayEffectContextHandle CtxHandle = Ctx.ASC->MakeEffectContext();
				CtxHandle.AddSourceObject(this);
				const FGameplayEffectSpecHandle Spec = Ctx.ASC->MakeOutgoingSpec(O.GameplayEffectClass, 1.f, CtxHandle);
				if (Spec.IsValid())
				{
					Ctx.ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
				}
			}
			break;

		case EStoneOutcomeType::AddTags:
			if (Ctx.Tags) { Ctx.Tags->AppendTags(O.Tags); }
			break;

		case EStoneOutcomeType::RemoveTags:
			if (Ctx.Tags)
			{
				for (const FGameplayTag& T : O.Tags)
				{
					Ctx.Tags->RemoveTag(T);
				}
			}
			break;

		case EStoneOutcomeType::PoolAddEvent:
			if (Ctx.EventPoolIds && O.EventId != NAME_None)
			{
				Ctx.EventPoolIds->AddUnique(O.EventId);
			}
			break;

		case EStoneOutcomeType::PoolRemoveEvent:
			if (Ctx.EventPoolIds && O.EventId != NAME_None)
			{
				Ctx.EventPoolIds->Remove(O.EventId);
			}
			break;

		case EStoneOutcomeType::ForceNextEvent:
			// This is handled at RunSubsystem level via a dedicated field; keep executor pure.
			// Use ScheduleEvent Forced with Offset=0 for deterministic behavior.
			break;

		case EStoneOutcomeType::ScheduleEvent:
			if (Ctx.Scheduler && Ctx.Time)
			{
				Ctx.Scheduler->Enqueue(O.Scheduled, *Ctx.Time);
			}
			break;

		case EStoneOutcomeType::SetFocusTag:
			if (Ctx.FocusTag && O.Tags.Num() > 0)
			{
				*Ctx.FocusTag = O.Tags.First();
			}
			break;
		}
	}
}
