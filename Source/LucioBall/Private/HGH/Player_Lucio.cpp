// Fill out your copyright notice in the Description page of Project Settings.


//#include "Player_Lucio.h"
#include "HGH/Player_Lucio.h"

#include "Components/CapsuleComponent.h"
#include "DataWrappers/ChaosVDJointDataWrappers.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "OSC/BouncyBall.h"

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

	if (ACharacter* MyChar = Cast<ACharacter>(this))
	{
		MyChar->LandedDelegate.AddDynamic(this, &APlayer_Lucio::OnMyLanded);
	}

	// OnComponentOverlap 이벤트들을 클래스에서 생성한 이벤트와 바인드
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &APlayer_Lucio::OnJumpPointBeginOverlap);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &APlayer_Lucio::OnJumpPointEndOverlap);
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

void APlayer_Lucio::OnMyLanded(const FHitResult& Hit)
{
	if (LandingShake && GetController())
	{
		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->StartCameraShake(LandingShake);
	}
}

void APlayer_Lucio::OnJumpPointBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this))
	{
		if (OtherActor->ActorHasTag(TEXT("JumpPoint")))
		{
			MoveComp->JumpZVelocity = 1400.f;
		}
	}
}

void APlayer_Lucio::OnJumpPointEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && (OtherActor != this))
	{
		if (OtherActor->ActorHasTag(TEXT("JumpPoint")))
		{
			MoveComp->JumpZVelocity = DefaultJumpPower;
		}
	}
}

void APlayer_Lucio::WallRide(float DeltaTime)
{
	// 외적을 이용해 접촉한 벽과 수평인 방향벡터 계산
	FVector SlideDircetion = FVector::CrossProduct(WallRideNormal, FVector(0, 0, 1));

	// 내적을 이용해 진입한 방향과 슬라이딩할 벡터가 같은 방향인지 확인
	float EntryDotProduct = FVector::DotProduct(WallRideEntryVelocity, SlideDircetion);

	// 내적한 값이 양수이면 같은 방향을 반환, 음수이면 반대로 돌려서 반환함 (블루프린트의 select노드와 똑같은 역할)
	FVector TrueSlideDirection = (EntryDotProduct > 0) ? SlideDircetion : -SlideDircetion;

	FVector TargetVelocity = TrueSlideDirection.GetSafeNormal() * WallRideSpeed;
	FVector CurrentVelocity = MoveComp->Velocity;

	FVector WallRideVelocity = FMath::VInterpTo(CurrentVelocity, TargetVelocity, DeltaTime, 100.f);

	bIsWallRiding = true;
	MoveComp->GravityScale = 0.f;
	if (GetActorLocation().Z < ( WallRideEntryZ + 100.f))
	{
		MoveComp->Velocity = FVector(WallRideVelocity.X, WallRideVelocity.Y, FMath::FInterpTo(WallRideEntryZ, WallRideEntryZ + 100.f, DeltaTime, 0.f) - WallRideEntryZ);
	}
	else
	{
		MoveComp->Velocity = FVector(WallRideVelocity.X, WallRideVelocity.Y, 0);
	}
}

void APlayer_Lucio::WallRideCameraTilt()
{
	if (bIsWallRiding)
	{
		
	}
}

void APlayer_Lucio::WallCheck(float DeltaTime)
{
	
	
}


