// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class BlueBattleEditor : ModuleRules
{
	public BlueBattleEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"ApplicationCore",
			"InputCore",
			"Slate",
			"SlateCore",
			"UMG",
			"UMGEditor",
			"UnrealEd",
			"Kismet",
			"FieldNotification",
			"ModelViewViewModel",
			"ModelViewViewModelBlueprint",
			"ModelViewViewModelEditor",
			"BlueBattle",
		});
	}
}
