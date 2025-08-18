// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/SkillWidget.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

void USkillWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (auto FillBrush = Inner->Brush.GetResourceObject())
	{
		if (auto Mat =  Cast<UMaterialInstance>(FillBrush))
		{
			Material = UMaterialInstanceDynamic::Create(Mat, this);
			Inner->SetBrushFromMaterial(Material);
		}
	}

	ElapsedTime = Duration;
}


void USkillWidget::UpdateProgress(float DeltaTime)
{
	ElapsedTime += DeltaTime;

	if (ElapsedTime >= Duration)
	{
		ElapsedTime = Duration;
	}
	
	Material->SetScalarParameterValue(TEXT("Progress"), ElapsedTime / Duration);
}

void USkillWidget::UseSkill()
{
	ElapsedTime = 0;
}
