// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/WarriorCharacter.h"
#include "BlueBattleGameplayTags.h"
#include "BlueBattleLogChannels.h"
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
#include "Mass/BlueBattleMassAgentComponent.h"
#include "MVVMSubsystem.h"
#include "UI/ViewModels/BlueBattleAttributeViewModel.h"
#include "UObject/UObjectHash.h"
#include "View/MVVMView.h"

AWarriorCharacter::AWarriorCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// AttributeComponent / ComboComponent 由蓝图组件树提供（AttributeSystem / ComboSystem），BeginPlay 时按类查找
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

	PawnExtComponent = CreateDefaultSubobject<UBlueBattlePawnExtensionComponent>(TEXT("PawnExtensionComponent"));
	HeroComponent = CreateDefaultSubobject<UBlueBattleHeroComponent>(TEXT("HeroComponent"));
	MassAgentComponent = CreateDefaultSubobject<UBlueBattleMassAgentComponent>(TEXT("MassAgentComponent"));
}

#if WITH_EDITOR
void AWarriorCharacter::PostLoad()
{
	Super::PostLoad();

	// 旧版本曾用 CreateDefaultSubobject 创建这两个组件，已保存的蓝图 CDO 下仍残留同名子对象，重新保存蓝图后即消失
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}
	TArray<UObject*> Subobjects;
	GetObjectsWithOuter(this, Subobjects, false);
	for (UObject* Subobject : Subobjects)
	{
		const bool bStaleAttribute = Subobject->IsA<UAttributeComponent>() && Subobject->GetFName() == TEXT("AttributeComponent");
		const bool bStaleCombo = Subobject->IsA<UComboComponent>() && Subobject->GetFName() == TEXT("ComboComponent");
		if (bStaleAttribute || bStaleCombo)
		{
			UE_LOG(LogBlueBattle, Log, TEXT("%s: removing stale subobject %s"), *GetPathName(), *Subobject->GetName());
			Subobject->ClearFlags(RF_Public | RF_Standalone | RF_DefaultSubObject | RF_ArchetypeObject);
			Subobject->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_NonTransactional);
			Subobject->MarkAsGarbage();
		}
	}
}
#endif

void AWarriorCharacter::BeginPlay()
{
	// 先于蓝图 BeginPlay 查找组件并建好 ViewModel，蓝图里创建的 Widget 也能拿到
	AttributeComponent = FindComponentByClass<UAttributeComponent>();
	ComboComponent = FindComponentByClass<UComboComponent>();

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
		LightAttack();
	}
	else if (InputTag == BlueBattleGameplayTags::InputTag_Action_HeavyAttack)
	{
		HeavyAttack();
	}
	else if (InputTag == BlueBattleGameplayTags::InputTag_Action_Dodge)
	{
		Dodge();
	}
	else if (InputTag == BlueBattleGameplayTags::InputTag_Action_Burst)
	{
		Burst();
	}
	else if (InputTag == BlueBattleGameplayTags::InputTag_ConsumeMP)
	{
		ConsumeMP();
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
	if (ComboComponent)
	{
		ComboComponent->PerformLightAttack();
	}
}

void AWarriorCharacter::HeavyAttack()
{
	if (ComboComponent)
	{
		ComboComponent->PerformHeavyAttack();
	}
}

void AWarriorCharacter::Dodge()
{
	if (ComboComponent)
	{
		ComboComponent->PerformDodge();
	}
}

void AWarriorCharacter::Burst()
{
	if (ComboComponent)
	{
		ComboComponent->ActivateBurst();
	}
}

void AWarriorCharacter::ConsumeMP()
{
	if (AttributeComponent)
	{
		AttributeComponent->ConsumeMP(ConsumeMPCost);
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

void AWarriorCharacter::HandleDeath(AActor* Killer)
{
	if (ABlueBattleGameMode* GameMode = GetWorld()->GetAuthGameMode<ABlueBattleGameMode>())
	{
		GameMode->NotifyWarriorDied(GetPlayerIndex());
	}
}
