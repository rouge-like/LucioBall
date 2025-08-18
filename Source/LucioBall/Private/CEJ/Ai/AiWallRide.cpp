// Fill out your copyright notice in the Description page of Project Settings.

#include "CEJ/Ai/AiWallRide.h"
#include "CEJ/Ai/WallRideComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AAiWallRide::AAiWallRide()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WallRideComp = CreateDefaultSubobject<UWallRideComponent>(TEXT("WallRideComp"));

	// 캡슐 크기만 50배 (메시는 그대로 유지)
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	Capsule->SetCapsuleSize(42.f * 50.f, 96.f * 50.f, true);

	// 캡슐 충돌 설정
	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Capsule->SetCollisionObjectType(ECC_Pawn);
	Capsule->SetCollisionResponseToAllChannels(ECR_Block);
	Capsule->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Capsule->SetCollisionResponseToChannel(ECC_Pawn,   ECR_Ignore);

	// 메시 크기/위치 그대로 (충돌은 끔)
	GetMesh()->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	GetMesh()->SetRelativeLocation(FVector(0.f, -10.f, 60.f));
	GetMesh()->SetRelativeScale3D(FVector(47.f));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshRef(
		TEXT("SkeletalMesh'/Game/CEJ/Animations/Skateboarding.Skateboarding'")
	);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> OverlayMatRef(
		TEXT("Material'/Game/CEJ/Asset/lucio_default_EMr_Mat.lucio_default_EMr_Mat'")
	);

	if (MeshRef.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshRef.Object);
		if (OverlayMatRef.Succeeded())
		{
			GetMesh()->SetOverlayMaterial(OverlayMatRef.Object);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("mesh material 경로 확인 필요"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("mesh 없음~ mesh 경로 확인!"));
	}
}

void AAiWallRide::BeginPlay()
{
	Super::BeginPlay();
	//월라이드 중엔 캐릭터무브먼트의 중력을 끌 거라 필요 없음 
}

bool AAiWallRide::FindWall(FVector& OutNormal, FVector& OutTangent)
{
	const FVector Start = GetActorLocation() + GetActorUpVector()*50.f;   // 가슴 높이
	const FVector End   = Start + GetActorForwardVector()*140.f;          // 전방 140

	FHitResult Hit;
	FCollisionQueryParams P(NAME_None, false, this);
	// 스피어트레이스가 모서리에서 안정적
	const float Radius = 20.f;
	const bool bHit = GetWorld()->SweepSingleByChannel(
		Hit, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius), P
	);

	if (!bHit) return false;

	const FVector N = Hit.ImpactNormal.GetSafeNormal();
	const FVector F = GetActorForwardVector();
	const float   d = FVector::DotProduct(F, N);     // -1 ~ +1

	// 벽으로 적절히 접근 중 
	if (d > -0.2f || d < -0.9f) return false;

	// 접선 구하고, 좌/우 정렬 (캐릭터 기준 오른쪽과 같은 쪽이면 유지)
	FVector T = FVector::CrossProduct(N, FVector::UpVector);
	if (!T.Normalize()) return false;
	const float side = FVector::DotProduct(GetActorRightVector(), T);
	if (side < 0.f) T *= -1.f;

	OutNormal  = N;
	OutTangent = T;
	return true;
}


// Called every frame
void AAiWallRide::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!WallRideComp) return;

	if (WallRideComp->bWallRiding)
	{
		FVector N, T;
		if (!FindWall(N, T))
		{
			// 기존: NoCollision → 충돌 끄면 움직임/지면 접촉이 꼬임
			// 항상 원복은 Block 상태로
			UCapsuleComponent* Capsule = GetCapsuleComponent();
			Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Capsule->SetCollisionResponseToAllChannels(ECR_Block);
			Capsule->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
			Capsule->SetCollisionResponseToChannel(ECC_Pawn,   ECR_Ignore);

			// 상태 전환
			//WallRideComp->EndWallRide(); // 필요 시 유지
			GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		}
		return;
	}

	// 진입 로직
	{
		FVector N, T;
		if (GetCharacterMovement()->IsFalling() && FindWall(N, T))
		{
			const float V0 = GetVelocity().Size();
			const float Vtarget = WallRideComp->VMaxWall;

			// 진입 직전에도 캡슐 Block 유지
			UCapsuleComponent* Capsule = GetCapsuleComponent();
			Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Capsule->SetCollisionResponseToAllChannels(ECR_Block);
			Capsule->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
			Capsule->SetCollisionResponseToChannel(ECC_Pawn,   ECR_Ignore);

			WallRideComp->BeginWallRide(N, T, V0, Vtarget);
			GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		}
	}
}

// Called to bind functionality to input
void AAiWallRide::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AAiWallRide::OnJumpPressed);
}

//벽타기 점프 
void AAiWallRide::OnJumpPressed()
{
	if (WallRideComp && (WallRideComp->bWallRiding || WallRideComp->bWallExit))
	{
		// 벽점프: 법선/접선/업 혼합
		const FVector N = WallRideComp->WallNormal;
		const FVector T = FVector::CrossProduct(N, FVector::UpVector).GetSafeNormal();
		FVector JumpDir = (N*0.6f + T*0.3f + FVector::UpVector*0.2f).GetSafeNormal();

		LaunchCharacter(JumpDir * 900.f, true, true);

		WallRideComp->EndWallRide();
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		return;
	}

	Jump(); // 평상시 점프
}

