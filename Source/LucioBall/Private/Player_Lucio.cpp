// Fill out your copyright notice in the Description page of Project Settings.


#include "Player_Lucio.h"

// Sets default values
APlayer_Lucio::APlayer_Lucio()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APlayer_Lucio::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APlayer_Lucio::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayer_Lucio::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void APlayer_Lucio::MyDive()
{
}

