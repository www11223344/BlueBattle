// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/CombatComponent.h"
#include "Combat/ComboComponent.h"
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

void UCombatComponent::BeginAttackWindow()
{
	bAttackWindowOpen = true;
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

	UComboComponent* Combo = Owner->FindComponentByClass<UComboComponent>();
	if (!Combo)
	{
		return;
	}

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == Owner || HitActorsThisWindow.Contains(HitActor))
		{
			continue;
		}

		HitActorsThisWindow.Add(HitActor);
		OnHitTarget.Broadcast(HitActor, Combo->ReportHit(HitActor), Hit);
	}
}
