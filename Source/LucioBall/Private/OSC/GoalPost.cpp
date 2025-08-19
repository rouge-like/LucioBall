// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/GoalPost.h"
#include "OSC/BouncyBall.h"
#include "OSC/LucioBallMode.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/TextRenderComponent.h"

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

	GolText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("GolText"));
	GolText->SetupAttachment(RootComponent);
	GolText->SetText(FText::FromString("GOL!"));
	GolText->SetTextRenderColor(FColor(255,255,0,0));
}

// Called when the game starts or when spawned
void AGoalPost::BeginPlay()
{
	Super::BeginPlay();
}

bool AGoalPost::IsActorPlayer(AActor* Actor)
{
	if (!Actor)
		return false;

	APawn* Pawn = Cast<APawn>(Actor);
	if (Pawn)
	{
		AController* Controller = Pawn->GetController();
		if (Controller)
		{
			UE_LOG(LogTemp, Warning, TEXT("IsPlayer?? : %d"),Controller->IsPlayerController());
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
			bool IsOwnGoal = !(IsPlayer ^ bIsPlayerTeam);
			UE_LOG(LogTemp, Warning, TEXT("IsOwn : %d // IsPlayer : %d"),IsOwnGoal, IsPlayer);
			LucioBallMode->SetGoalScore(!bIsPlayerTeam, IsOwnGoal, Attacker->GetName());
		}
		else
		{
			LucioBallMode->SetGoalScore(!bIsPlayerTeam);
		}
		Ball->Destroy();

		if (GoalVFX)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this,GoalVFX,GetActorLocation(), GetActorRotation(), FVector(3.0f),true,true);
		}

		bShowText = true;
	}
}


// Called every frame
void AGoalPost::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShowText)
	{
		CurrentTime += DeltaTime;

		float Alpha = CurrentTime;

		if (Alpha >= 2.0f)
		{
			bShowText = false;
			GolText->SetTextRenderColor(FColor(255,255,0, 0));
			CurrentTime = 0;
		}
		else if (Alpha >= 1.0f)
		{
			GolText->SetTextRenderColor(FColor(255,255,0,(2 - Alpha) * 255));
		}
		else
		{
			GolText->SetTextRenderColor(FColor(255,255,0,Alpha * 255));
		}
	}
}

