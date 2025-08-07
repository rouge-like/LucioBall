// Fill out your copyright notice in the Description page of Project Settings.


#include "GoalPost.h"

#include "BouncyBall.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"


// Sets default values
AGoalPost::AGoalPost()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GoalPostMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GoalPostMesh"));
	GoalPostMesh->SetNotifyRigidBodyCollision(true);
	
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollisionComponent"));
	BoxCollision->SetHiddenInGame(false);
	BoxCollision->ShapeColor = FColor::Red;
	BoxCollision->OnComponentHit.AddDynamic(this, &AGoalPost::OnGoalHit);
	BoxCollision->SetNotifyRigidBodyCollision(true);

	RootComponent = BoxCollision;
}

// Called when the game starts or when spawned
void AGoalPost::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGoalPost::OnGoalHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ABouncyBall* Ball = Cast<ABouncyBall>(OtherActor);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1,1.0f,FColor::Yellow, OtherActor->GetName());	
	}
	if (Ball)
	{
		Ball->Destroy();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,1.0f,FColor::Yellow, "GOAL!");	
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,1.0f,FColor::Yellow, OtherComp->GetName());	
		}
	}
}


// Called every frame
void AGoalPost::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

