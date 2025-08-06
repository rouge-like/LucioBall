// Fill out your copyright notice in the Description page of Project Settings.


#include "BouncyBall.h"

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
	FVector FinalVelocity;
	/*if (OtherComp && OtherComp->IsSimulatingPhysics())
	{
		FVector Normal = Hit.ImpactNormal;
		Normal.Normalize();

		FVector OtherVelocity = OtherComp->GetPhysicsLinearVelocity();
		FVector RelativeVelocity = PreviousVelocity - OtherVelocity;

		FVector ReflectedRelativeVelocity = RelativeVelocity - Normal * (2 * ReflectedRelativeVelocity.Dot(Normal));

		FinalVelocity = ReflectedRelativeVelocity + OtherVelocity;
		
	}
	else*/
	{
		FVector ReflectedVelocity = PreviousVelocity - 2 * PreviousVelocity.Dot(Hit.ImpactNormal) * Hit.ImpactNormal;
		FinalVelocity =  ReflectedVelocity * Elasticity;
	}
	
	
	BallMesh->SetPhysicsLinearVelocity(FinalVelocity);

	UE_LOG(LogTemp, Warning, TEXT("Ball Hit! New Velocity: %s"), *FinalVelocity.ToString());
}
void ABouncyBall::BouncyBallAddImpulse(FVector Impulse)
{
	const FVector FinalVelocity =  PreviousVelocity + Impulse;
	
	BallMesh->SetPhysicsLinearVelocity(FinalVelocity);
	
}
