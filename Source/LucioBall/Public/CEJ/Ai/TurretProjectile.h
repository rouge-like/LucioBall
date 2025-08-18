#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TurretProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class ATurretProjectile : public AActor
{
	GENERATED_BODY()
public:
	ATurretProjectile();

	// 스폰 직후 조준 벡터를 주입
	UFUNCTION(BlueprintCallable)
	void InitLaunchDir(const FVector& InDir);


protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleDefaultsOnly) class USphereComponent* Collision;
	UPROPERTY(VisibleAnywhere) class UProjectileMovementComponent* Movement;

	UPROPERTY(EditDefaultsOnly, Category="Projectile") float Speed = 4000.f;
	UPROPERTY(EditDefaultsOnly, Category="Projectile") float LifeSeconds = 3.f;

	// 넘겨받은 방향(정규화)
	FVector LaunchDir = FVector::ZeroVector;
; 

};
