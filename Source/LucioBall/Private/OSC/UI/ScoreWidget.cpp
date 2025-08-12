// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/ScoreWidget.h"
#include "Components/TextBlock.h"

void UScoreWidget::SetPlayerScore(int32 NewScore)
{
	if (PlayerScoreText)
	{
		PlayerScoreText->SetText(FText::AsNumber(NewScore));
	}
};

void UScoreWidget::SetOtherScore(int32 NewScore)
{
	if (OtherScoreText)
	{
		OtherScoreText->SetText(FText::AsNumber(NewScore));
	}
};