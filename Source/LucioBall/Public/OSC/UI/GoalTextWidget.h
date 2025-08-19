// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GoalTextWidget.generated.h"

/**
 * 
 */
class UTextBlock;
UCLASS()
class LUCIOBALL_API UGoalTextWidget : public UUserWidget
{
	GENERATED_BODY()
private:
	FTimerHandle TimerHandle;

	UFUNCTION()
	void SetHidden();
	
public:
	UGoalTextWidget(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GoalText;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* GoalTextAnimation;
	
	void UpdateGoalText(FText Text);
};
