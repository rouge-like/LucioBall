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
class USkillWidget;
class UUltGaugeWidget;

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
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUltGaugeWidget> UltWidget;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USkillWidget> Skill0;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USkillWidget> Skill1;

	void UpdatePlayerScore(int32 NewScore);
	void UpdateOtherScore(int32 NewScore);
	void UpdateTimer(float NewTime);
	void UpdateGoalText(FText Text);
	void UpdateSkill(int Idx, float Sec);
	void UpdateUlt(float DeltaTime);
	
	UFUNCTION(BlueprintCallable)
	void UseSkill(int Idx);
};
