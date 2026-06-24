// Copyright 2024 Locomotion System. All Rights Reserved.

using UnrealBuildTool;

public class GameAnimationSample : ModuleRules
{
	public GameAnimationSample(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		// ── Include: 允许 #include "Character/Core/..." 等相对路径 ──
		PublicIncludePaths.Add(ModuleDirectory);
	
		// ── Core Dependencies ──
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayTags",
		});
		
		// ── Animation Dependencies ──
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"PoseSearch",
			"AnimationWarpingRuntime",
			"AnimationLocomotionLibraryRuntime",
			"MotionWarping",
			"Chooser",
		});
		
		// ── Movement ──
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Mover",
		});
		
		// ── Network ──
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"NetworkPrediction",
		});
		
		// ── Editor-only（暂不需要，后续越障编辑器工具时启用）──
		// if (Target.bBuildEditor)
		// {
		// 	PrivateDependencyModuleNames.Add("EditorScriptingUtilities");
		// }
	}
}
