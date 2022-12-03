// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Blink : ModuleRules
{
	public Blink(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"BlinkOpenCV",
			"Asclepius"
		});
	}
}
