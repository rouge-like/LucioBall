// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/BouncyBall.h"
#include "OSC/VelCharacter.h"
#include "Engine/Engine.h"

// Sets default values
ABouncyBall::ABouncyBall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	RootComponent = BallMesh;
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));

	if (SphereMeshAsset.Succeeded())
	{
		BallMesh->SetStaticMesh(SphereMeshAsset.Object);
	}
	
	BallMesh->SetSimulatePhysics(true);
	
	BallMesh->SetNotifyRigidBodyCollision(true);
	
	BallMesh->OnComponentHit.AddDynamic(this, &ABouncyBall::OnBallHit);
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
	
	PreviousVelocity = BallMesh->GetPhysicsLinearVelocity();
}

void ABouncyBall::OnBallHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (PreviousVelocity.SizeSquared() <= 0.01f)
	{
		BallMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
		return;	
	}
	
	float Dot = PreviousVelocity.Dot(Hit.ImpactNormal);

	if (Dot > -0.1f)
	{
		// 마찰력 적용
		return;
	}
	
	FVector FinalVelocity;
	FVector ReflectedVelocity = PreviousVelocity - 2 * Dot * Hit.ImpactNormal;
	
	FinalVelocity =  ReflectedVelocity * Elasticity;
	
	BallMesh->SetPhysicsLinearVelocity(FinalVelocity);
}
void ABouncyBall::BouncyBallAddImpulse(FVector Impulse, AActor* Attacker)
{
	const FVector FinalVelocity =  PreviousVelocity + Impulse;
	LastAttacker = Attacker;
	BallMesh->SetPhysicsLinearVelocity(FinalVelocity);
}

AActor* ABouncyBall::GetLastAttacker() const
{
	return LastAttacker;
}
