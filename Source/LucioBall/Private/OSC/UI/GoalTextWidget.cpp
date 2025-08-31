// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/GoalTextWidget.h"
#include "Components/TextBlock.h"

UGoalTextWidget::UGoalTextWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetHidden();
}

void UGoalTextWidget::SetHidden()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void UGoalTextWidget::UpdateGoalText(FText Text)
{
	if (GoalTextAnimation)
	{
		PlayAnimation(GoalTextAnimation);
	}
	SetVisibility(ESlateVisibility::Visible);
	GoalText->SetText(Text);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UGoalTextWidget::SetHidden, 2.0f );
}
