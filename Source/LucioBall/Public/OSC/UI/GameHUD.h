// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GameHUD.generated.h"

/**
 * 
 */
class UGameUIWidget;
UCLASS()
class LUCIOBALL_API AGameHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	AGameHUD();
	
	TSubclassOf<UGameUIWidget> GameUIClass;

	UPROPERTY()
	TObjectPtr<UGameUIWidget> GameUIWidget;

public:
	UGameUIWidget* GetGameUIWidget() const { return GameUIWidget; };
};
