// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Circuit : ModuleRules
{
	public Circuit(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{ 
			"Core",
			"CoreUObject",
			"Engine", 
			"InputCore",
			"EnhancedInput",
			"ChaosVehicles",
			"PhysicsCore",
			"UMG",
            "Niagara"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
		{
			"SlateCore"
		});
    }
}
