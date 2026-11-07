// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class AWarriorCharacter;
class UWorld;

/**
 * 局内调试指令：修改都经 UBlueBattleUnitSubsystem 写入 Mass 数据层。
 * 非 Shipping 下同时注册为控制台命令（BB.*），调试窗口也通过控制台命令调用这里。
 * Side 参数：0 = P1，1 = P2，-1 = 双方。
 */
namespace BlueBattleDebug
{
	BLUEBATTLE_API TArray<AWarriorCharacter*> FindWarriors(UWorld* World, int32 Side);

	BLUEBATTLE_API FString GetSideName(int32 Side);

	/** 返回实际生效的角色数（已死亡或尚未绑定实体的角色不计） */
	BLUEBATTLE_API int32 SetHP(UWorld* World, int32 Side, float HP);

	/** Damage 小于 0 时恢复按招式表结算；返回实际生效的角色数 */
	BLUEBATTLE_API int32 SetDamage(UWorld* World, int32 Side, float Damage);
}
