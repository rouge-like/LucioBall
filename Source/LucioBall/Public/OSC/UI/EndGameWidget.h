// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EndGameWidget.generated.h"

/**
 * 
 */
class UMaterial;
UCLASS()
class LUCIOBALL_API UEndGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterial> VictoryMaterial;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterial> DefeatMaterial;

	void OnVictory();
	void OnDefeact();
};
