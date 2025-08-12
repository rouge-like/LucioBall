// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameUIWidget.generated.h"

/**
 * 
 */
class UScoreWidget;
UCLASS()
class LUCIOBALL_API UGameUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScoreWidget> ScoreWidget;

	void UpdatePlayerScore(int32 NewScore);
	void UpdateOtherScore(int32 NewScore);
};
