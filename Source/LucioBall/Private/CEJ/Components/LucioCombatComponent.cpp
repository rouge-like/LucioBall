// Fill out your copyright notice in the Description page of Project Settings.


#include "CEJ/Components/LucioCombatComponent.h"
#include "OSC/BouncyBall.h"

ULucioCombatComponent::ULucioCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULucioCombatComponent::KickTowardsGoal(ABouncyBall* Ball, const AActor* Goal, AActor* Attacker) const
{
	if (!Ball || !Goal) return;
	const FVector BallLoc = Ball->GetActorLocation();
	FVector Dir = (Goal->GetActorLocation() - BallLoc).GetSafeNormal();
	Dir.Z = 0.3f; // 약간 상승각
	Dir.Normalize();
	Ball->BouncyBallAddImpulse(Dir * AttackImpulse, const_cast<AActor*>(Attacker));
}

void ULucioCombatComponent::ClearUpAndOut(ABouncyBall* Ball, const AActor* Goal, AActor* Attacker) const
{
	if (!Ball) return;
	const FVector BallLoc = Ball->GetActorLocation();

	if (Goal)
	{
		FVector Dir = (Goal->GetActorLocation() - BallLoc).GetSafeNormal();
		Dir = FVector(Dir.X, Dir.Y, 1.f).GetSafeNormal();
		Ball->BouncyBallAddImpulse(Dir * (AttackImpulse * 0.9f), const_cast<AActor*>(Attacker));
	}
	else
	{
		Ball->BouncyBallAddImpulse(FVector(0,0,300.f), const_cast<AActor*>(Attacker));
	}
}

