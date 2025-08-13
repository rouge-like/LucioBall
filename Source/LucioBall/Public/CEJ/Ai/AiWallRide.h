// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AiWallRide.generated.h"

UCLASS()
class LUCIOBALL_API UWallRideComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWallRideComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 상태
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bWallRiding = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bWallExit = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bEnteringPhase = false;

	// 파라미터(감각용 시작점)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VMaxWall = 1400.f;                 // 1200~1600 권장

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TEnter = 0.25f;                    // 0.25s 권장

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TExit  = 0.25f;                    // 0.2~0.3s 권장

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WallFrictionA = 0.f;               // 등가속 계수(감속이면 음수). 맵/소재별 미세 조정

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StickyNormalForce = 0.f;           // 벽에 붙는 “감각”용(선택). 필요 없으면 0.

	// 런타임 상태
	UPROPERTY(BlueprintReadOnly)
	float Speed = 0.f;

	UPROPERTY(BlueprintReadOnly)
	FVector Velocity = FVector::ZeroVector;

	// 내부 계산된 가속도
	float AEnter = 0.f;   // (VTarget - V0) / TEnter
	float AExit  = 0.f;   // -(V0) / TExit

	// 벽 접선/법선은 캐릭터가 벽을 잡을 때 갱신해두세요
	UFUNCTION(BlueprintCallable)
	void BeginWallRide(const FVector& InWallNormal, const FVector& InTangent, float CurrentSpeed, float TargetSpeed);

	UFUNCTION(BlueprintCallable)
	void EndWallRide(); // 이탈 시작

private:
	FVector WallNormal = FVector::ZeroVector;   // 단위벡터
	FVector WallTangent = FVector::ForwardVector;// 단위벡터

	FVector ComputeWallTangent() const { return WallTangent; }
	FVector GetGravityVector() const; // 월드 중력
};
