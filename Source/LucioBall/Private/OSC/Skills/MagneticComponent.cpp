// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Skills/MagneticComponent.h"
#include "OSC/BouncyBall.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

// Sets default values for this component's properties
UMagneticComponent::UMagneticComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMagneticComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	FindBall();
}


// Called every frame
void UMagneticComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ElapsedTime += DeltaTime;
	// ...
	if (!BallActor) FindBall();


	if (ElapsedTime >= CoolTime)
	{
		FVector OwnerLocation = GetOwner()->GetActorLocation();
		FVector BallLocation = BallActor->GetActorLocation();
		float Distance = FVector::Dist(OwnerLocation, BallLocation);

		if (Distance > MinRange && Distance < MaxRange)
		{
			FVector Direction = (OwnerLocation - BallLocation).GetSafeNormal();
			FVector PullForce = Direction * 1000 * DeltaTime;

			BallActor->BouncyBallSetVelocity(FVector(0,0,0), GetOwner());
			BallActor->SetActorLocation(OwnerLocation);
			//BallActor->BouncyBallSetVelocity(PullForce, GetOwner());

			ElapsedTime = 0;
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, "UseMagnetic");
		}
	}


}
void UMagneticComponent::FindBall()
{
	TArray<AActor*> FoundBalls;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABouncyBall::StaticClass(), FoundBalls);

	if (FoundBalls.Num() > 0)
	{
		BallActor = Cast<ABouncyBall>(FoundBalls[0]);
	}
}

