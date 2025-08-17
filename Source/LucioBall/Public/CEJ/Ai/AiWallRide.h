// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AiWallRide.generated.h"


class UWallRideComponent;

UCLASS()
class LUCIOBALL_API AAiWallRide : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAiWallRide();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UFUNCTION()
	void OnJumpPressed();

	// 벽 찾기(스피어 트레이스 + 각도 판정)
	bool FindWall(FVector& OutNormal, FVector& OutTangent);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WallRide")
	UWallRideComponent* WallRideComp = nullptr;

	// 트레이스 파라미터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRide|Trace")
	float TraceForward = 140.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRide|Trace")
	float TraceHeight = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRide|Trace")
	float TraceRadius = 20.f;

	// 각도 판정(Dot(F, N) 범위)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRide|Trace")
	float DotMin = -0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRide|Trace")
	float DotMax = -0.2f;
};
