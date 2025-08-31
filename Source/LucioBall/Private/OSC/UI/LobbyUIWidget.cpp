// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/LobbyUIWidget.h"

#include "Kismet/GameplayStatics.h"

void ULobbyUIWidget::PlayOnPlayAnimation()
{
	if (OnPlayAnimation)
		PlayAnimation(OnPlayAnimation);
}

FReply ULobbyUIWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Q)
	{
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		UKismetSystemLibrary::QuitGame(GetWorld(), PlayerController, EQuitPreference::Quit, false);
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
