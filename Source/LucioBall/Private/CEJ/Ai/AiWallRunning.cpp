// Fill out your copyright notice in the Description page of Project Settings.

#include "CEJ/Ai/AiWallRunning.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h" 


// Sets default values
AAiWallRunning::AAiWallRunning()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	GetMesh()->SetRelativeLocation(FVector(0.f, -10.f, 60.f));
	//GetMesh()->SetRelativeScale3D(FVector(47.f));
	
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshRef(
		TEXT("SkeletalMesh'/Game/CEJ/Animations/StaySkateboarding.StaySkateboarding'")
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

	JumpSensor = CreateDefaultSubobject<USphereComponent>(TEXT("JumpSensor"));
	JumpSensor->SetupAttachment(GetCapsuleComponent());
	JumpSensor->InitSphereRadius(JumpSensorRadius);          // 기본 반지름
	JumpSensor->SetRelativeLocation(FVector(0, 0, 40.f));    // 살짝 위로
	JumpSensor->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	JumpSensor->SetCollisionObjectType(ECC_Pawn);
	JumpSensor->SetCollisionResponseToAllChannels(ECR_Ignore);
	// JumpPoint가 보통 WorldStatic/WorldDynamic 이므로 둘만 Overlap
	JumpSensor->SetCollisionResponseToChannel(ECC_WorldStatic,  ECR_Overlap);
	JumpSensor->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	JumpSensor->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void AAiWallRunning::BeginPlay()
{
	Super::BeginPlay();
	
	MoveComp = GetCharacterMovement();
    
	// 센서 반지름 에디터값 반영
	if (JumpSensor) JumpSensor->SetSphereRadius(JumpSensorRadius);

	// 오버랩 이벤트 바인딩
	if (JumpSensor)
	{
		JumpSensor->OnComponentBeginOverlap.AddDynamic(this, &AAiWallRunning::OnJumpSensorBeginOverlap);
		JumpSensor->OnComponentEndOverlap  .AddDynamic(this, &AAiWallRunning::OnJumpSensorEndOverlap);
	}

	// (옵션) 스폰 직후 JumpPoint 위면 자동 점프
	if (IsOnJumpPoint())
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AAiWallRunning::TryAutoJumpFromJumpPoint);
	}

	bPrevUseControllerRotationYaw = bUseControllerRotationYaw;
	bPrevOrientRotationToMovement = GetCharacterMovement()->bOrientRotationToMovement;
	PrevRotationRate = GetCharacterMovement()->RotationRate;

}


// Called every frame
void AAiWallRunning::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	CheckForWall();

	StateMachine();

	if (bIsWallRunning)
	{
		WallRunningMovement();
	}
}

// Called to bind functionality to input
void AAiWallRunning::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AAiWallRunning::CheckForWall()
{
	 if (!GetWorld()) return;

    // 기준 축 (카메라/오리엔테이션이 Pitch되어도 수평만 사용)
    const FVector Right3D = (Orientation ? Orientation->GetRightVector() : GetActorRightVector()).GetSafeNormal();

    FVector Right2D = FVector(Right3D.X, Right3D.Y, 0.f);
    if (!Right2D.Normalize()) return;
    const FVector Left2D = -Right2D;

    // 시작점: 캡슐 중앙보다 약간 위, 그리고 몸에서 살짝 안쪽(반대방향)으로 인셋
    const float Half = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 88.f;
    const FVector Chest = GetActorLocation() + FVector(0,0, Half*0.5f);
    const float Inset = 20.f;

    const FVector StartR = Chest - Right2D * Inset;
    const FVector StartL = Chest - Left2D  * Inset;

    // 태그/법선 필터를 통과한 벽만 채택
    FHitResult HR, HL;
    bWallRight = TraceForWallWithTag(StartR, Right2D, WallCheckDistance, HR);
    bWallLeft  = TraceForWallWithTag(StartL, Left2D,  WallCheckDistance, HL);

    RightWallHit = bWallRight ? HR : FHitResult{};
    LeftWallHit  = bWallLeft  ? HL : FHitResult{};

#if WITH_EDITOR
	if (GEngine) {
		const FString S = FString::Printf(
			TEXT("HasWall=%d (L:%d R:%d)  Air=%d  Forward=%d  Jump=%d"),
			(bWallLeft||bWallRight), bWallLeft, bWallRight, AboveGround(), 
			(bAutoWallRun || VerticalInput>0.f), bHasRecentJump);
		GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, S);
	}
	
    DrawDebugDirectionalArrow(
    	GetWorld(),
    	StartR,
    	StartR + Right2D * WallCheckDistance, //(오/왼)
    	20.f,
    	bWallRight ? FColor::Green : FColor::Red,//(벽 감지되면 초록, 아니면 빨강)
    	false, // 영구 여부 (false → 한 프레임만 그리기)
    	0.f,
    	0,
    	2.f);
	
    DrawDebugDirectionalArrow(
    	GetWorld(),
    	StartL,
    	StartL + Left2D  * WallCheckDistance,
    	40.f,
    	bWallLeft ? FColor::Red : FColor::Black,
    	false,
    	0.f,
    	0,
    	2.f);

    if (bWallRight)
    {
        DrawDebugPoint(GetWorld(), RightWallHit.ImpactPoint, 10.f, FColor::Yellow, false, 0.f, 0);
    	DrawDebugDirectionalArrow(
			GetWorld(),
			RightWallHit.ImpactPoint,                                 // Start
			RightWallHit.ImpactPoint + RightWallHit.ImpactNormal * 60.f, // End
			12.f,                                                     // ArrowSize
			FColor::Yellow,
			false,                                                    // bPersistentLines
			0.f,                                                      // LifeTime
			0,                                                        // DepthPriority
			2.f                                                       // Thickness
		);
    }
    if (bWallLeft)
    {
        DrawDebugPoint(GetWorld(), LeftWallHit.ImpactPoint, 10.f, FColor::Yellow, false, 0.f, 0);
    	DrawDebugDirectionalArrow(
			GetWorld(),
			LeftWallHit.ImpactPoint,
			LeftWallHit.ImpactPoint + LeftWallHit.ImpactNormal * 60.f,
			12.f,
			FColor::Yellow,
			false,
			0.f,
			0,
			2.f
		);
    }
