// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UltGaugeWidget.generated.h"

/**
 * 
 */
class UTextBlock;
class UImage;
class UMaterialInstanceDynamic;
UCLASS()
class LUCIOBALL_API UUltGaugeWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GaugeText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Gauge;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstanceDynamic> Material;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ElapsedTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Duration = 30.0f;
	
	void UpdateProgress(float DeltaTime);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInstance* MaterialInstance;

	UFUNCTION(BlueprintCallable)
	void UseUlt();
protected:
	virtual void NativeConstruct() override;	
};
