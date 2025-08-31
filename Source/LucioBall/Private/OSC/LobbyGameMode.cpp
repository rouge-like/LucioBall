// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/LobbyGameMode.h"

#include "Kismet/GameplayStatics.h"

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		PlayerController->SetInputMode(InputMode);
		PlayerController->bShowMouseCursor = true;
	}
	
	UGameplayStatics::PlaySound2D(GetWorld(), LobbyBGM);
}
