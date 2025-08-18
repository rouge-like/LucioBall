// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/LucioBallMode.h"
#include "Kismet/GameplayStatics.h"
#include "OSC/BouncyBall.h"
#include "OSC/UI/GameHUD.h"
#include "OSC/UI/GameUIWidget.h"

ALucioBallMode::ALucioBallMode()
{
	HUDClass = AGameHUD::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ALucioBallMode::BeginPlay()
{
	SpawnBouncyBall();

	CurrentTime = Time;
	HUD = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD<AGameHUD>();
}

void ALucioBallMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentTime -= DeltaTime;

	if (HUD)
	{
		UGameUIWidget* GameUI = HUD->GetGameUIWidget();
		GameUI->UpdateTimer(CurrentTime);
		GameUI->UpdateSkill(0, DeltaTime);
		GameUI->UpdateSkill(1, DeltaTime);
		GameUI->UpdateUlt(DeltaTime);
	}
	
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
		Goal = TEXT("OwnGoal!");
	}
	else
	{
		Goal = TEXT("Goal!");
	}
	
	FString Log = AttackerName + TEXT("'s ") + Goal;
	
	if (IsPlayerTeam)
	{
		PlayerScore += 1;
		
		if (HUD)
		{
			UGameUIWidget* GameUI = HUD->GetGameUIWidget();
			GameUI->UpdatePlayerScore(PlayerScore);
			GameUI->UpdateGoalText(FText::FromString(Log));
		}
	}
	else
	{
		AIScore += 1;
		
		if (HUD)
		{
			UGameUIWidget* GameUI = HUD->GetGameUIWidget();
			GameUI->UpdateOtherScore(AIScore);
			GameUI->UpdateGoalText(FText::FromString(Log));
		}
	}

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ALucioBallMode::SpawnBouncyBall, 5.0f, false);
}

void ALucioBallMode::SetGoalScore(bool IsPlayerTeam)
{
	GEngine->AddOnScreenDebugMessage(-1,1.0f,FColor::Yellow, TEXT("Goal!"));
	
	if (IsPlayerTeam)
	{
		PlayerScore += 1;
		
		if (HUD)
		{
			UGameUIWidget* GameUI = HUD->GetGameUIWidget();
			GameUI->UpdatePlayerScore(PlayerScore);
			GameUI->UpdateGoalText(FText::FromString(TEXT("Goal!")));
		}
	}
	else
	{
		AIScore += 1;
		
		if (HUD)
		{
			UGameUIWidget* GameUI = HUD->GetGameUIWidget();
			GameUI->UpdateOtherScore(AIScore);
			GameUI->UpdateGoalText(FText::FromString(TEXT("Goal!")));
		}
	}
	
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ALucioBallMode::SpawnBouncyBall, 5.0f, false);
}
