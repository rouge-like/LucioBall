// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/LucioBallMode.h"
#include "Kismet/GameplayStatics.h"
#include "OSC/BouncyBall.h"
#include "OSC/UI/GameHUD.h"
#include "OSC/UI/GameUIWidget.h"

ALucioBallMode::ALucioBallMode()
{
	HUDClass = AGameHUD::StaticClass();
}

void ALucioBallMode::BeginPlay()
{
	SpawnBouncyBall();
}

void ALucioBallMode::SpawnBouncyBall()
{
	if (BouncyBall)
	{
		GetWorld()->SpawnActor<ABouncyBall>(BouncyBall, BallSpawnPosition, FRotator::ZeroRotator);
	}
}

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
		
		AGameHUD* HUD = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD<AGameHUD>();
		if (HUD)
		{
			UGameUIWidget* GameUI = HUD->GetGameUIWidget();
			GameUI->UpdatePlayerScore(PlayerScore);
		}

	}
	else
	{
		AIScore += 1;
		
		AGameHUD* HUD = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD<AGameHUD>();
		if (HUD)
		{
			UGameUIWidget* GameUI = HUD->GetGameUIWidget();
			GameUI->UpdateOtherScore(AIScore);
		}
	}

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ALucioBallMode::SpawnBouncyBall, 5.0f, false);
}
