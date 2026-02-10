// Copyright by MykeUhu

#include "Game/Buildings/StoneFoliageClearComponent.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/PrimitiveComponent.h"
#include "Runtime/Foliage/Public/FoliageInstancedStaticMeshComponent.h"
#include "Runtime/Foliage/Public/InstancedFoliageActor.h"

class UFoliageInstancedStaticMeshComponent;

UStoneFoliageClearComponent::UStoneFoliageClearComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStoneFoliageClearComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bClearOnBeginPlay)
	{
		ClearFoliageNow();
	}
}

bool UStoneFoliageClearComponent::IsMeshAllowed(UStaticMesh* Mesh) const
{
	if (OnlyTheseMeshes.Num() == 0) return true;
	if (!Mesh) return false;

	for (const TSoftObjectPtr<UStaticMesh>& Allowed : OnlyTheseMeshes)
	{
		if (Allowed.IsNull()) continue;
		if (Allowed.Get() == Mesh) return true;
	}
	return false;
}

void UStoneFoliageClearComponent::ClearFoliageNow()
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (!World || !Owner) return;

	// Decide clearing region
	const FVector Origin = Owner->GetActorLocation();

	FBox ClearBox(EForceInit::ForceInitToZero);
	float ClearRadius = 0.f;

	if (Shape == EStoneFoliageClearShape::UseOwnerBounds)
	{
		// Use the full component bounds of the owner (best for buildings)
		FBox Bounds = Owner->GetComponentsBoundingBox(/*bNonColliding=*/true);
		Bounds = Bounds.ExpandBy(BoundsPadding);
		ClearBox = Bounds;
	}
	else
	{
		ClearRadius = FMath::Max(0.f, SphereRadius);
	}

	int32 TotalRemoved = 0;

	// World Partition safe: iterate all InstancedFoliageActors that are currently loaded
	for (TActorIterator<AInstancedFoliageActor> It(World); It; ++It)
	{
		AInstancedFoliageActor* IFA = *It;
		if (!IFA) continue;

		// Iterate foliage mesh components owned by this IFA
		TArray<UFoliageInstancedStaticMeshComponent*> FoliageComps;
		IFA->GetComponents<UFoliageInstancedStaticMeshComponent>(FoliageComps);

		for (UFoliageInstancedStaticMeshComponent* Comp : FoliageComps)
		{
			if (!Comp) continue;
			if (!IsMeshAllowed(Comp->GetStaticMesh())) continue;

			const int32 Count = Comp->GetInstanceCount();
			if (Count <= 0) continue;

			TArray<int32> ToRemove;
			ToRemove.Reserve(128);

			FTransform InstXf;
			for (int32 i = 0; i < Count; ++i)
			{
				if (!Comp->GetInstanceTransform(i, InstXf, /*bWorldSpace=*/true))
				{
					continue;
				}

				const FVector P = InstXf.GetLocation();

				bool bInside = false;
				if (Shape == EStoneFoliageClearShape::UseOwnerBounds)
				{
					bInside = ClearBox.IsInsideOrOn(P);
				}
				else
				{
					bInside = FVector::DistSquared(P, Origin) <= FMath::Square(ClearRadius);
				}

				if (bInside)
				{
					ToRemove.Add(i);
				}
			}

			if (ToRemove.Num() > 0)
			{
				// RemoveInstances expects indices; it handles internal compaction.
				Comp->RemoveInstances(ToRemove);
				TotalRemoved += ToRemove.Num();
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[StoneFoliageClear] %s removed %d foliage instances."), *GetNameSafe(Owner), TotalRemoved);
}
