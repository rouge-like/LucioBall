// Fill out your copyright notice in the Description page of Project Settings.


#include "GoalPost.h"

#include "BouncyBall.h"
#include "LucioBallMode.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"


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

bool AGoalPost::IsActorPlayer(AActor* Actor)
{
	if (Actor)
		return false;

	APawn* Pawn = Cast<APawn>(Actor);
	if (Pawn)
	{
		AController* Controller = Pawn->GetController();
		if (Controller)
		{
			return Controller->IsPlayerController();
		}
	}
	return false;
}


void AGoalPost::OnGoalHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ABouncyBall* Ball = Cast<ABouncyBall>(OtherActor);

	if (Ball)
	{
		AActor* Attacker = Ball->GetLastAttacker();
		AGameModeBase* GameModeBase = UGameplayStatics::GetGameMode(GetWorld());
		ALucioBallMode* LucioBallMode = Cast<ALucioBallMode>(GameModeBase);
		
		if (Attacker)
		{
			bool IsPlayer = IsActorPlayer(Attacker);
			bool IsOwnGoal = !(IsPlayer ^ IsPlayerTeam);

			LucioBallMode->SetGoalScore(IsPlayerTeam, IsOwnGoal, Attacker->GetName());
		}
		Ball->Destroy();
	}
}


// Called every frame
void AGoalPost::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