#endif
	
}

// 벽 달리기 시작
void AAiWallRunning::StartWallRun()
{
	bIsWallRunning = true;

	auto* Move = GetCharacterMovement();
	Move->SetMovementMode(MOVE_Flying);      // 걷기→비행(벽에서 미끄러짐)
	Move->GravityScale = 0.f;
	Move->BrakingFrictionFactor = 0.f;
	Move->GroundFriction = 0.f;              // (걷기모드로 돌아올 때만 의미)

	// 벽달리기 동안은 이동방향을 바라보게
	bUseControllerRotationYaw = false;
	Move->bUseControllerDesiredRotation = false;
	Move->bOrientRotationToMovement = true;
	Move->RotationRate = FRotator(0.f, 1440.f, 0.f); // 회전 반응 빠르게
}

// 벽 달리기 종료
void AAiWallRunning::StopWallRun()
{
	bIsWallRunning = false;

	auto* Move = GetCharacterMovement();
	Move->SetMovementMode(MOVE_Walking);
	Move->GravityScale = 1.f;
	Move->BrakingFrictionFactor = 2.f;       // 프로젝트 기본값에 맞게
	Move->GroundFriction = 8.f;

	//캐릭터 방향 원래대로
	bUseControllerRotationYaw = bPrevUseControllerRotationYaw;
	Move->bOrientRotationToMovement = bPrevOrientRotationToMovement;
	Move->RotationRate = PrevRotationRate;
}


