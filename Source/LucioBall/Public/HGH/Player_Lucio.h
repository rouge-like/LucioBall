// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Player_Lucio.generated.h"

UCLASS()
class LUCIOBALL_API APlayer_Lucio : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayer_Lucio();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// UPROPERTY(EditAnywhere,BlueprintReadWrite)
	// class UCapsuleComponent* CapsuleComp;
	//
	// UPROPERTY(EditAnywhere,BlueprintReadWrite)
	// class USkeletalMeshComponent* SkeletalMeshComp;

	class UCharacterMovementComponent* MoveComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsWallRiding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DiveForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultMaxWalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultWallRideSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultJumpPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WallRideSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WallJumpPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WallRideNormal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WallRideEntryVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WallRideEntryZ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SoundWaveDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SoundWaveRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SoundWavePower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SoundWaveCD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanUseSoundWave;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBuffMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBuffTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBuffCD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanUseSpeedBuff;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UltBuffMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UltBuffTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UltBuffCD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UltBuffJumpPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanUseUlt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSpeedBoosted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanUseDive;
	
	
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UCameraComponent* FollowCamComp;*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UInputMappingContext* IMC_Player;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UInputAction* IA_Dive;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UInputAction* IA_Jump;

	UFUNCTION(BlueprintCallable)
	void OnDive(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	void OnWallJump(const FInputActionValue&  Value);

	

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UCameraShakeBase> LandingShake;
	
	UFUNCTION(BlueprintCallable)
	void OnMyLanded(const FHitResult& Hit);

	UFUNCTION(BlueprintCallable)
	void OnJumpPointBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void OnJumpPointEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	UFUNCTION(BlueprintCallable)
	void WallRide(float DeltaTime);
	
};
