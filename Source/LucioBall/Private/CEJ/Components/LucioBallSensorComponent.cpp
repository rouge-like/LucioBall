
#include "CEJ/Components/LucioBallSensorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "OSC/BouncyBall.h"

ULucioBallSensorComponent::ULucioBallSensorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULucioBallSensorComponent::EnsureTargets(int32 DesiredGoalYSign)
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (!Ball.IsValid())
	{
		// 1) 태그로
		TArray<AActor*> Found;
		if (!BallTag.IsNone())
		{
			UGameplayStatics::GetAllActorsWithTag(World, BallTag, Found);
			if (Found.Num() > 0) Ball = Cast<ABouncyBall>(Found[0]);
		}
		// 2) 클래스 탐색
		if (!Ball.IsValid())
		{
			for (TActorIterator<ABouncyBall> It(World); It; ++It) { Ball = *It; break; }
		}
	}

	if (!Goal.IsValid())
	{
		Goal = ResolveGoalByY(DesiredGoalYSign);
	}
}

AActor* ULucioBallSensorComponent::ResolveGoalByY(int32 YSign) const
{
	if (YSign == 0) return nullptr;

	UWorld* World = GetWorld();
	if (!World) return nullptr;

	TArray<AActor*> Goals;
	UGameplayStatics::GetAllActorsWithTag(World, GoalTag, Goals);
	AActor* Best = nullptr;
	for (AActor* G : Goals)
	{
		if (!G) continue;
		const float Y = G->GetActorLocation().Y;
		if ((YSign < 0 && Y < 0.f) || (YSign > 0 && Y > 0.f)) { Best = G; break; }
	}
	return Best;
}

FVector ULucioBallSensorComponent::GetBallLocation() const
{
	return Ball.IsValid() ? Ball->GetActorLocation() : FVector::ZeroVector;
}

FVector ULucioBallSensorComponent::GetBallLandLocation() const
{
	return Ball.IsValid() ? Ball->GetLandLocation() : GetBallLocation();
}

bool ULucioBallSensorComponent::IsBallNearby(float Threshold) const
{
	const AActor* Owner = GetOwner();
	if (!Owner || !Ball.IsValid()) return false;
	return FVector::Dist(Owner->GetActorLocation(), Ball->GetActorLocation()) <= Threshold;
}
