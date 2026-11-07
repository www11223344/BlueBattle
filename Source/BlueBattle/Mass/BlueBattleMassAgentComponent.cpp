// Fill out your copyright notice in the Description page of Project Settings.

#include "Mass/BlueBattleMassAgentComponent.h"
#include "BlueBattleLogChannels.h"
#include "Mass/BlueBattleMassTraits.h"

UBlueBattleMassAgentComponent::UBlueBattleMassAgentComponent()
{
	UBlueBattleUnitTrait* UnitTrait = CreateDefaultSubobject<UBlueBattleUnitTrait>(TEXT("UnitTrait"));
	EntityConfig.AddTrait(*UnitTrait);

	UBlueBattleAttributeSyncTrait* AttributeSyncTrait = CreateDefaultSubobject<UBlueBattleAttributeSyncTrait>(TEXT("AttributeSyncTrait"));
	EntityConfig.AddTrait(*AttributeSyncTrait);

	UBlueBattleComboSyncTrait* ComboSyncTrait = CreateDefaultSubobject<UBlueBattleComboSyncTrait>(TEXT("ComboSyncTrait"));
	EntityConfig.AddTrait(*ComboSyncTrait);
}

void UBlueBattleMassAgentComponent::OnRegister()
{
	// 蓝图里序列化过的 EntityConfig 会覆盖构造函数装配的 Trait，注册到 Mass 前补齐必需的 Trait
	EnsureTrait(UBlueBattleUnitTrait::StaticClass());
	EnsureTrait(UBlueBattleAttributeSyncTrait::StaticClass());
	EnsureTrait(UBlueBattleComboSyncTrait::StaticClass());

	Super::OnRegister();
}

void UBlueBattleMassAgentComponent::EnsureTrait(TSubclassOf<UMassEntityTraitBase> TraitClass)
{
	if (!EntityConfig.FindTrait(TraitClass))
	{
		EntityConfig.AddTrait(*NewObject<UMassEntityTraitBase>(this, TraitClass));
	}
}

void UBlueBattleMassAgentComponent::SetEntityHandleInternal(const FMassEntityHandle NewHandle)
{
	Super::SetEntityHandleInternal(NewHandle);

	UE_LOG(LogBlueBattle, Log, TEXT("%s bound to Mass entity %s"), *GetNameSafe(GetOwner()), *NewHandle.DebugGetDescription());
	OnEntityChanged.Broadcast(this);
}
