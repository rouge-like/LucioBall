// Fill out your copyright notice in the Description page of Project Settings.


#include "LucioBall/Public/Osc/UI/TimerWidget.h"
#include "Components/TextBlock.h"

void UTimerWidget::UpdateRemainTime(float time)
{
	int IntTime = FMath::FloorToInt(time + 1);
	
	int32 Min = IntTime / 60;
	int32 Sec = IntTime % 60;    

	if (RemainTimeText)
	{
		if (time <= 0)
			RemainTimeText->SetText(FText::FromString(TEXT("0:00")));
		else
			RemainTimeText->SetText(FText::FromString(FString::Printf(TEXT("%d:%02d"), Min, Sec)));
	}
}

