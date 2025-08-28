// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LucioBallMode.generated.h"


class AAiLucioDynamic;
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
	UPROPERTY(EditDefaultsOnly)
	float EndGameTimer = 2.3f;
	
	UFUNCTION()
	void SpawnBouncyBall();

	void OnGameEnd();
	
public:
	ALucioBallMode();
	
	UPROPERTY(EditDefaultsOnly)
	float Time = 240;

	UPROPERTY(EditDefaultsOnly)
	USoundBase* VictorySFX;

	UPROPERTY(EditDefaultsOnly)
	USoundBase* DefeatSFX;
	
	UPROPERTY(EditDefaultsOnly, Category = "A_Mode")
	TSubclassOf<ABouncyBall> BouncyBall;

	UPROPERTY(EditDefaultsOnly, Category = "A_Mode")
	FVector BallSpawnPosition;

	UPROPERTY(EditDefaultsOnly, Category = "A_Mode")
	TSubclassOf<AAiLucioDynamic> LucioAIFactory;
	
	UPROPERTY(EditDefaultsOnly, Category = "A_Mode")
	TArray<FVector> SpawnPoints;
	
	UFUNCTION(BlueprintCallable, Category = "A_Mode")
	void SetGoalScore(bool IsPlayerTeam, bool IsOwnGoal, FString AttackerName);
	
	void SetGoalScore(bool IsPlayerTeam);
	

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};
