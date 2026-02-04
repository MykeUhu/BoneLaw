// Copyright by MykeUhu

using UnrealBuildTool;
using System.Collections.Generic;

public class BoneLawEditorTarget : TargetRules
{
	public BoneLawEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;

		ExtraModuleNames.AddRange( new string[] { "BoneLaw" } );
	}
}
