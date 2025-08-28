// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/GameHUD.h"
#include "OSC/UI/GameUIWidget.h"
#include "Blueprint/UserWidget.h"

AGameHUD::AGameHUD()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> BPGameUIClass(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/Assets/UI/BluePrints/WBP_GameUI.WBP_GameUI_C'"));

	if (BPGameUIClass.Succeeded())
	{
		GameUIClass = BPGameUIClass.Class;
	}
}

void AGameHUD::BeginPlay()
{
	Super::BeginPlay();
	
	if (GameUIClass)
	{
		GameUIWidget = CreateWidget<UGameUIWidget>(GetWorld(), GameUIClass);

		if (GameUIWidget)
		{
			GameUIWidget->AddToViewport(1);
		}
	}
}

