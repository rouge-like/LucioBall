// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/LucioBallMode.h"

#include "CEJ/Ai/AiLucioDynamic.h"
#include "GameFramework/PlayerStart.h"
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

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		FInputModeGameOnly InputMode;
		
		PlayerController->SetInputMode(InputMode);
		PlayerController->bShowMouseCursor = false;
	}
}

void ALucioBallMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentTime > 0.0f)
	{
		CurrentTime -= DeltaTime;
	}
	else
	{
		OnGameEnd();
	}

	if (bIsGameEnding)
	{
		SlowdownTimer += DeltaTime;
		float Alpha = FMath::Clamp(SlowdownTimer / SlowdownDuration, 0.0f, 1.0f);
		float NewTimeDilation = FMath::Lerp(1.0f, 0.2f, Alpha);
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), NewTimeDilation);

		if (SlowdownTimer >= SlowdownDuration)
		{
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);
			
		}
		if (SlowdownTimer >= EndGameTimer)
		{
			UGameplayStatics::OpenLevel(GetWorld(), TEXT("LobbyMap"));
		}
	}

	UGameUIWidget* GameUI = HUD->GetGameUIWidget();
	GameUI->UpdateTimer(CurrentTime);
	GameUI->UpdateSkill(0, DeltaTime);
	GameUI->UpdateSkill(1, DeltaTime);
	GameUI->UpdateUlt(DeltaTime);
}

void ALucioBallMode::SpawnBouncyBall()
{
	if (BouncyBall)
	{
		GetWorld()->SpawnActor<ABouncyBall>(BouncyBall, BallSpawnPosition, FRotator::ZeroRotator);

		// APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		// if (PlayerController)
		// {
		// 	APawn* Player = PlayerController->GetPawn();
		// 	AActor* PlayerStart = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass());
		// 	if (Player)
		// 	{
		// 		Player->SetActorLocation(PlayerStart->GetActorLocation());
		// 		Player->SetActorRotation(PlayerStart->GetActorRotation());
		// 	}
		// }
	}
}

void ALucioBallMode::OnGameEnd()
{
	CurrentTime = 0.0f;
	if (!bIsGameEnding)
	{
		bIsGameEnding = true;
		
		if (PlayerScore >= AIScore)
		{
			HUD->GetGameUIWidget()->OnVictory();
			UGameplayStatics::PlaySound2D(GetWorld(), VictorySFX);
		}
		else
		{
			HUD->GetGameUIWidget()->OnDefeat();
			UGameplayStatics::PlaySound2D(GetWorld(), DefeatSFX);
		}
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

void ALucioBallMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	FString Team = UGameplayStatics::ParseOption(Options, TEXT("mode"));
	UE_LOG(LogTemp, Warning, TEXT("Mode : %s"), *Team);

	if (Team == TEXT("1vs2"))
	{
		AAiLucioDynamic* AI1 = GetWorld()->SpawnActor<AAiLucioDynamic>(LucioAIFactory, SpawnPoints[0], FRotator(0, 90, 0));
		AI1->Tags.Add(TEXT("Defender"));
		
		AAiLucioDynamic* AI2 = GetWorld()->SpawnActor<AAiLucioDynamic>(LucioAIFactory, SpawnPoints[1], FRotator(0, 90, 0));
		AI2->Tags.Add(TEXT("Attacker"));
	}
	else
	{
		AAiLucioDynamic* AI2 = GetWorld()->SpawnActor<AAiLucioDynamic>(LucioAIFactory, SpawnPoints[1], FRotator(0, 90, 0));
		AI2->Tags.Add(TEXT("Defender"));
	}
}
