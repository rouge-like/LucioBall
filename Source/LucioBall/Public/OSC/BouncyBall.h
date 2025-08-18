// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BouncyBall.generated.h"

UCLASS()
class LUCIOBALL_API ABouncyBall : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABouncyBall();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BallMesh;

protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BouncyBall Physics")
	float Elasticity = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<AActor> LastAttacker;

	UFUNCTION()
	void OnBallHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	FVector PreviousVelocity;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable, Category = "BouncyBall")
	void BouncyBallAddImpulse(FVector Impulse, AActor* Attacker);

	UFUNCTION(BlueprintCallable, Category = "BouncyBall")
	AActor* GetLastAttacker() const;
};
