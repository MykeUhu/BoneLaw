// Copyright by MykeUhu

#include "Core/Components/StoneWorldIdComponent.h"

#include "GameFramework/Actor.h"

UStoneWorldIdComponent::UStoneWorldIdComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStoneWorldIdComponent::BeginPlay()
{
	Super::BeginPlay();

	// Runtime fallback:
	// - Placed-in-level actors should already have an ID (generated in editor and saved in the map).
	// - Runtime-spawned actors (future basebuilding) may need an ID generated now.
	if (!WorldId.IsValid())
	{
		const AActor* Owner = GetOwner();
		const bool bWasLoadedFromPackage = Owner && Owner->HasAnyFlags(RF_WasLoaded);
		if (bWasLoadedFromPackage)
		{
			UE_LOG(LogTemp, Error, TEXT("[StoneWorldId] Actor '%s' was loaded without a WorldId. This should be assigned in-editor. Generating a new one at runtime (may break save references)."),
				Owner ? *Owner->GetName() : TEXT("<null>"));
		}

		GenerateNewId();
	}
}

void UStoneWorldIdComponent::RegenerateId()
{
#if WITH_EDITOR
	GenerateNewId();
	if (AActor* Owner = GetOwner())
	{
		Owner->Modify();
		Owner->MarkPackageDirty();
	}
#else
	UE_LOG(LogTemp, Warning, TEXT("[StoneWorldId] RegenerateId is editor-only."));
#endif
}

#if WITH_EDITOR
void UStoneWorldIdComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (!WorldId.IsValid())
	{
		GenerateNewId();
	}
}

void UStoneWorldIdComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
	if (!WorldId.IsValid())
	{
		GenerateNewId();
	}
}
#endif

void UStoneWorldIdComponent::GenerateNewId()
{
	WorldId = FGuid::NewGuid();
	if (const AActor* Owner = GetOwner())
	{
		UE_LOG(LogTemp, Log, TEXT("[StoneWorldId] Assigned WorldId %s to actor '%s'."), *WorldId.ToString(), *Owner->GetName());
	}
}
