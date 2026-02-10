// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StoneFoliageClearComponent.generated.h"

UENUM(BlueprintType)
enum class EStoneFoliageClearShape : uint8
{
	UseOwnerBounds UMETA(DisplayName="Use Owner Bounds (Box)"),
	UseSphereRadius UMETA(DisplayName="Use Sphere Radius"),
};

UCLASS(ClassGroup=(Stone), meta=(BlueprintSpawnableComponent))
class BONELAW_API UStoneFoliageClearComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStoneFoliageClearComponent();

	/** Call this when building placement is confirmed */
	UFUNCTION(BlueprintCallable, Category="Stone|Foliage")
	void ClearFoliageNow();

	/** Optional: call in editor for testing (won't affect cooked runtime) */
	UFUNCTION(CallInEditor, Category="Stone|Foliage")
	void ClearFoliageNow_Editor() { ClearFoliageNow(); }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="Stone|Foliage")
	bool bClearOnBeginPlay = false;

	UPROPERTY(EditAnywhere, Category="Stone|Foliage")
	EStoneFoliageClearShape Shape = EStoneFoliageClearShape::UseOwnerBounds;

	/** Extra size added to owner bounds box (cm) */
	UPROPERTY(EditAnywhere, Category="Stone|Foliage", meta=(EditCondition="Shape==EStoneFoliageClearShape::UseOwnerBounds"))
	FVector BoundsPadding = FVector(50.f, 50.f, 50.f);

	/** Sphere radius in cm */
	UPROPERTY(EditAnywhere, Category="Stone|Foliage", meta=(EditCondition="Shape==EStoneFoliageClearShape::UseSphereRadius", ClampMin="0.0"))
	float SphereRadius = 200.f;

	/** If true, only clears foliage types whose mesh matches these (optional filter). Leave empty to clear all foliage. */
	UPROPERTY(EditAnywhere, Category="Stone|Foliage")
	TArray<TSoftObjectPtr<UStaticMesh>> OnlyTheseMeshes;

private:
	bool IsMeshAllowed(UStaticMesh* Mesh) const;
};
