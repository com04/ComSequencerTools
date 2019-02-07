// Copyright 2018-2019 com04, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class ComSequencerTools : ModuleRules
{
	public ComSequencerTools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);


        string EnginePath = Path.GetFullPath(Target.RelativeEnginePath);
        PrivateIncludePaths.Add(EnginePath + "Source/Editor/Sequencer/Public");
        PrivateIncludePaths.Add(EnginePath + "Source/Editor/MovieSceneTools/Public");
        PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Sequencer",
				"MovieSceneTools",
				"MovieSceneTracks",
				"MovieScene",
				"UnrealEd",
				"EditorStyle",
				"Projects",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
