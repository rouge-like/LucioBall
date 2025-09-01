// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LucioMoveAssistComponent.generated.h"

UCLASS(ClassGroup=(Lucio), meta=(BlueprintSpawnableComponent))
class LUCIOBALL_API ULucioMoveAssistComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	ULucioMoveAssistComponent();

	UPROPERTY(EditAnywhere, Category="Move")
	float AcceptanceRadius = 75.f;

	// 거리/상황에 따라 스킬을 켜고 속도를 적용
	void OptimizeChaseAndSpeed(class APawn* Pawn,
							   class UCharacterMovementComponent* Move,
							   class ULucioSkillComponent* Skill,
							   const FVector& MyLoc,
							   const FVector& BallLandLoc,
							   int32 TeamGoalYSign);

	// AIController 이동 + 직접 가속(보정)
	void MoveToWithMaxSpeed(class AAIController* AI, class APawn* Pawn, const FVector& Location) const;
};
