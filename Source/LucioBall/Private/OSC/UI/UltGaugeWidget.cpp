// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/UltGaugeWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

void UUltGaugeWidget::UseUlt()
{
	ElapsedTime = 0;
}

void UUltGaugeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (MaterialInstance)
	{
		Material = UMaterialInstanceDynamic::Create(MaterialInstance, this);
		Gauge->SetBrushFromMaterial(Material);
	}
	
}

void UUltGaugeWidget::UpdateProgress(float DeltaTime)
{
	ElapsedTime += DeltaTime;

	if (ElapsedTime >= Duration)
	{
		ElapsedTime = Duration;
	}
	float Ratio = ElapsedTime / Duration;

	Material->SetScalarParameterValue(TEXT("Progress"), Ratio);
	GaugeText->SetText(FText::AsPercent(Ratio));
}