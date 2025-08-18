// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/BouncyBall.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ABouncyBall::ABouncyBall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetSphereRadius(Radius);
	RootComponent = Sphere;
	
	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	BallMesh->SetupAttachment(RootComponent);
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));

	if (SphereMeshAsset.Succeeded())
	{
		BallMesh->SetStaticMesh(SphereMeshAsset.Object);
	}

	Sphere->SetSimulatePhysics(false);
	Sphere->SetNotifyRigidBodyCollision(true);
	
	// 모든 충돌은 OnBouncyBallHit을 통해 처리하도록 설정합니다.
	Sphere->OnComponentHit.AddDynamic(this, &ABouncyBall::OnBouncyBallHit);
}

// Called when the game starts or when spawned
void ABouncyBall::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABouncyBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. 접지 상태 확인
	bool bIsOnGround = CheckGrounded();
	
	// 1a. 접지가 아닐 경우 중력 적용
	if (!bIsOnGround)
	{
		const FVector Gravity = FVector(0.0f, 0.0f, GetWorld()->GetGravityZ());
		CurrentVelocity += Gravity * GravityScale * DeltaTime;
	}
	// 1b. 접지 일 경우 마찰력 적용
	else
	{
		const FVector FrictionDirection = FVector(CurrentVelocity.X, CurrentVelocity.Y, 0);
		const float FrictionForce = Friction * DeltaTime;

		if (CurrentVelocity.Size() <= FrictionForce)
			CurrentVelocity = FVector(0, 0, 0);
		else
			CurrentVelocity -= FrictionDirection * Friction * DeltaTime;

		DrawDebugDirectionalArrow(GetWorld(), GetActorLocation() - FVector(0.f, 0.f, Radius), GetActorLocation() - FVector(0.f, 0.f, Radius) - FrictionDirection * Friction * DeltaTime, 100, FColor::Yellow, 0, 1.0f, 0, 3.f);
	}

	// 2. 이동 (충돌 반응은 OnBouncyBallHit에서 처리)
	SetActorLocation(GetActorLocation() + CurrentVelocity * DeltaTime, true);

	// 3. 시각적 회전
	if (!CurrentVelocity.IsNearlyZero() && Radius > 0.f)
	{
		const FVector RotationAxis = FVector::CrossProduct(FVector::UpVector, CurrentVelocity.GetSafeNormal());
		const float RotationAngle = (CurrentVelocity.Size() * DeltaTime) / Radius;
		const FQuat DeltaRotation(RotationAxis, RotationAngle);
		BallMesh->AddWorldRotation(DeltaRotation, false);
	}

	if (bIsOnGround && FMath::Abs(CurrentVelocity.Z) < 5.f)
	{
		CurrentVelocity.Z = 0.f;
	}
}

FVector ABouncyBall::CalculateReflectionAndFriction(const FVector& InVelocity, const FHitResult& Hit)
{
	const float Dot = InVelocity.Dot(Hit.ImpactNormal);

	// 이미 접지 상태이거나 스쳐지나감
	if (Dot > -0.1f)
	{
		return InVelocity;
	}

	// 반사: 정면 충돌
	const FVector ReflectedVelocity = InVelocity - 2 * Dot * Hit.ImpactNormal;
	return ReflectedVelocity * Elasticity;
}

bool ABouncyBall::CheckGrounded()
{
	FHitResult Hit;
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0.f, 0.f, Radius + 1);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	return bHit && Hit.ImpactNormal.Z > 0.9f;
}

// 최종적으로 완성된 OnHit 로직
void ABouncyBall::OnBouncyBallHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 1. 먼저, 충돌한 표면을 기준으로 공의 기본 반사 속도를 계산합니다.
	FVector NewVelocity = CalculateReflectionAndFriction(CurrentVelocity, Hit);

	// 2. 만약 충돌한 상대가 캐릭터라면, 추가적인 상호작용을 계산합니다.
	if (ACharacter* HitCharacter = Cast<ACharacter>(OtherActor))
	{
		if (UCharacterMovementComponent* MovementComp = HitCharacter->GetCharacterMovement())
		{
			LastAttacker = OtherActor;
			
			const FVector CharacterVelocity = MovementComp->GetLastUpdateVelocity();
			FVector PushDirection = GetActorLocation() - HitCharacter->GetActorLocation();
			PushDirection.Z = 0.0f;
			PushDirection.Normalize();
			const float PushStrength = FVector::DotProduct(CharacterVelocity, PushDirection);
			
			// 2a. 캐릭터의 속도를 반사된 공의 속도에 더해줍니다.
			NewVelocity += PushDirection * PushStrength * Elasticity * 0.25f;

			// 2b. 충돌로 감속된 캐릭터에게 이전 속도를 되찾아 줍니다.
			HitCharacter->LaunchCharacter(CharacterVelocity, true, true);
		}
	}
	
	DrawDebugDirectionalArrow(GetWorld(), Hit.ImpactPoint - CurrentVelocity.GetSafeNormal() * 100, Hit.ImpactPoint, 100, FColor::Blue, false, 1.0f, 3.f);
	DrawDebugDirectionalArrow(GetWorld(), Hit.ImpactPoint, Hit.ImpactPoint + NewVelocity.GetSafeNormal() * 100, 100, FColor::Green, false, 1.0f, 3.f);
	DrawDebugDirectionalArrow(GetWorld(), Hit.ImpactPoint, Hit.ImpactPoint + Hit.ImpactNormal * 100, 100, FColor::Red, false, 1.0f, 3.f);

	// 3. 최종 계산된 속도를 공에 적용합니다.
	CurrentVelocity = NewVelocity;
	SetActorLocation(GetActorLocation() + Hit.ImpactNormal * 0.1f);



}


void ABouncyBall::BouncyBallAddImpulse(FVector Impulse, AActor* Attacker)
{
	CurrentVelocity += Impulse;
	LastAttacker = Attacker;
}

AActor* ABouncyBall::GetLastAttacker() const
{
	return LastAttacker;
}
