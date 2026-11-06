// Fill out your copyright notice in the Description page of Project Settings.

#include "BlueBattleGameplayTags.h"

namespace BlueBattleGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Game, "UI.Layer.Game", "HUD 层（局内常驻界面）");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_GameMenu, "UI.Layer.GameMenu", "局内菜单层");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Menu, "UI.Layer.Menu", "菜单层（主菜单、暂停、结算）");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Modal, "UI.Layer.Modal", "模态层（确认框）");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_Spawned, "InitState.Spawned", "Actor/组件已生成");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataAvailable, "InitState.DataAvailable", "初始化所需数据已就绪");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataInitialized, "InitState.DataInitialized", "数据已完成初始化");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_GameplayReady, "InitState.GameplayReady", "可以进入游戏逻辑");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "移动");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look, "InputTag.Look", "转动视角");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_ConsumeMP, "InputTag.ConsumeMP", "耗魔（按住持续触发）");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Action_LightAttack, "InputTag.Action.LightAttack", "轻击");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Action_HeavyAttack, "InputTag.Action.HeavyAttack", "重击");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Action_Dodge, "InputTag.Action.Dodge", "闪避");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Action_Burst, "InputTag.Action.Burst", "爆气");
}
