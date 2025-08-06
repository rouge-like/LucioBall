// Fill out your copyright notice in the Description page of Project Settings.


#include "GoalPost.h"


// Sets default values
AGoalPost::AGoalPost()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GoalPostMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GoalPostMesh"));
	RootComponent = GoalPostMesh;
}

// Called when the game starts or when spawned
void AGoalPost::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGoalPost::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

