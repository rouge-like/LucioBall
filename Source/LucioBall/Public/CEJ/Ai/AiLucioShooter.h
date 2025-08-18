// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
class ATurretProjectile;                 // 전방선언 

#include "AiLucioShooter.generated.h"

class UArrowComponent;

UCLASS()
class LUCIOBALL_API AAiLucioShooter : public ACharacter
{
	GENERATED_BODY()


public:
	AAiLucioShooter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere) UArrowComponent* Muzzle;

	// 1초마다 쏠 탄환 클래스와 주기
	UPROPERTY(EditAnywhere, Category="Combat")
	TSubclassOf<ATurretProjectile> ProjectileClass;  // BP_Turret(부모가 ATurretProjectile)

	UPROPERTY(EditAnywhere, Category="Combat")
	float FireRate = 1.0f;                           // 1초

	TWeakObjectPtr<AActor> Ball;

	FTimerHandle FindBallTimer;
	FTimerHandle FireTimer;

	void FindBall();
	void StartFiring();
	UFUNCTION() void FireOnce();                     // 타이머 콜백
	void AimAtTarget(float DeltaSeconds);
};
