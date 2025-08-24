// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LucioBallMode.generated.h"


class ABouncyBall;
class AGameHUD;
UCLASS()
class LUCIOBALL_API ALucioBallMode : public AGameModeBase
{
	GENERATED_BODY()

private:
	int32 PlayerScore;
	int32 AIScore;
	
	float CurrentTime;

	FTimerHandle TimerHandle;
	
	AGameHUD* HUD;

	bool bIsGameEnding = false;
	float SlowdownTimer = 0.0f;
	const float SlowdownDuration = 1.0f;
	
	UFUNCTION()
	void SpawnBouncyBall();
	
public:
	ALucioBallMode();
	
	UPROPERTY(EditDefaultsOnly)
	float Time = 240;
	
	UPROPERTY(EditDefaultsOnly, Category = "A_Mode")
	TSubclassOf<ABouncyBall> BouncyBall;

	UPROPERTY(EditDefaultsOnly, Category = "A_Mode")
	FVector BallSpawnPosition;
	
	UFUNCTION(BlueprintCallable, Category = "A_Mode")
	void SetGoalScore(bool IsPlayerTeam, bool IsOwnGoal, FString AttackerName);
	
	void SetGoalScore(bool IsPlayerTeam);

	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};
