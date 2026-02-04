#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "StoneContentSettings.generated.h"

/**
 * @class UStoneContentSettings
 * @brief Provides configurable settings for managing Stone assets, validation, and developer-specific controls.
 *
 * This class allows configuration of critical Stone-related assets and validation settings,
 * ensuring the content adheres to project rules. The settings include specifying a
 * registry for attributes, defining required worldline events, and toggling development-only
 * convenience features. It inherits from UDeveloperSettings to integrate with Unreal's
 * settings framework.
 *
 * Configuration Category:
 * - Config=Game: Indicates this class contains game-specific configuration settings.
 * - DefaultConfig: Specifies the class generates a default configuration file.
 * - meta=(DisplayName="Stone Content"): Sets the display name in the settings UI.
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Stone Content"))
class BONELAW_API UStoneContentSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Registry Asset (Design decision, not Engine SSOT)
	UPROPERTY(EditAnywhere, Config, Category="Stone|Registry")
	TSoftObjectPtr<class UStoneAttributeRegistry> AttributeRegistry;

	// Required payoff events (Bible gate)
	UPROPERTY(EditAnywhere, Config, Category="Stone|Validation")
	TArray<FName> RequiredWorldlineEventIds;


	// Dev-only convenience
	UPROPERTY(EditAnywhere, Config, Category="Stone|Validation")
	bool bRunValidatorOnBeginPlayDevOnly = true;
};