// 벽 달리기 이동 로직
void AAiWallRunning::WallRunningMovement()
{
	// 1) 중력 OFF + 수직속도 제거 ( rb.useGravity=false; rb.velocity.y=0)
    UCharacterMovementComponent* Move = GetCharacterMovement();
    Move->GravityScale = 0.0f;

    FVector V = Move->Velocity;
    V.Z = 0.0f;
    Move->Velocity = V;

    // 2) 벽 히트/법선
    const FHitResult& Hit = bWallRight ? RightWallHit : LeftWallHit;
    if (!Hit.bBlockingHit)
    {
        StopWallRun();
        return;
    }
    const FVector WallNormal = Hit.ImpactNormal.GetSafeNormal();

    // 3) 벽 진행 방향 = N × Up  (Vector3.Cross(wallNormal, transform.up))
    FVector WallForward = FVector::CrossProduct(WallNormal, FVector::UpVector).GetSafeNormal();

    // 4) 플레이어 전방과 더 가까운 쪽으로 정렬 (magnitude 비교와 동일)
    const FVector Forward = (Orientation ? Orientation->GetForwardVector()
                                         : GetActorForwardVector()).GetSafeNormal();
    if (FVector::DistSquared(Forward, WallForward) > FVector::DistSquared(Forward, -WallForward))
    {
        WallForward *= -1.f;
    }

    // 5) 벽을 따라 앞으로 미는 힘 (rb.AddForce(wallForward * wallRunForce))
		// (A) 속도 직접 고정
		Move->Velocity = WallForward * WallRunSpeed;
		// (B) 힘으로 밀기
		Move->AddForce(WallForward * WallRunForce * Move->Mass);

    // 6) Push-to-wall
    //  - 왼벽에서 오른쪽(>0) 입력, 오른벽에서 왼쪽(<0) 입력이면 "바깥쪽 의도"
    float AwayInput = 0.f;
    if (bWallLeft)       AwayInput = FMath::Max(0.f,  HorizontalInput);
    else if (bWallRight) AwayInput = FMath::Max(0.f, -HorizontalInput);

    // 입력으로 취소 정도 적용 (0~1)
    AwayInput = FMath::Clamp(AwayInput * PushCancelStrength, 0.f, 1.f);

    // 속도에 따른 스케일(******플레이하면서 값맞춰보자)
    float SpeedScale = 1.f;
    if (PushToWallBySpeed)
    {
        const float Speed = GetVelocity().Size();
        SpeedScale = FMath::Clamp(PushToWallBySpeed->GetFloatValue(Speed), 0.f, 5.f);
    }

    // 최종 힘 = 기본힘 * (1 - AwayInput) * 속도스케일
    const float KeepFactor  = 1.f - AwayInput;    // 1=강하게 붙임, 0=안 붙임
    const float DynamicPush = BasePushToWallForce * KeepFactor * SpeedScale;

    // 바깥쪽으로 스티어 중이 아니면 벽쪽으로 힘을 준다
    if (!(bWallLeft && HorizontalInput > 0.f) && !(bWallRight && HorizontalInput < 0.f))
    {
        Move->AddForce(-WallNormal * DynamicPush);
    }

#if WITH_EDITOR
    DrawDebugDirectionalArrow(
    	GetWorld(),
    	GetActorLocation(),
        GetActorLocation() + WallForward * 120.f,
        30.f,
        FColor::Green,
        false,
        0.f,
        0,
        2.f);
	
    DrawDebugLine(GetWorld(),
    	Hit.ImpactPoint,
    	Hit.ImpactPoint + (-WallNormal) * 60.f,
		FColor::Cyan,
		false,
		0.f,
		0,
		2.f);
#endif
}

void AAiWallRunning::StateMachine()
{
	const bool bHasWall   = (bWallLeft || bWallRight);
	const bool bAirEnough = AboveGround();
	const bool bWantsForward = bAutoWallRun ? true : (VerticalInput > 0.f);// AI면 입력 없이 자동진행

	// 점프 요구 조건 추가
	const bool bJumpGateOK = !bRequireJumpToStart || bHasRecentJump;

	if (bHasWall && bWantsForward && bAirEnough && bJumpGateOK)
	{
		if (!bIsWallRunning) StartWallRun();
	}
	else if (bIsWallRunning)
	{
		StopWallRun();
	}
}

bool AAiWallRunning::AboveGround() const
{
	//AboveGround() – 보통 레이캐스트로 바닥까지 거리 체크
	// 언리얼: LineTrace로 캡슐 아래로 사선/수직 레이 캐스팅
	if (!GetWorld()) return false;

	const FVector Start = GetActorLocation();
	const float CapsuleHalf = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 88.f;

	const float CheckDistance = CapsuleHalf + 150.f; // 바닥과 여유 거리(원하는 값으로 조정)
	const FVector End = Start - FVector::UpVector * CheckDistance;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AboveGround), false, this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params);

#if WITH_EDITOR
	DrawDebugDirectionalArrow(
		GetWorld(),
		Start,                 // 시작점
		End,                   // 끝점
		20.f,                  // 화살촉 크기
		bHit ? FColor::Green : FColor::Red, // 맞았으면 초록, 아니면 빨강
		false,                 // 지속 여부 (false면 한 프레임만)
		0.f,                   // 지속 시간
		0,                     // Depth priority
		2.f                    // 선 두께
	);
#endif

	// 바닥에 너무 가깝지 않고 "공중"이라고 판단하려면 false 반환,
	// 벽달리기 튜토리얼들에선 "지면에서 일정 높이 이상"을 요구하므로
	// 여기선 "지면과 일정 거리 이상 떨어져 있으면 true"로 처리
	return !bHit; // Unity 예제의 AboveGround()와 동일한 의도: 공중이면 true
}

void AAiWallRunning::FixedUpdate()
{
	if (bIsWallRunning)
	{
		WallRunningMovement();
	}
}

