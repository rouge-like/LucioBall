// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EndGameWidget.generated.h"

/**
 * 
 */
class UMaterial;
class UMediaPlayer;
class UMediaSource;
class UImage;
UCLASS()
class LUCIOBALL_API UEndGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMediaPlayer> MediaPlayer;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterial> VictoryMaterial;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterial> DefeatMaterial;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UMediaSource> Victory;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UMediaSource> Defeat;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MediaImage; 

	virtual void NativeConstruct() override;
	
	void OnVictory();
	void OnDefeact();

	UFUNCTION()
	void OnPlaybackResumed();
};
