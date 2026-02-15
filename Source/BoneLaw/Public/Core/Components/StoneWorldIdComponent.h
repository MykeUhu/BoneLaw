// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StoneWorldIdComponent.generated.h"

/**
 * UStoneWorldIdComponent - Persistent identity for world actors
 * 
 * CRITICAL: For placed-in-level actors, the ID MUST be editor-stable.
 * 
 * HYBRID DESIGN:
 * A) PLACED CONTENT (TaskActors in level):
 *    - WorldId is EditInstanceOnly → each placed actor gets unique ID in editor
 *    - ID is generated once at editor-time and saved with the map
 *    - At runtime: ID is read-only and stable across sessions
 * 
 * B) RUNTIME SPAWNED (basebuilding, dynamic settlers):
 *    - ID is generated at spawn time (in BeginPlay)
 *    - Saved in FSavedBuildable or FSavedSettler
 * 
 * Usage:
 *   - Add to Task Actors placed in level (wood spots, water sources)
 *   - Add to dynamically spawned actors (future basebuilding)
 */
UCLASS(ClassGroup=(Stone), meta=(BlueprintSpawnableComponent))
class BONELAW_API UStoneWorldIdComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStoneWorldIdComponent();

	/** Returns the persistent world ID for this actor. */
	UFUNCTION(BlueprintPure, Category="Stone|Identity")
	FGuid GetWorldId() const { return WorldId; }

	/** Returns true if ID has been assigned. */
	UFUNCTION(BlueprintPure, Category="Stone|Identity")
	bool HasValidId() const { return WorldId.IsValid(); }
	
	/** Editor-only: Regenerate ID (use with caution - breaks save references). */
	UFUNCTION(BlueprintCallable, Category="Stone|Identity", meta=(CallInEditor="true"))
	void RegenerateId();

protected:
	virtual void BeginPlay() override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void OnComponentCreated() override;
#endif

private:
	/** 
	 * Persistent unique identifier for this world object.
	 * 
	 * PLACED ACTORS: EditInstanceOnly ensures each placed actor has unique ID.
	 * The ID is generated in editor and saved with the map.
	 * 
	 * RUNTIME ACTORS: ID is generated at spawn time if invalid.
	 */
	UPROPERTY(EditInstanceOnly, SaveGame, Category="Stone|Identity", meta=(DisplayName="World Object ID"))
	FGuid WorldId;
	
	/** Generates new ID (runtime fallback for spawned actors). */
	void GenerateNewId();
};
