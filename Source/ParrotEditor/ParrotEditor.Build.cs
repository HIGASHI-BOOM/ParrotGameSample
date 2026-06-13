// 2024 Secret Dimension, Inc.

using UnrealBuildTool;

public class ParrotEditor : ModuleRules
{
    public ParrotEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
			"CommonInput",
			"CommonUI",
			"Core",
			"CoreUObject",
			"Engine",
			"EnhancedInput",
			"GameplayTags",
			"InputCore",
			"UMG",
			"Parrot"
		});

		PrivateDependencyModuleNames.Add("Blutility");
    }
}
