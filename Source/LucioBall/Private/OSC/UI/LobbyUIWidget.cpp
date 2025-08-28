// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/LobbyUIWidget.h"

void ULobbyUIWidget::PlayOnPlayAnimation()
{
	if (OnPlayAnimation)
		PlayAnimation(OnPlayAnimation);
}
