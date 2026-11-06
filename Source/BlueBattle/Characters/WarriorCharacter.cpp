// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/WarriorCharacter.h"
#include "BlueBattleGameplayTags.h"
#include "Blueprint/UserWidget.h"
#include "Characters/BlueBattleHeroComponent.h"
#include "Characters/BlueBattlePawnExtensionComponent.h"
#include "Combat/AttributeComponent.h"
#include "Combat/CombatComponent.h"
#include "Combat/ComboComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/BlueBattleGameMode.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "MVVMSubsystem.h"
#include "UI/ViewModels/BlueBattleAttributeViewModel.h"
#include "View/MVVMView.h"

AWarriorCharacter::AWarriorCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// CombatComponent 为 C++ 新增的命中检测组件（蓝图无对应），由本类创建。
	// AttributeComponent / ComboComponent 复用蓝图已有的 AttributeSystem / ComboSystem
	// （它们已 Reparent 到 C++ 基类），在 BeginPlay 中按类查找，避免重复挂载两套组件。
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

	PawnExtComponent = CreateDefaultSubobject<UBlueBattlePawnExtensionComponent>(TEXT("PawnExtensionComponent"));
	HeroComponent = CreateDefaultSubobject<UBlueBattleHeroComponent>(TEXT("HeroComponent"));
}

namespace
{
	/** BP_Warrior 资产里残留旧版原生子对象，与蓝图组件树里的 AttributeSystem/ComboSystem 同类；只保留蓝图组件 */
	template <typename TComponent>
	TComponent* FindBlueprintComponent(AActor* Actor)
	{
		TInlineComponentArray<TComponent*> Components(Actor);
		TComponent* Found = nullptr;
		for (TComponent* Component : Components)
		{
			if (Component->CreationMethod == EComponentCreationMethod::SimpleConstructionScript)
			{
				Found = Component;
				break;
			}
		}
		if (!Found)
		{
			return Components.Num() > 0 ? Components[0] : nullptr;
		}
		for (TComponent* Component : Components)
		{
			if (Component != Found)
			{
				Component->DestroyComponent();
			}
		}
		return Found;
	}
}

void AWarriorCharacter::BeginPlay()
{
	// 先于蓝图 BeginPlay 查找组件并建好 ViewModel，蓝图里创建的 Widget 也能拿到
	AttributeComponent = FindBlueprintComponent<UAttributeComponent>(this);
	ComboComponent = FindBlueprintComponent<UComboComponent>(this);

	TInlineComponentArray<USceneComponent*> SceneComponents(this);
	for (USceneComponent* SceneComponent : SceneComponents)
	{
		if (SceneComponent->GetFName() == DodgeDirectionComponentName)
		{
			DodgeDirectionComponent = SceneComponent;
			break;
		}
	}

	AttributeViewModel = NewObject<UBlueBattleAttributeViewModel>(this);
	AttributeViewModel->SetAttributeComponent(AttributeComponent);

	Super::BeginPlay();

	if (ComboComponent)
	{
		ComboComponent->OnComboStepChanged.AddDynamic(this, &AWarriorCharacter::HandleComboStepChanged);
	}
	if (CombatComponent)
	{
		CombatComponent->OnHitTarget.AddDynamic(this, &AWarriorCharacter::HandleHitTarget);
	}
	if (AttributeComponent)
	{
		AttributeComponent->OnDeath.AddDynamic(this, &AWarriorCharacter::HandleDeath);
	}

	BindAttributeViewModelToWidgetComponents();
}

void AWarriorCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	PawnExtComponent->HandleControllerChanged();
}

void AWarriorCharacter::UnPossessed()
{
	Super::UnPossessed();
	PawnExtComponent->HandleControllerChanged();
}

void AWarriorCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();
	PawnExtComponent->HandleControllerChanged();
}

void AWarriorCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	PawnExtComponent->HandlePlayerStateReplicated();
}

void AWarriorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PawnExtComponent->SetupPlayerInputComponent();
}

void AWarriorCharacter::HandleInputAction(const FGameplayTag& InputTag)
{
	if (InputTag == BlueBattleGameplayTags::InputTag_Action_LightAttack)
	{
		InputLightAttack();
	}
	else if (InputTag == BlueBattleGameplayTags::InputTag_Action_HeavyAttack)
	{
		InputHeavyAttack();
	}
	else if (InputTag == BlueBattleGameplayTags::InputTag_Action_Dodge)
	{
		InputDodge();
	}
	else if (InputTag == BlueBattleGameplayTags::InputTag_Action_Burst)
	{
		InputBurst();
	}
	else if (InputTag == BlueBattleGameplayTags::InputTag_ConsumeMP)
	{
		InputConsumeMP();
	}
}

