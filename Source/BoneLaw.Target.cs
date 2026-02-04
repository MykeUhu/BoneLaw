// Copyright by MykeUhu

using UnrealBuildTool;
using System.Collections.Generic;

public class BoneLawTarget : TargetRules
{
	public BoneLawTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;

		ExtraModuleNames.AddRange( new string[] { "BoneLaw" } );
	}
}
