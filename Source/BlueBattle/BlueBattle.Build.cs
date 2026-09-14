// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class BlueBattle : ModuleRules
{
	public BlueBattle(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		// 允许以模块根为基准的相对包含，例如 "Combat/AttributeComponent.h"
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

		PrivateDependencyModuleNames.AddRange(new string[] { "UMG", "Slate", "SlateCore" });

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"ModelContextProtocol",
				"ModelContextProtocolEngine"
			});
		}

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
