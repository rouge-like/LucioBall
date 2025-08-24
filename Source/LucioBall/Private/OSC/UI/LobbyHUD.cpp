// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/LobbyHUD.h"
#include "OSC/UI/LobbyUIWidget.h"
#include "Blueprint/UserWidget.h"

ALobbyHUD::ALobbyHUD()
{
	static ConstructorHelpers::FClassFinder<ULobbyUIWidget> BPLobbyUIClass(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/Assets/UI/BluePrints/WBP_LobbyUI.WBP_LobbyUI_C'"));

	if (BPLobbyUIClass.Succeeded())
	{
		LobbyUIClass = BPLobbyUIClass.Class;
	}
}


void ALobbyHUD::BeginPlay()
{
	Super::BeginPlay();

	if (LobbyUIClass)
	{
		LobbyUIWidget = CreateWidget<ULobbyUIWidget>(GetWorld(), LobbyUIClass);

		if (LobbyUIWidget)
		{
			LobbyUIWidget->AddToViewport();
		}
	}
}
