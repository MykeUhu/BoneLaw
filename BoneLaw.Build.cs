// Copyright by MykeUhu

using UnrealBuildTool;

public class BoneLaw : ModuleRules
{
	public BoneLaw(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput", // Enhanced Input System (UE5)

			// UI
			"UMG",
			"Slate",
			"SlateCore",

			// Gameplay Tags
			"GameplayTags",
			
			// Für UDeveloperSettings
			"DeveloperSettings",

			// GAS
			"GameplayAbilities",
			"GameplayTasks",
		});


		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// Used by StoneContentValidator (FAssetRegistryModule, FARFilter)
			"AIModule", 
			"AssetRegistry", 
			"Foliage",
		});
	}
}
