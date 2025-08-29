// Fill out your copyright notice in the Description page of Project Settings.

#include "CEJ/Components/LucioMoveAssistComponent.h"
#include "AIController.h"
#include "CEJ/Components/LucioSkillComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ULucioMoveAssistComponent::ULucioMoveAssistComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULucioMoveAssistComponent::OptimizeChaseAndSpeed(
	APawn* Pawn, UCharacterMovementComponent* Move, ULucioSkillComponent* Skill,
	const FVector& MyLoc, const FVector& BallLandLoc, int32 TeamGoalYSign)
{
	if (!Pawn || !Move || !Skill) return;

	const float Dist = FVector::Dist(MyLoc, BallLandLoc);
	const float LandY = BallLandLoc.Y;

	//내 득점방향 쪽에 있고, 거리가 멀면 부스트
	const bool bSameSide = (TeamGoalYSign < 0) ? (LandY < 0.f) : (LandY > 0.f);
	const bool bEmergency = (Dist > 400.f) || (bSameSide && Dist > 300.f);

	if (bEmergency) Skill->TryUse();

	// 최종 속도 적용
	Skill->ApplyTo(Move);
}

void ULucioMoveAssistComponent::MoveToWithMaxSpeed(AAIController* AI, APawn* Pawn, const FVector& Location) const
{
	if (!AI || !Pawn) return;

	FAIMoveRequest Req;
	Req.SetGoalLocation(Location);
	Req.SetUsePathfinding(true);
	Req.SetAllowPartialPath(true);
	Req.SetAcceptanceRadius(AcceptanceRadius);
	AI->MoveTo(Req);

	const FVector Dir = (Location - Pawn->GetActorLocation()).GetSafeNormal();
	Pawn->AddMovementInput(Dir, 1.f);
}

