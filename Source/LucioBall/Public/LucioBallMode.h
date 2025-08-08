// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LucioBallMode.generated.h"


UCLASS()
class LUCIOBALL_API ALucioBallMode : public AGameModeBase
{
	GENERATED_BODY()

private:
	int PlayerScore;
	int AIScore;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Components")
	void SetGoalScore(bool IsPlayerTeam, bool IsOwnGoal, FString AttackerName);
};
