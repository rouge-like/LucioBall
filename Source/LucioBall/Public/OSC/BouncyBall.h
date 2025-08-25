// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BouncyBall.generated.h"

class USphereComponent;
class UWidgetComponent;
class USplineComponent;

UCLASS()
class LUCIOBALL_API ABouncyBall : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABouncyBall();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> BallMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> Sphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> BallWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> GroundChecker;

protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Elasticity = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Friction = 0.35f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GravityScale = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<AActor> LastAttacker;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector CurrentVelocity;

	FVector LandLocation;

	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<USoundBase>> BounceSFXs;
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundAttenuation> Attenuation;
	UFUNCTION()
	void OnBouncyBallHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	FVector CalculateReflectionAndFriction(const FVector& InVelocity, const FHitResult& Hit);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable)
	void BouncyBallAddImpulse(FVector Impulse, AActor* Attacker);

	UFUNCTION(BlueprintCallable)
	AActor* GetLastAttacker() const { return LastAttacker; };

	UFUNCTION(BlueprintCallable)
	FVector GetBouncyBallVelocity() const { return CurrentVelocity; }
	
	UFUNCTION(BlueprintCallable)
	FVector GetLandLocation() const { return LandLocation; }
};
