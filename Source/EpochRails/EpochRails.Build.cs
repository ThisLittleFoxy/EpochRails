// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class EpochRails : ModuleRules
{
	public EpochRails(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

		PrivateDependencyModuleNames.AddRange(new string[] { });

        PrivateIncludePaths.AddRange(
            new string[]
            {
                "EpochRails"
            }
        );
    }
}