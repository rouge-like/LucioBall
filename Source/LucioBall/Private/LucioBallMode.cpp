// Fill out your copyright notice in the Description page of Project Settings.


#include "LucioBallMode.h"

void ALucioBallMode::SetGoalScore(bool IsPlayerTeam, bool IsOwnGoal, FString AttackerName)
{
	FString Goal;

	if (IsOwnGoal)
	{
		Goal = TEXT("Goal!");
	}
	else
	{
		Goal = TEXT("OwnGoal!");
	}
	
	FString Log = AttackerName + TEXT("'s ") + Goal;
	GEngine->AddOnScreenDebugMessage(-1,1.0f,FColor::Yellow, Log);
	
	if (IsPlayerTeam)
	{
		PlayerScore += 1;
	}
	else
	{
		AIScore += 1;
	}
}
