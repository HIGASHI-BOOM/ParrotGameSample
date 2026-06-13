// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Parrot : ModuleRules
{
	public Parrot(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
        {
            "CommonInput",
            "CommonUI",
            "Core", 
			"CoreUObject", 
			"Engine",
			"EngineSettings",
            "EnhancedInput",
            "GameplayTags",
            "InputCore",
            "UMG",
			"CommonLoadingScreen"
        });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncommented since we are using Slate UI functions
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
