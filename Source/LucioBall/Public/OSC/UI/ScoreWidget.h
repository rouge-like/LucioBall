// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreWidget.generated.h"

/**
 * 
 */
class UTextBlock;
UCLASS()
class LUCIOBALL_API UScoreWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerScoreText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> OtherScoreText;

	void SetPlayerScore(int32 NewScore);
	void SetOtherScore(int32 NewScore);
};