// 태그와 법선으로 필터링해서 벽 탐지
bool AAiWallRunning::TraceForWallWithTag(const FVector& Start, const FVector& Dir, float Distance, FHitResult& OutHit) const
{
	FCollisionQueryParams Params(SCENE_QUERY_STAT(WallCheck), false, this);

	// 스피어 스윕(모서리/울퉁불퉁 완화). 선호 안 하면 LineTraceMulti를 써도 됨
	TArray<FHitResult> Hits;
	const bool bAny = GetWorld()->SweepMultiByChannel(
		Hits,
		Start,
		Start + Dir * Distance,
		FQuat::Identity,
		TraceChannel,                               // 기존 채널 그대로
		FCollisionShape::MakeSphere(12.f),          // 반지름 12cm 정도
		Params
	);
	if (!bAny) return false;

	// 거리 순서대로 정렬(가까운 것부터)
	Hits.Sort([](const FHitResult& A, const FHitResult& B)
	{
		return A.Distance < B.Distance;
	});

	for (const FHitResult& H : Hits)
	{
		if (!H.bBlockingHit) continue;

		// 1) 태그 검사: Actor가 있고 WallTag를 갖고 있어야 함
		const AActor* A = H.GetActor();
		if (!A || !A->ActorHasTag(WallTag)) continue;

		// 2) 법선 검사: 바닥/천장 제외 (Up과의 내적이 작을수록 벽)
		const FVector N = H.ImpactNormal.GetSafeNormal();
		const float UpDot = FMath::Abs(FVector::DotProduct(N, FVector::UpVector));
		if (UpDot >= MaxUpDotForWall) continue;

		OutHit = H;
		return true; // 이 히트를 벽으로 채택
	}

	return false;
}

void AAiWallRunning::TryAutoJumpFromJumpPoint()
{
	// 지면 위면 정상 점프, 공중이면 Launch로 보정
	if (MoveComp && MoveComp->IsMovingOnGround())
	{
		Jump();            // ACharacter::Jump → OnJumped_Implementation 호출됨
	}
	else
	{
		LaunchCharacter(FVector(0,0,AutoJumpZ), false, true);
		MarkRecentJump();  // LaunchCharacter는 OnJumped가 안 불리므로 수동 마킹
	}
}

void AAiWallRunning::MarkRecentJump()
{
	bHasRecentJump = true;
	GetWorldTimerManager().ClearTimer(RecentJumpResetHandle);
	GetWorldTimerManager().SetTimer(
		RecentJumpResetHandle, this, &AAiWallRunning::ClearRecentJump, RecentJumpWindow, false);

}

void AAiWallRunning::ClearRecentJump()
{
	bHasRecentJump = false;
}

void AAiWallRunning::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	MarkRecentJump();
}

void AAiWallRunning::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	ClearRecentJump(); // 착지하면 윈도우 종료

}

bool AAiWallRunning::IsOnJumpPoint() const
{
	if (!GetWorld()) return false;

	// 발 밑 아주 작은 구체로 오버랩 → JumpPoint Tag 보유 액터 찾기
	const FVector C = GetActorLocation();
	const float Radius = 60.f;
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(IsOnJumpPoint), false, this);

	const bool bAny = GetWorld()->OverlapMultiByChannel(
		Overlaps,
		C,
		FQuat::Identity,
		ECC_WorldStatic,
		FCollisionShape::MakeSphere(Radius),
		Params
	);

	if (!bAny) return false;

	for (const auto& O : Overlaps)
	{
		const AActor* A = O.GetActor();
		if (A && A->ActorHasTag(JumpPointTag))
		{
			return true;
		}
	}
	return false;
}

void AAiWallRunning::OnJumpSensorBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bAutoJumpFromSensor || !OtherActor || OtherActor == this) return;

	bool bIsJumpPoint = false;

	// 태그로 판정
	if (bUseJumpPointTag && OtherActor->ActorHasTag(JumpPointTag))
	{
		bIsJumpPoint = true;
	}

	// 클래스(BP)로 판정 (에디터에서 BP_JumpPoint 할당)
	if (!bIsJumpPoint && JumpPointClass && OtherActor->IsA(JumpPointClass))
	{
		bIsJumpPoint = true;
	}

	if (!bIsJumpPoint) return;

	// 점프 실행 (지상이면 Jump, 공중이면 Launch로 보정)
	if (MoveComp && MoveComp->IsMovingOnGround())
	{
		Jump();          // ACharacter::Jump
	}
	else
	{
		LaunchCharacter(FVector(0, 0, AutoJumpZ), false, true);
	}
	MarkRecentJump();   // 벽타기 게이트용 "최근 점프" ON

#if WITH_EDITOR
	UE_LOG(LogTemp, Log, TEXT("[JumpSensor] BeginOverlap with JumpPoint -> JUMP"));
#endif
}

void AAiWallRunning::OnJumpSensorEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
#if WITH_EDITOR
	UE_LOG(LogTemp, Log, TEXT("[JumpSensor] EndOverlap"));
#endif
}


