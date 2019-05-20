// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SurvivalGame : ModuleRules
{
	public SurvivalGame(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	    bEnableExceptions = true;
        MinFilesUsingPrecompiledHeaderOverride = 1;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AIModule", "UMG", "Slate", "SlateCore", "PFNNAnimation" });

	    if(Target.Type == TargetType.Editor)
	    {
	        PublicDependencyModuleNames.AddRange(new string[] { "PFNNAnimationEditor", "AnimGraph", "BlueprintGraph" });
	    }

    }

}

