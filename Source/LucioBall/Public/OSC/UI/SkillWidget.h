// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillWidget.generated.h"

/**
 * 
 */
class UImage;
class UMaterialInstanceDynamic;
UCLASS()
class LUCIOBALL_API USkillWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Inner;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstanceDynamic> Material;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ElapsedTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Duration = 1.0f;
	
	void UpdateProgress(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void UseSkill();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInstance* MaterialInstance;
	
protected:
	virtual void NativeConstruct() override;
};