void AWarriorCharacter::UpdateDodgeDirection(const FVector2D& MoveInput)
{
	if (!DodgeDirectionComponent)
	{
		return;
	}

	if (FMath::Abs(MoveInput.Y) > FMath::Abs(MoveInput.X))
	{
		const FRotator ActorRotation = GetActorRotation();
		DodgeDirectionComponent->SetWorldRotation(MoveInput.Y > 0.f ? ActorRotation : FRotator(0.f, ActorRotation.Yaw + 180.f, 0.f));
	}
	else if (MoveInput.X != 0.f)
	{
		const FRotator RightRotation = GetActorRightVector().Rotation();
		DodgeDirectionComponent->SetWorldRotation(MoveInput.X > 0.f ? RightRotation : FRotator(0.f, RightRotation.Yaw + 180.f, 0.f));
	}
}

void AWarriorCharacter::BindAttributeViewModelToWidgetComponents()
{
	if (!AttributeViewModel)
	{
		return;
	}

	TInlineComponentArray<UWidgetComponent*> WidgetComponents(this);
	for (UWidgetComponent* WidgetComponent : WidgetComponents)
	{
		if (!WidgetComponent->GetUserWidgetObject())
		{
			WidgetComponent->InitWidget();
		}

		if (UMVVMView* View = UMVVMSubsystem::GetViewFromUserWidget(WidgetComponent->GetUserWidgetObject()))
		{
			View->SetViewModelByClass(AttributeViewModel);
		}
	}
}

int32 AWarriorCharacter::GetPlayerIndex() const
{
	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		const UGameInstance* GameInstance = GetGameInstance();
		const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
		if (GameInstance && LocalPlayer)
		{
			const int32 Index = GameInstance->GetLocalPlayers().IndexOfByKey(LocalPlayer);
			if (Index != INDEX_NONE)
			{
				return Index;
			}
		}
	}

	int32 Index = 0;
	for (TActorIterator<AWarriorCharacter> It(GetWorld()); It; ++It, ++Index)
	{
		if (*It == this)
		{
			return Index;
		}
	}
	return INDEX_NONE;
}

void AWarriorCharacter::LightAttack()
{
	if (ComboComponent && IsAlive())
	{
		ComboComponent->PerformLightAttack();
	}
}

void AWarriorCharacter::HeavyAttack()
{
	if (ComboComponent && IsAlive())
	{
		ComboComponent->PerformHeavyAttack();
	}
}

void AWarriorCharacter::Dodge()
{
	if (!ComboComponent || !IsAlive())
	{
		return;
	}

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (Now - LastDodgeTime < DodgeCooldown)
	{
		return;
	}
	LastDodgeTime = Now;

	ComboComponent->PerformDodge();
	OnDodgePerformed();
}

void AWarriorCharacter::Burst()
{
	if (ComboComponent && IsAlive())
	{
		ComboComponent->ActivateBurst();
	}
}

bool AWarriorCharacter::IsAttacking() const
{
	return ComboComponent ? ComboComponent->IsAttacking() : false;
}

int32 AWarriorCharacter::GetComboStep() const
{
	return ComboComponent ? ComboComponent->GetComboCount() : 0;
}

bool AWarriorCharacter::IsAlive() const
{
	return AttributeComponent ? AttributeComponent->IsAlive() : true;
}

void AWarriorCharacter::HandleComboStepChanged(int32 ComboStep, EAttackType AttackType)
{
	// 转发到蓝图，由蓝图播放对应蒙太奇
	OnAttackPerformed(ComboStep, AttackType);
}

void AWarriorCharacter::HandleHitTarget(AActor* HitActor, float DamageDealt, const FHitResult& HitResult)
{
	// 通知被击中方播放受击表现
	if (AWarriorCharacter* HitWarrior = Cast<AWarriorCharacter>(HitActor))
	{
		HitWarrior->OnHitReaction(DamageDealt, this);
	}
}

void AWarriorCharacter::HandleDeath(AActor* Killer)
{
	OnDeathFeedback(Killer);

	if (ABlueBattleGameMode* GameMode = GetWorld()->GetAuthGameMode<ABlueBattleGameMode>())
	{
		GameMode->NotifyWarriorDied(GetPlayerIndex());
	}
}
