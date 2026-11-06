// Fill out your copyright notice in the Description page of Project Settings.

#include "BlueBattleGameplayTags.h"

namespace BlueBattleGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Game, "UI.Layer.Game", "HUD 层（局内常驻界面）");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_GameMenu, "UI.Layer.GameMenu", "局内菜单层");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Menu, "UI.Layer.Menu", "菜单层（主菜单、暂停、结算）");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Modal, "UI.Layer.Modal", "模态层（确认框）");
}
