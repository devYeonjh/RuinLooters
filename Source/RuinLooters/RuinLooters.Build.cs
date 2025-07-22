// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RuinLooters : ModuleRules
{
	public RuinLooters(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(new string[] { "RuinLooters" });

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "AIModule", "UMG", "Slate", "SlateCore", "LevelSequence", "MovieScene", "Niagara", "ACERuntime", "ACECore", "A2FLocal", "Http", "Json", "JsonUtilities" });
	}
}
