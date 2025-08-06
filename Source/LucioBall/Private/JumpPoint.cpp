// Fill out your copyright notice in the Description page of Project Settings.


#include "JumpPoint.h"


// Sets default values
AJumpPoint::AJumpPoint()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	JumpPointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JumpPointMesh"));
	RootComponent = JumpPointMesh;

	JumpPointMesh -> SetSimulatePhysics(false);
	JumpPointMesh -> SetNotifyRigidBodyCollision(true);
	
	Tags.Add(FName(TEXT("JumpPoint")));
}

// Called when the game starts or when spawned
void AJumpPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AJumpPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}