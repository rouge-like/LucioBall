// Fill out your copyright notice in the Description page of Project Settings.


#include "CEJ/Ai/TurretProjectile.h"

#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

// Sets default values

ATurretProjectile::ATurretProjectile()
{	
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(6.f);
	Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = Collision;

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->UpdatedComponent = Collision;
	Movement->InitialSpeed = Speed;
	Movement->MaxSpeed = Speed;
	Movement->ProjectileGravityScale = 0.f;
	Movement->bRotationFollowsVelocity = true;
}

void ATurretProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// 만약 타겟이 비어있다면 주인공을 찾아서 채우고
	// 그렇지 않다면 그냥 패스 하고싶다.
	// if (nullptr == Target)
	// {
	// 	Target = GetWorld()->GetFirstPlayerController()->GetPawn();
	// }

	// 1) 발사자(Owner/Instigator)와 충돌 무시: 스폰 직후 겹침으로 멈추는 문제 방지
	if (AActor* Ow = GetOwner())
	{
		Collision->IgnoreActorWhenMoving(Ow, /*bShouldIgnore=*/true);
	}
	if (APawn* Inst = GetInstigator())
	{
		Collision->IgnoreActorWhenMoving(Inst, /*bShouldIgnore=*/true);
	}
	// 필요하다면 전부 무시하고 싶을 때:
	// Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	// 2) InitLaunchDir를 못 받았을 경우 Forward로라도 발사 (안전장치)
	if (Movement->Velocity.IsNearlyZero())
	{
		Movement->Velocity = GetActorForwardVector() * Movement->InitialSpeed;
	}

	SetLifeSpan(LifeSeconds);
}

void ATurretProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATurretProjectile::InitLaunchDir(const FVector& InDir)
{
	const FVector Dir = InDir.GetSafeNormal();
	Movement->Velocity = Dir * Movement->InitialSpeed;  // 조준 벡터로 발사
	SetActorRotation(Dir.Rotation());
}


