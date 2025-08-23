// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LobbyHUD.generated.h"

/**
 * 
 */
class ULobbyUIWidget;
UCLASS()
class LUCIOBALL_API ALobbyHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	ALobbyHUD();
	TSubclassOf<ULobbyUIWidget> LobbyUIClass;

	UPROPERTY()
	TObjectPtr<ULobbyUIWidget> LobbyUIWidget;
public:
	UFUNCTION(BlueprintCallable)
	ULobbyUIWidget* GetLobbyUIWidget() const { return LobbyUIWidget; };
};
