// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LucioCombatComponent.generated.h"

class ABouncyBall;

UCLASS(ClassGroup=(Lucio), meta=(BlueprintSpawnableComponent))
class LUCIOBALL_API ULucioCombatComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	ULucioCombatComponent();

	UPROPERTY(EditAnywhere, Category="Combat")
	float AttackImpulse = 2000.f; // 기본 공격 힘

	// 골대 방향으로 약간 상승각(Z) 주어 킥
	void KickTowardsGoal(ABouncyBall* Ball, const AActor* Goal, AActor* Attacker) const;

	// 수비 클리어(상승 성분 강조)
	void ClearUpAndOut(ABouncyBall* Ball, const AActor* Goal, AActor* Attacker) const;
};

