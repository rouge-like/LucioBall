// Fill out your copyright notice in the Description page of Project Settings.


//#include "Player_Lucio.h"
#include "HGH/Player_Lucio.h"

#include "Components/CapsuleComponent.h"
#include "DataWrappers/ChaosVDJointDataWrappers.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
APlayer_Lucio::APlayer_Lucio()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	// SetRootComponent(CapsuleComp);
	//
	// SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	// SkeletalMeshComp->SetupAttachment(RootComponent);

	MoveComp = GetCharacterMovement();

}

// Called when the game starts or when spawned
void APlayer_Lucio::BeginPlay()
{
	Super::BeginPlay();

	DefaultMaxWalkSpeed = MoveComp->MaxWalkSpeed;
	DefaultWallRideSpeed = WallRideSpeed;
	DefaultJumpPower = MoveComp->JumpZVelocity;

	
	APlayerController* pc = GetWorld()->GetFirstPlayerController();
	UEnhancedInputLocalPlayerSubsystem* subsys = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer());

	if (subsys)
	{
		subsys->AddMappingContext(IMC_Player, 0);
	}
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
	UEnhancedInputComponent *input = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (input)
	{
		input->BindAction(IA_Dive, ETriggerEvent::Triggered, this, &APlayer_Lucio::OnDive);
		input->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &APlayer_Lucio::OnWallJump);
	}

}

void APlayer_Lucio::OnDive(const FInputActionValue& Value)
{
	if (!bIsWallRiding && bCanUseDive)
	{
		APlayer_Lucio::LaunchCharacter(FVector(0, 0, DiveForce), false, true);
	}
}

void APlayer_Lucio::OnWallJump(const FInputActionValue& Vorce)
{
	if (bIsWallRiding)
	{
		APlayer_Lucio::bIsWallRiding = false;
		MoveComp->GravityScale = 1.75f;
		APlayer_Lucio::LaunchCharacter(FVector((WallRideNormal * WallJumpPower) + (FVector(0, 0, 1) * WallJumpPower) + MoveComp->Velocity), true, true);
		
	}
}

