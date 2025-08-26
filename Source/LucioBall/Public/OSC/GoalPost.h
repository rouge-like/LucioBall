// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GoalPost.generated.h"

class UBoxComponent;
class UNiagaraSystem;
class UTextRenderComponent;
UCLASS()
class LUCIOBALL_API AGoalPost : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGoalPost();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> GoalPostMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> BoxCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	bool bIsPlayerTeam;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UNiagaraSystem> GoalVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UTextRenderComponent> GolText;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnGoalHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	bool IsActorPlayer(AActor* Actor);
	bool bShowText = false;
	float CurrentTime = 0;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USoundBase> GoalSFX;
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USoundBase> CrowdSFX;
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundAttenuation> Attenuation;
};
