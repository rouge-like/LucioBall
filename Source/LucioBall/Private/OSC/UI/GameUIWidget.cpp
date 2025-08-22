// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/GameUIWidget.h"
#include "OSC/UI/ScoreWidget.h"
#include "OSC/UI/TimerWidget.h"
#include "OSC/UI/GoalTextWidget.h"
#include  "OSC/UI/SkillWidget.h"
#include  "OSC/UI/UltGaugeWidget.h"

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

void UGameUIWidget::UpdateSkill(int Idx, float Sec)
{
	switch (Idx)
	{
		case 0:
		Skill0->UpdateProgress(Sec);
		break;
		case 1:
		Skill1->UpdateProgress(Sec);
		break;
	}
}

void UGameUIWidget::UpdateUlt(float DeltaTime)
{
	UltWidget->UpdateProgress(DeltaTime);
}

void UGameUIWidget::UseSkill(int Idx)
{
	switch (Idx)
	{
	case 0:
		Skill0->UseSkill();
		break;
	case 1:
		Skill1->UseSkill();
		break;
	}
}

void UGameUIWidget::UseUlt()
{
	UltWidget->UseUlt();
}
