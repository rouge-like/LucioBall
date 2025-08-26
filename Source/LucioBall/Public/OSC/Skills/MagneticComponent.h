// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MagneticComponent.generated.h"


class UNiagaraSystem;
class ABouncyBall;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LUCIOBALL_API UMagneticComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMagneticComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UNiagaraSystem> SkillVFX;
private:
	UPROPERTY()
	TObjectPtr<ABouncyBall> BallActor;

	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;

	UPROPERTY(EditAnyWhere)
	float ElapsedTime = 10.0f;

	UPROPERTY(EditAnyWhere)
	float CoolTime = 10.f;

	UPROPERTY(EditAnyWhere)
	float MinRange = 200.f;
	
	UPROPERTY(EditAnyWhere)
	float MaxRange = 2000.f;

	bool bIsPulling = false;

	UPROPERTY(EditAnywhere)
	float PullInterpSpeed = 5.f;

	FVector PullStartLocation;
	
	float Alpha;

	void FindBall();
};
