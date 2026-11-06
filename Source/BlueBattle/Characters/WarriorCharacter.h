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
struct FGameplayTag;

/**
 * 格斗角色基类：整合属性、战斗、连招三大 C++ 组件。
 * BP_Warrior 应 Reparent 到本类，蓝图仅保留动画/特效/资源引用等表现层。
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

	/** HeroComponent 收到动作输入后调用，按输入标签分发到下面的 Input* 蓝图事件 */
	void HandleInputAction(const FGameplayTag& InputTag);

	/** 按移动输入（X = 右，Y = 前）旋转闪避方向组件，取绝对值较大的轴 */
	void UpdateDodgeDirection(const FVector2D& MoveInput);

	UFUNCTION(BlueprintImplementableEvent, Category = "Warrior|Input")
	void InputLightAttack();

	UFUNCTION(BlueprintImplementableEvent, Category = "Warrior|Input")
	void InputHeavyAttack();

	UFUNCTION(BlueprintImplementableEvent, Category = "Warrior|Input")
	void InputDodge();

	UFUNCTION(BlueprintImplementableEvent, Category = "Warrior|Input")
	void InputBurst();

	/** 按住期间每帧触发 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Warrior|Input")
	void InputConsumeMP();

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

	/** 本角色的属性 ViewModel（头顶血条等 WidgetComponent 的 MVVM View 共享） */
	UFUNCTION(BlueprintPure, Category = "Warrior|UI")
	UBlueBattleAttributeViewModel* GetAttributeViewModel() const { return AttributeViewModel; }

	/** 把属性 ViewModel 注入到本角色所有 WidgetComponent 的 Widget（蓝图替换 Widget 后可再次调用） */
	UFUNCTION(BlueprintCallable, Category = "Warrior|UI")
	void BindAttributeViewModelToWidgetComponents();

	/** 控制本角色的本地玩家序号（0 = P1，1 = P2）；无本地玩家控制时按场景中角色顺序 */
	UFUNCTION(BlueprintPure, Category = "Warrior")
	int32 GetPlayerIndex() const;

	// ---- 表现层回调（蓝图实现：播放蒙太奇/特效/音效）----

	/** 请求播放某一段攻击（蓝图负责根据段数选择并播放蒙太奇） */
	UFUNCTION(BlueprintImplementableEvent, Category = "Warrior|Feedback")
	void OnAttackPerformed(int32 ComboStep, EAttackType AttackType);

	/** 请求播放闪避表现 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Warrior|Feedback")
	void OnDodgePerformed();

	/** 受击表现（蓝图播放受击动画/特效） */
	UFUNCTION(BlueprintImplementableEvent, Category = "Warrior|Feedback")
	void OnHitReaction(float DamageDealt, AActor* DamageCauser);

	/** 死亡表现（蓝图播放死亡蒙太奇/结算） */
	UFUNCTION(BlueprintImplementableEvent, Category = "Warrior|Feedback")
	void OnDeathFeedback(AActor* Killer);

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

	/** 闪避方向组件（蓝图组件树中的 SceneComponent）的名字 */
	UPROPERTY(EditDefaultsOnly, Category = "Warrior|Combat")
	FName DodgeDirectionComponentName = TEXT("闪避方向");

	/** 属性组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Warrior|Components")
	TObjectPtr<UAttributeComponent> AttributeComponent;

	/** 战斗组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Warrior|Components")
	TObjectPtr<UCombatComponent> CombatComponent;

	/** 连招组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Warrior|Components")
	TObjectPtr<UComboComponent> ComboComponent;

	UPROPERTY(Transient)
	TObjectPtr<UBlueBattleAttributeViewModel> AttributeViewModel;

	/** 闪避冷却（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior|Combat")
	float DodgeCooldown = 0.6f;

private:
	/** 连招段数变化时的内部回调 → 转发到蓝图 OnAttackPerformed */
	UFUNCTION()
	void HandleComboStepChanged(int32 ComboStep, EAttackType AttackType);

	/** 命中目标时的内部回调 */
	UFUNCTION()
	void HandleHitTarget(AActor* HitActor, float DamageDealt, const FHitResult& HitResult);

	/** 死亡内部回调 → 转发到蓝图 OnDeathFeedback */
	UFUNCTION()
	void HandleDeath(AActor* Killer);

	float LastDodgeTime = -100.f;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> DodgeDirectionComponent;
};
