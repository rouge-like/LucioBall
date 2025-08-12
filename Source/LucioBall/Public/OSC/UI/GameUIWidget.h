// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GoalTextWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameUIWidget.generated.h"

/**
 * 
 */
class UScoreWidget;
class UTimerWidget;
class UTextWidget;
UCLASS()
class LUCIOBALL_API UGameUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScoreWidget> ScoreWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTimerWidget> TimerWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UGoalTextWidget> GoalTextWidget;

	void UpdatePlayerScore(int32 NewScore);
	void UpdateOtherScore(int32 NewScore);
	void UpdateTimer(float NewTime);
	void UpdateGoalText(FText Text);
};
