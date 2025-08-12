// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/GameUIWidget.h"
#include "OSC/UI/ScoreWidget.h"
#include "OSC/UI/TimerWidget.h"
#include "OSC/UI/GoalTextWidget.h"

void UGameUIWidget::UpdatePlayerScore(int32 NewScore)
{
	ScoreWidget->SetPlayerScore(NewScore);
}

void UGameUIWidget::UpdateOtherScore(int32 NewScore)
{
	ScoreWidget->SetOtherScore(NewScore);
}

void UGameUIWidget::UpdateTimer(float NewTime)
{
	TimerWidget->UpdateRemainTime(NewTime);
}

void UGameUIWidget::UpdateGoalText(FText Text)
{
	GoalTextWidget->UpdateGoalText(Text);
}
