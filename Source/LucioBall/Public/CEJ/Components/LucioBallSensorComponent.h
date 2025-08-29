#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LucioBallSensorComponent.generated.h"

class ABouncyBall;

UCLASS(ClassGroup=(Lucio), meta=(BlueprintSpawnableComponent))
class LUCIOBALL_API ULucioBallSensorComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	ULucioBallSensorComponent();

	UPROPERTY(EditAnywhere, Category="Sense|Tags")
	FName BallTag = TEXT("BouncyBall");          
	UPROPERTY(EditAnywhere, Category="Sense|Tags")
	FName GoalTag = TEXT("SoccerGoal");           

	UFUNCTION(BlueprintCallable, Category="Sense")
	void EnsureTargets(int32 DesiredGoalYSign);  

	UFUNCTION(BlueprintCallable, Category="Sense")
	FVector GetBallLocation() const;

	UFUNCTION(BlueprintCallable, Category="Sense")
	FVector GetBallLandLocation() const;          // ABouncyBall::GetLandLocation()

	UFUNCTION(BlueprintCallable, Category="Sense")
	bool IsBallNearby(float Threshold) const;

	UPROPERTY(VisibleAnywhere, Category="Sense")
	TWeakObjectPtr<ABouncyBall> Ball;

	UPROPERTY(VisibleAnywhere, Category="Sense")
	TWeakObjectPtr<AActor> Goal; // y부호로 고른 득점 대상 골대

private:
	AActor* ResolveGoalByY(int32 YSign) const;
};
