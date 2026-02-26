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

void UStoneOutcomeExecutor::ApplyOutcomes(const TArray<FStoneOutcome>& Outcomes, const FStoneOutcomeContext& Ctx)
{
	for (const FStoneOutcome& O : Outcomes)
	{
		ApplyOutcome(O, Ctx);
	}
}

void UStoneOutcomeExecutor::ApplyOutcome(const FStoneOutcome& O, const FStoneOutcomeContext& Ctx)
{
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

		UE_LOG(LogTemp, Log, TEXT("[StoneOutcomeExecutor] AttributeDelta %s %+0.2f (%.2f -> %.2f)"),
			*O.AttributeTag.ToString(), O.Magnitude, Current, Next);
		break;
	}

	case EStoneOutcomeType::ApplyGameplayEffect:
	{
		if (!Ctx.ASC)
		{
			UE_LOG(LogTemp, Warning, TEXT("[StoneOutcomeExecutor] ApplyGameplayEffect requested but ASC is null."));
			break;
		}
		if (!*O.GameplayEffectClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("[StoneOutcomeExecutor] ApplyGameplayEffect requested but GameplayEffectClass is null."));
			break;
		}

		FGameplayEffectContextHandle EffectCtx = Ctx.ASC->MakeEffectContext();
			EffectCtx.AddSourceObject(Ctx.SourceObject ? Ctx.SourceObject : Ctx.ASC->GetAvatarActor());

		const FGameplayEffectSpecHandle Spec = Ctx.ASC->MakeOutgoingSpec(O.GameplayEffectClass, 1.f, EffectCtx);
		if (Spec.IsValid())
		{
			Ctx.ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			UE_LOG(LogTemp, Log, TEXT("[StoneOutcomeExecutor] Applied GE %s"), *O.GameplayEffectClass->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[StoneOutcomeExecutor] Failed to build GE spec for %s"), *O.GameplayEffectClass->GetName());
		}
		break;
	}

	case EStoneOutcomeType::AddTags:
	{
		// Local runtime tags (optional)
		if (Ctx.Tags)
		{
			Ctx.Tags->AppendTags(O.Tags);
		}

		// Authoritative GAS tags on the agent
		if (Ctx.ASC && O.Tags.Num() > 0)
		{
			Ctx.ASC->AddLooseGameplayTags(O.Tags);
		}

		UE_LOG(LogTemp, Log, TEXT("[StoneOutcomeExecutor] AddTags x%d"), O.Tags.Num());
		break;
	}

	case EStoneOutcomeType::RemoveTags:
	{
		if (Ctx.Tags)
		{
			Ctx.Tags->RemoveTags(O.Tags);
		}

		if (Ctx.ASC && O.Tags.Num() > 0)
		{
			Ctx.ASC->RemoveLooseGameplayTags(O.Tags);
		}

		UE_LOG(LogTemp, Log, TEXT("[StoneOutcomeExecutor] RemoveTags x%d"), O.Tags.Num());
		break;
	}

	case EStoneOutcomeType::ScheduleEvent:
	{
		// (Optional) wenn du später Scheduler wieder willst – aktuell ohne RunSubsystem lassen wir es sauber “no-op”:
		if (Ctx.Scheduler && Ctx.Time && O.Scheduled.IsValid())
		{
			Ctx.Scheduler->Enqueue(O.Scheduled, *Ctx.Time);
			UE_LOG(LogTemp, Log, TEXT("[StoneOutcomeExecutor] ScheduledEvent enqueued."));
		}
		else
		{
			UE_LOG(LogTemp, Verbose, TEXT("[StoneOutcomeExecutor] ScheduleEvent ignored (no scheduler/time)."));
		}
		break;
	}

	case EStoneOutcomeType::SetFocusTag:
	case EStoneOutcomeType::ForceNextEvent:
	case EStoneOutcomeType::PoolAddEvent:
	case EStoneOutcomeType::PoolRemoveEvent:
	{
		// RunSubsystem-Only Features: bewusst deaktiviert, weil globale Events weg sind.
		UE_LOG(LogTemp, Verbose, TEXT("[StoneOutcomeExecutor] Run-level outcome ignored (Type=%d)."), (int32)O.Type);
		break;
	}

	default:
		break;
	}
}