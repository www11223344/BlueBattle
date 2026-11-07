// Fill out your copyright notice in the Description page of Project Settings.

#include "Debug/BlueBattleDebugCommands.h"
#include "Characters/WarriorCharacter.h"
#include "Combat/AttributeComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "Mass/BlueBattleUnitSubsystem.h"

namespace
{
	template <typename TFunc>
	int32 ForEachUnitEntity(UWorld* World, int32 Side, TFunc&& Func)
	{
		UBlueBattleUnitSubsystem* UnitSubsystem = World ? World->GetSubsystem<UBlueBattleUnitSubsystem>() : nullptr;
		if (!UnitSubsystem)
		{
			return 0;
		}

		int32 NumApplied = 0;
		for (AWarriorCharacter* Warrior : BlueBattleDebug::FindWarriors(World, Side))
		{
			const UAttributeComponent* Attributes = Warrior->GetAttributeComponent();
			if (Attributes && Attributes->GetMassEntity().IsSet() && Func(*UnitSubsystem, Attributes->GetMassEntity()))
			{
				++NumApplied;
			}
		}
		return NumApplied;
	}
}

namespace BlueBattleDebug
{
	TArray<AWarriorCharacter*> FindWarriors(UWorld* World, int32 Side)
	{
		TArray<AWarriorCharacter*> Warriors;
		if (!World)
		{
			return Warriors;
		}
		for (TActorIterator<AWarriorCharacter> It(World); It; ++It)
		{
			if (Side < 0 || It->GetPlayerIndex() == Side)
			{
				Warriors.Add(*It);
			}
		}
		return Warriors;
	}

	FString GetSideName(int32 Side)
	{
		return Side < 0 ? TEXT("双方") : FString::Printf(TEXT("P%d"), Side + 1);
	}

	int32 SetHP(UWorld* World, int32 Side, float HP)
	{
		return ForEachUnitEntity(World, Side, [HP](UBlueBattleUnitSubsystem& UnitSubsystem, FMassEntityHandle Entity)
		{
			return UnitSubsystem.SetHP(Entity, HP);
		});
	}

	int32 SetDamage(UWorld* World, int32 Side, float Damage)
	{
		return ForEachUnitEntity(World, Side, [Damage](UBlueBattleUnitSubsystem& UnitSubsystem, FMassEntityHandle Entity)
		{
			return UnitSubsystem.SetDamageOverride(Entity, Damage);
		});
	}
}

#if !UE_BUILD_SHIPPING
namespace
{
	bool ParseSideAndValue(const TArray<FString>& Args, const TCHAR* Usage, FOutputDevice& Ar, int32& OutSide, float& OutValue)
	{
		if (Args.Num() < 2 || !Args[0].IsNumeric() || !Args[1].IsNumeric())
		{
			Ar.Logf(TEXT("用法：%s"), Usage);
			return false;
		}
		OutSide = FCString::Atoi(*Args[0]);
		OutValue = FCString::Atof(*Args[1]);
		return true;
	}

	const TCHAR* SetHPUsage = TEXT("BB.SetHP <Side: 0=P1 1=P2 -1=双方> <生命值>");
	const TCHAR* SetDamageUsage = TEXT("BB.SetDamage <Side: 0=P1 1=P2 -1=双方> <每次命中伤害，-1 恢复招式表>");

	FAutoConsoleCommandWithWorldArgsAndOutputDevice SetHPCommand(
		TEXT("BB.SetHP"),
		SetHPUsage,
		FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World, FOutputDevice& Ar)
		{
			int32 Side = 0;
			float HP = 0.f;
			if (ParseSideAndValue(Args, SetHPUsage, Ar, Side, HP))
			{
				const int32 NumApplied = BlueBattleDebug::SetHP(World, Side, HP);
				Ar.Logf(TEXT("%s 生命值设为 %g（%d 个角色生效）"), *BlueBattleDebug::GetSideName(Side), HP, NumApplied);
			}
		}));

	FAutoConsoleCommandWithWorldArgsAndOutputDevice SetDamageCommand(
		TEXT("BB.SetDamage"),
		SetDamageUsage,
		FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World, FOutputDevice& Ar)
		{
			int32 Side = 0;
			float Damage = 0.f;
			if (ParseSideAndValue(Args, SetDamageUsage, Ar, Side, Damage))
			{
				const int32 NumApplied = BlueBattleDebug::SetDamage(World, Side, Damage);
				const FString DamageText = Damage < 0.f ? FString(TEXT("招式表")) : FString::Printf(TEXT("%g"), Damage);
				Ar.Logf(TEXT("%s 每次命中伤害设为 %s（%d 个角色生效）"), *BlueBattleDebug::GetSideName(Side), *DamageText, NumApplied);
			}
		}));
}
#endif
