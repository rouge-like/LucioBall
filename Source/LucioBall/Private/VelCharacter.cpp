// Fill out your copyright notice in the Description page of Project Settings.


#include "VelCharacter.h"

// Sets default values
AVelCharacter::AVelCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PreviousVelocity = FVector::ZeroVector;
}

// Called when the game starts or when spawned
void AVelCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVelCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PreviousVelocity = GetVelocity();
}

// Called to bind functionality to input
void AVelCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

FVector AVelCharacter::GetPreviousVelocity()
{
	return PreviousVelocity;
}


