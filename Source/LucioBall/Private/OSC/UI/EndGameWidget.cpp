// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/EndGameWidget.h"
#include "MediaPlayer.h"
#include "Components/Image.h"


void UEndGameWidget::NativeConstruct()
{
	Super::NativeConstruct();

	MediaPlayer->OnPlaybackResumed.AddDynamic(this, &UEndGameWidget::OnPlaybackResumed);
}

void UEndGameWidget::OnVictory()
{
	MediaImage->SetBrushFromMaterial(VictoryMaterial);
	MediaPlayer->OpenSource(Victory);
	MediaPlayer->Play();
}

void UEndGameWidget::OnDefeact()
{
	MediaImage->SetBrushFromMaterial(DefeatMaterial);
	MediaPlayer->OpenSource(Defeat);
	MediaPlayer->Play();
}

void UEndGameWidget::OnPlaybackResumed()
{
	MediaImage->SetVisibility(ESlateVisibility::Visible);
}


