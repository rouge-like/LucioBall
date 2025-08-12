// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LucioBallMode.generated.h"


class ABouncyBall;
UCLASS()
class LUCIOBALL_API ALucioBallMode : public AGameModeBase
{
	GENERATED_BODY()

private:
	int32 PlayerScore;
	int32 AIScore;

	FTimerHandle TimerHandle;

	UFUNCTION()
	void SpawnBouncyBall();
	
public:
	ALucioBallMode();

	UPROPERTY(EditDefaultsOnly, Category = "Actors")
	TSubclassOf<ABouncyBall> BouncyBall;

	UPROPERTY(EditDefaultsOnly, Category = "Actors")
	FVector BallSpawnPosition;
	
	UFUNCTION(BlueprintCallable, Category = "Components")
	void SetGoalScore(bool IsPlayerTeam, bool IsOwnGoal, FString AttackerName);

	virtual void BeginPlay() override;
};
