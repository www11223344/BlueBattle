// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/CombatComponent.h"
#include "Combat/AttributeComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::BeginAttackWindow(float DamageMultiplier)
{
	bAttackWindowOpen = true;
	CurrentDamageMultiplier = DamageMultiplier;
	HitActorsThisWindow.Reset();
	SetComponentTickEnabled(true);
}

void UCombatComponent::EndAttackWindow()
{
	bAttackWindowOpen = false;
	SetComponentTickEnabled(false);
	HitActorsThisWindow.Reset();
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bAttackWindowOpen)
	{
		PerformHitDetection();
	}
}

void UCombatComponent::PerformHitDetection()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UWorld* World = Owner->GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Start = Owner->GetActorLocation() + Owner->GetActorForwardVector() * TraceRadius;
	const FVector End = Owner->GetActorLocation() + Owner->GetActorForwardVector() * (TraceRange + TraceRadius);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	TArray<FHitResult> Hits;
	const bool bHit = World->SweepMultiByChannel(
		Hits,
		Start,
		End,
		FQuat::Identity,
		TraceChannel,
		FCollisionShape::MakeSphere(TraceRadius),
		Params);

	if (bDrawDebug)
	{
		DrawDebugLine(World, Start, End, FColor::Red, false, 0.5f, 0, 1.5f);
		DrawDebugSphere(World, End, TraceRadius, 12, FColor::Yellow, false, 0.5f);
	}

	if (!bHit)
	{
		return;
	}

	// 拿到发起者攻击力（如果有 AttributeComponent）
	float AttackPower = 20.f;
	if (const UAttributeComponent* MyAttr = Owner->FindComponentByClass<UAttributeComponent>())
	{
		AttackPower = MyAttr->GetAttackPower();
	}

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == Owner || HitActorsThisWindow.Contains(HitActor))
		{
			continue;
		}

		HitActorsThisWindow.Add(HitActor);

		float DamageDealt = 0.f;
		if (UAttributeComponent* TargetAttr = HitActor->FindComponentByClass<UAttributeComponent>())
		{
			const float RawDamage = AttackPower * CurrentDamageMultiplier;
			DamageDealt = TargetAttr->ApplyDamage(RawDamage, Owner);
		}

		OnHitTarget.Broadcast(HitActor, DamageDealt, Hit);
	}
}
