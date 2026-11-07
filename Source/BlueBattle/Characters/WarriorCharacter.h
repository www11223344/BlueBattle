// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularCharacter.h"
#include "Combat/BlueBattleTypes.h"
#include "WarriorCharacter.generated.h"

class UAttributeComponent;
class UCombatComponent;
class UComboComponent;
class UBlueBattleAttributeViewModel;
class UBlueBattleHeroComponent;
class UBlueBattlePawnExtensionComponent;
class UBlueBattleMassAgentComponent;
struct FGameplayTag;

/**
 * 格斗角色基类：组合 Mass 绑定组件与属性、战斗、连招组件，作为输入入口。
 * 表现回调都在组件上（ComboComponent::OnPlay*Montage、AttributeComponent::On*Feedback），本类不承载表现逻辑。
 */
UCLASS()
class BLUEBATTLE_API AWarriorCharacter : public AModularCharacter
{
	GENERATED_BODY()

public:
	AWarriorCharacter();

	// ---- 输入入口（供 PlayerController / 蓝图输入事件调用）----

	/** 轻击 */
	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat")
	void LightAttack();

	/** 重击 */
	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat")
	void HeavyAttack();

	/** 闪避 */
	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat")
	void Dodge();

	/** 爆气 / EX */
	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat")
	void Burst();

	/** 耗魔：按住期间每次触发扣除 ConsumeMPCost */
	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat")
	void ConsumeMP();

	/** HeroComponent 收到动作输入后调用，转到对应的动作入口 */
	void HandleInputAction(const FGameplayTag& InputTag);

	/** 按移动输入（X = 右，Y = 前）旋转闪避方向组件，取绝对值较大的轴 */
	void UpdateDodgeDirection(const FVector2D& MoveInput);

	// ---- 供 AnimBP 读取的状态 ----

	UFUNCTION(BlueprintPure, Category = "Warrior|State")
	bool IsAttacking() const;

	UFUNCTION(BlueprintPure, Category = "Warrior|State")
	int32 GetComboStep() const;

	UFUNCTION(BlueprintPure, Category = "Warrior|State")
	bool IsAlive() const;

	// ---- 组件访问 ----

	UFUNCTION(BlueprintPure, Category = "Warrior|Components")
	UAttributeComponent* GetAttributeComponent() const { return AttributeComponent; }

	UFUNCTION(BlueprintPure, Category = "Warrior|Components")
	UCombatComponent* GetCombatComponent() const { return CombatComponent; }

	UFUNCTION(BlueprintPure, Category = "Warrior|Components")
	UComboComponent* GetComboComponent() const { return ComboComponent; }

	UFUNCTION(BlueprintPure, Category = "Warrior|Components")
	UBlueBattleMassAgentComponent* GetMassAgentComponent() const { return MassAgentComponent; }

	/** 本角色的属性 ViewModel（头顶血条等 WidgetComponent 的 MVVM View 共享） */
	UFUNCTION(BlueprintPure, Category = "Warrior|UI")
	UBlueBattleAttributeViewModel* GetAttributeViewModel() const { return AttributeViewModel; }

	/** 把属性 ViewModel 注入到本角色所有 WidgetComponent 的 Widget（蓝图替换 Widget 后可再次调用） */
	UFUNCTION(BlueprintCallable, Category = "Warrior|UI")
	void BindAttributeViewModelToWidgetComponents();

	/** 控制本角色的本地玩家序号（0 = P1，1 = P2）；无本地玩家控制时按场景中角色顺序 */
	UFUNCTION(BlueprintPure, Category = "Warrior")
	int32 GetPlayerIndex() const;

#if WITH_EDITOR
	virtual void PostLoad() override;
#endif

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Warrior|Components")
	TObjectPtr<UBlueBattlePawnExtensionComponent> PawnExtComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Warrior|Components")
	TObjectPtr<UBlueBattleHeroComponent> HeroComponent;

	/** 绑定 MassEntity（数据层），属性/连招 Fragment 经 Translator 同步到本 Actor 的组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Warrior|Components")
	TObjectPtr<UBlueBattleMassAgentComponent> MassAgentComponent;

	/** 闪避方向组件（蓝图组件树中的 SceneComponent）的名字 */
	UPROPERTY(EditDefaultsOnly, Category = "Warrior|Combat")
	FName DodgeDirectionComponentName = TEXT("闪避方向");

	/** 属性组件（蓝图组件树中的 AttributeSystem，BeginPlay 时查找） */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Warrior|Components")
	TObjectPtr<UAttributeComponent> AttributeComponent;

	/** 战斗组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Warrior|Components")
	TObjectPtr<UCombatComponent> CombatComponent;

	/** 连招组件（蓝图组件树中的 ComboSystem，BeginPlay 时查找） */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Warrior|Components")
	TObjectPtr<UComboComponent> ComboComponent;

	UPROPERTY(Transient)
	TObjectPtr<UBlueBattleAttributeViewModel> AttributeViewModel;

	/** 耗魔输入每次触发扣除的 MP */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat")
	float ConsumeMPCost = 20.f;

private:
	/** 死亡内部回调 → 通知 GameMode 判负（死亡表现由属性组件的 OnDeathFeedback 负责） */
	UFUNCTION()
	void HandleDeath(AActor* Killer);

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> DodgeDirectionComponent;
};
