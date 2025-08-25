// Fill out your copyright notice in the Description page of Project Settings.


#include "CEJ/Ai/AiLucio.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h" 

// Sets default values
AAiLucio::AAiLucio()
{
    PrimaryActorTick.bCanEverTick = true;

    AutoPossessAI     = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AAIController::StaticClass();

    GetCharacterMovement()->JumpZVelocity = 800.f;
    GetCharacterMovement()->GravityScale  = 1.0f;
    GetCharacterMovement()->AirControl    = 0.6f;

    GetMesh()->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
    GetMesh()->SetRelativeLocation(FVector(0.f, -10.f, 60.f));
    GetMesh()->SetRelativeScale3D(FVector(47.f));
    
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshRef(
        TEXT("SkeletalMesh'/Game/CEJ/Animations/Stay.Stay'")
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
    
    GetMesh()->SetAnimationMode(EAnimationMode::Type::AnimationSingleNode);

    static ConstructorHelpers::FObjectFinder<UAnimSequence> StayAnimRef(
        TEXT("AnimSequence'/Game/CEJ/Animations/Stay_Anim.Stay_Anim'")
    );
    if (StayAnimRef.Succeeded()) StayAnim = StayAnimRef.Object;

    static ConstructorHelpers::FObjectFinder<UAnimSequence> MoveAnimRef(
        TEXT("AnimSequence'/Game/CEJ/Animations/Skateboarding_Anim.Skateboarding_Anim'")
    );
    if (MoveAnimRef.Succeeded()) MoveAnim = MoveAnimRef.Object;

    // 시작은 Idle로
    if (StayAnim)
    {
        GetMesh()->PlayAnimation(StayAnim, true);
        CurrentAnim = StayAnim;
    }
    
    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);

}

void AAiLucio::BeginPlay()
{
    Super::BeginPlay();

    CachedAI = Cast<AAIController>(GetController());  
    FindJumpPointOnce();
    FindBallOnce();
    FindGoalOnce();

    GotoState(ELucioAIState::SeekJumpPoint);
}

void AAiLucio::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    CachedAI = Cast<AAIController>(NewController);
}

void AAiLucio::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    MoveCmdCooldown = FMath::Max(0.f, MoveCmdCooldown - DeltaSeconds);

    switch (State)
    {
        case ELucioAIState::SeekJumpPoint: Tick_SeekJumpPoint(DeltaSeconds); break;
        case ELucioAIState::Jumping:       Tick_Jumping(DeltaSeconds);       break;
        case ELucioAIState::WallRun:       Tick_WallRun(DeltaSeconds);       break;
        case ELucioAIState::FallingFast:   Tick_FallingFast(DeltaSeconds);   break;
        case ELucioAIState::BallKick:      Tick_BallKick(DeltaSeconds);      break;
        default: break;
    }

    if (!MoveAnim) return;

    const float Speed2D = GetVelocity().Size2D();
    const float MoveThreshold = 10.f; // 멈춤/이동 판단 임계값

    if (Speed2D > MoveThreshold)
    {
        // 이동 중: 애님이 안 돌고 있으면 루프 재생
        if (!GetMesh()->IsPlaying())
        {
            GetMesh()->PlayAnimation(MoveAnim, /*bLoop*/ true);
        }
    }
    else
    {
        // 정지 상태: 애님 정지(또는 Idle 애님으로 교체)
        if (GetMesh()->IsPlaying())
        {
            GetMesh()->Stop();
            // Idle 애님이 있으면 여기서 PlayAnimation(IdleAnim, true) 등으로 전환
        }
    }
    

}

// 상태 전이
void AAiLucio::GotoState(ELucioAIState NewState)
{
    State = NewState;

    // 낙하 강도 조절
    if (State == ELucioAIState::FallingFast)
        GetCharacterMovement()->GravityScale = FastFallGravityScale;
    else
        GetCharacterMovement()->GravityScale = NormalGravityScale;
}

// 탐색 유틸
void AAiLucio::FindJumpPointOnce()
{
    if (JumpPoint.IsValid()) return;

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), JumpPointTag, Found);
    if (Found.Num() > 0)
        JumpPoint = Found[0];
}

void AAiLucio::FindBallOnce()
{
    if (BallActor.IsValid()) return;

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), BallTag, Found);
    if (Found.Num() > 0)
        BallActor = Found[0];
}

void AAiLucio::FindGoalOnce()
{
    if (GoalActor.IsValid()) return;

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), GoalTag, Found);
    if (Found.Num() > 0)
        GoalActor = Found[0];
}

void AAiLucio::Tick_SeekJumpPoint(float Dt)
{
    if (!JumpPoint.IsValid())
    {
        FindJumpPointOnce();
        if (!JumpPoint.IsValid())
        {
            // 점프포인트 없으면 볼 킥으로
            GotoState(ELucioAIState::BallKick);
            return;
        }
    }

    const FVector MyLoc = GetActorLocation();
    const FVector JPLoc = JumpPoint->GetActorLocation();
    const float Dist2D  = FVector::Dist2D(MyLoc, JPLoc);

    // 이동 명령 (컨트롤러가 없으면 다음 틱 재시도)
    MoveToActorSmart(JumpPoint.Get(), AcceptanceRadius);

    // 근접 시 점프
    if (Dist2D <= FMath::Max(ApproachTolerance, AcceptanceRadius * 1.3f))
    {

        Jump(); // ACharacter::Jump()는 void (반환값 없음)
        
        if (USkeletalMeshComponent* MeshComp = GetMesh())
        {
            if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
            {
                if (UAnimSequence* JumpAnim = LoadObject<UAnimSequence>(nullptr,
                    TEXT("/Script/Engine.AnimSequence'/Game/CEJ/Animations/Jumping_Anim.Jumping_Anim'")))
                {
                    MeshComp->PlayAnimation(JumpAnim, false);
                }
            }
        }

        GotoState(ELucioAIState::Jumping);

        
    }
}

void AAiLucio::Tick_Jumping(float Dt)
{
    // 전방으로 스피어 트레이스해서 벽 찾기
    const FVector Start  = GetActorLocation();
    const FVector Dir    = GetActorForwardVector();
    FHitResult Hit;

    if (TraceForWall(Start, Dir, WallTraceLength, Hit))
    {
        if (AActor* HitActor = Hit.GetActor())
        {
            if (HitActor->ActorHasTag(WallTag))
            {
                bHasWall   = true;
                WallNormal = Hit.ImpactNormal.GetSafeNormal();
                WallTangent = ComputeWallTangent(WallNormal);

                // 벽 쪽으로 살짝 붙이기
                AddActorWorldOffset(-WallNormal * StickStrength, true);

                // 현재 속도를 벽 접선에 투영해 달리기 시작
                const FVector V      = GetVelocity();
                const FVector Vt     = FVector::VectorPlaneProject(V, WallNormal);
                const FVector VtDir  = Vt.GetSafeNormal();
                const float   Speed  = FMath::Clamp(Vt.Size(), 600.f, MaxWallSpeed);

                LaunchCharacter(VtDir * Speed, true, true);

                GotoState(ELucioAIState::WallRun);
                return;
            }
        }
    }

    // 착지하면 볼킥으로
    if (GetCharacterMovement()->IsMovingOnGround())
    {
        GotoState(ELucioAIState::BallKick);
    }

    const float Speed = GetVelocity().Size();
    UpdateLocomotionAnim(Speed);
}

void AAiLucio::Tick_WallRun(float Dt)
{
    if (!bHasWall)
    {
        GotoState(ELucioAIState::FallingFast);
        return;
    }

    // 진행 방향으로 계속 벽이 있는지 확인
    FHitResult FwdHit;
    const FVector ProbeStart = GetActorLocation() - WallNormal * 30.f;
    const bool bStillWallAhead = TraceForWall(ProbeStart, WallTangent, WallEndProbe, FwdHit);

    // 벽쪽으로 약하게 스티킹
    AddActorWorldOffset(-WallNormal * StickStrength * Dt, true);

    // 접선 방향으로 속도 유지
    const FVector DesiredVel = WallTangent.GetSafeNormal() * MaxWallSpeed;
    const FVector Curr = GetVelocity();
    const FVector NewVel = FMath::VInterpTo(Curr, DesiredVel, Dt, 2.5f);
    LaunchCharacter(NewVel, true, true);

    if (!bStillWallAhead)
    {
        GotoState(ELucioAIState::FallingFast);
    }
}

void AAiLucio::Tick_FallingFast(float Dt)
{
    if (GetCharacterMovement()->IsMovingOnGround())
    {
        GotoState(ELucioAIState::BallKick);
    }
}

void AAiLucio::Tick_BallKick(float Dt)
{
    if (!BallActor.IsValid()) { FindBallOnce(); if (!BallActor.IsValid()) { GotoState(ELucioAIState::SeekJumpPoint); return; } }
    if (!GoalActor.IsValid())  FindGoalOnce();

    const FVector Ball = BallActor->GetActorLocation();
    const FVector Goal = GoalActor.IsValid() ? GoalActor->GetActorLocation()
                                             : (Ball + GetActorForwardVector() * 100.f);

    // 1) 항상 공을 바짝 추적 (StopOnOverlap=false 권장, 아래 3) 참고)
    MoveToActorSmart(BallActor.Get(), /*StopRadius*/ 25.f);

    // 2) 가까우면 무조건 킥 (공이 정지해 있어도 킥함)
    const float DistToBall = FVector::Dist2D(GetActorLocation(), Ball);
    if (DistToBall <= KickTriggerDistance)           // e.g. 50~80cm
    {
        if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(BallActor->GetRootComponent()))
        {
            if (Prim->IsSimulatingPhysics())
            {
                const FVector DirToGoal = (Goal - Ball).GetSafeNormal();
                const float   KickImpulse = 80000.f;  // 필요 시 조절 (쿨타임이 없으니 수치 낮추는 걸 권장)
                Prim->AddImpulseAtLocation(DirToGoal * KickImpulse, Ball);

                if (bDebug)
                {
                    DrawDebugDirectionalArrow(GetWorld(), Ball, Ball + DirToGoal * 500.f,
                                              80.f, FColor::Cyan, false, 0.15f, 0, 6.f);
                    GEngine->AddOnScreenDebugMessage(3001, 0.f, FColor::Green,
                        FString::Printf(TEXT("KICK Dist=%.1fcm Imp=%.0f"), DistToBall, KickImpulse));
                }
            }
        }
    }
}

bool AAiLucio::MoveToActorSmart(AActor* Target, float Radius)
{
    if (!Target) return false;

    // 컨트롤러 확보(BeginPlay 타이밍 이슈 대응)
    AAIController* AICon = CachedAI ? CachedAI : Cast<AAIController>(GetController());
    if (!AICon) return false;
    CachedAI = AICon;

    if (MoveCmdCooldown > 0.f) return true;
    MoveCmdCooldown = MoveCmdInterval;

    const EPathFollowingRequestResult::Type Res =
        AICon->MoveToActor(Target,
                            Radius, /*StopOnOverlap*/ false,
                           /*UsePathfinding*/ true, /*CanStrafe*/ true,
                           /*FilterClass*/ nullptr, /*AllowPartialPath*/ true);

    const bool bOk =
        (Res == EPathFollowingRequestResult::RequestSuccessful) ||
        (Res == EPathFollowingRequestResult::AlreadyAtGoal);

    if (!bOk)
    {
        UE_LOG(LogTemp, Warning, TEXT("MoveToActor not successful (Res=%d). Check NavMesh/obstacles."), (int)Res);
    }
    return bOk;
}

bool AAiLucio::MoveToLocationSmart(const FVector& Dest, float Radius)
{
    AAIController* AICon = CachedAI ? CachedAI : Cast<AAIController>(GetController());
    if (!AICon) return false;
    CachedAI = AICon;

    if (MoveCmdCooldown > 0.f) return true;
    MoveCmdCooldown = MoveCmdInterval;

    const EPathFollowingRequestResult::Type Res =
        AICon->MoveToLocation(Dest, Radius, /*StopOnOverlap*/ true,
                              /*UsePathfinding*/ true, /*ProjectGoal*/ true,
                              /*CanStrafe*/ true, /*FilterClass*/ nullptr,
                              /*AllowPartialPath*/ true);

    const bool bOk =
        (Res == EPathFollowingRequestResult::RequestSuccessful) ||
        (Res == EPathFollowingRequestResult::AlreadyAtGoal);

    if (!bOk)
    {
        UE_LOG(LogTemp, Warning, TEXT("MoveToLocation not successful (Res=%d). Check NavMesh/obstacles."), (int)Res);
    }
    return bOk;
}

bool AAiLucio::TraceForWall(const FVector& From, const FVector& Dir, float Length, FHitResult& OutHit) const
{
    const FVector End = From + Dir.GetSafeNormal() * Length;

    TArray<AActor*> Ignore;
    Ignore.Add(const_cast<AAiLucio*>(this));

    const bool bHit = UKismetSystemLibrary::SphereTraceSingle(
        const_cast<AAiLucio*>(this),
        From, End,
        WallTraceRadius,
        UEngineTypes::ConvertToTraceType(ECC_Visibility),
        /*bTraceComplex*/ true,
        Ignore,
        EDrawDebugTrace::None,
        OutHit,
        /*bIgnoreSelf*/ true
    );

    if (bDebug)
    {
        if (bHit)
        {
            DrawDebugLine(GetWorld(), From, OutHit.ImpactPoint, FColor::Red, false, 0.f, 0, 2.f);
            DrawDebugPoint(GetWorld(), OutHit.ImpactPoint, 14.f, FColor::Red, false, 0.f);
        }
        else
        {
            DrawDebugLine(GetWorld(), From, End, FColor::Green, false, 0.f, 0, 2.f);
        }
    }

    return bHit;
}

FVector AAiLucio::ComputeWallTangent(const FVector& InWallNormal, const FVector& Up /*=FVector::UpVector*/) const
{
    FVector T = FVector::CrossProduct(Up, InWallNormal).GetSafeNormal();
    if (T.IsNearlyZero())
    {
        const FVector Fwd = GetActorForwardVector();
        T = FVector::VectorPlaneProject(Fwd, InWallNormal).GetSafeNormal();
    }
    return T;
}

void AAiLucio::ComputeBallApproach(const FVector& Ball, const FVector& Goal,
                                   FVector& OutApproach, FVector& OutKick, FVector& OutDirToGoal) const
{
    OutDirToGoal = (Goal - Ball).GetSafeNormal();
    OutApproach  = Ball - OutDirToGoal * BehindBallDistance;
    OutKick      = Ball + OutDirToGoal * KickThroughDistance;
}

void AAiLucio::UpdateLocomotionAnim(float Speed)
{
    const bool bIdle = Speed < 0.f;
    UAnimSequence* Desired = bIdle ? StayAnim : MoveAnim;

    if (Desired && Desired != CurrentAnim &&
        GetMesh()->GetAnimationMode() == EAnimationMode::AnimationSingleNode)
    {
        GetMesh()->PlayAnimation(Desired, /*bLoop=*/true);
        CurrentAnim = Desired;
    }
}

void AAiLucio::FindFieldOnce()
{
    if (FieldActor.IsValid()) return;

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FieldTag, Found);
    if (Found.Num() > 0)
        FieldActor = Found[0];
}

FVector AAiLucio::GetFieldCenter() const
{
    if (FieldActor.IsValid())
    {
        FVector Origin, Extent;
        FieldActor->GetActorBounds(
            /*bOnlyCollidingComponents*/ true,
            Origin,
            Extent);
        return Origin; // GetActorBounds의 Origin이 바운딩 박스 중앙
    }
    // 필드가 없으면 월드 원점 fallback
    return FVector::ZeroVector;
}

bool AAiLucio::IsInAttackingHalf(const FVector& Point) const
{
    if (!GoalActor.IsValid()) return true; // 골대를 못 찾으면 공격 쪽으로 처리
    const FVector G = GoalActor->GetActorLocation();
    const FVector C = GetFieldCenter();

    const FVector Axis = (C - G);
    const float   Len  = Axis.Size();
    if (Len < KINDA_SMALL_NUMBER) return true;

    const FVector Dir = Axis / Len;
    const float tPoint = FVector::DotProduct(Point - G, Dir);    // Goal 기준 진행도
    const float tHalf  = Len * 0.5f;                             // 절반 지점
    return tPoint >= tHalf;
}

void AAiLucio::AttackFSM_TickBall(float Dt)
{
    if (!BallActor.IsValid()) FindBallOnce();
    if (!GoalActor.IsValid()) FindGoalOnce();
    if (!BallActor.IsValid() || !GoalActor.IsValid())
    {
        GotoState(ELucioAIState::SeekJumpPoint);
        return;
    }

    const FVector MyLoc = GetActorLocation();
    const FVector Ball  = BallActor->GetActorLocation();
    const FVector Goal  = GoalActor->GetActorLocation();

    // ── AI → 골대 벡터 (0이 아니면 유효) ─────────────────────────────
    const FVector MyToGoal = Goal - MyLoc;
    const bool bHasGoalDir = !MyToGoal.IsNearlyZero(); // 길이 0 아님?

    // 항상 공을 바짝 따라감 (StopOnOverlap=false 권장)
    MoveToActorSmart(BallActor.Get(), /*StopRadius*/ 25.f);

    if (!bHasGoalDir) return; // 방향이 없으면 아무 것도 안 함

    // 가까우면 즉시 킥 (공이 안 움직여도 킥)
    // 필요하면 프로젝트에 맞게 값 조정
    constexpr float KickTriggerDistanceCm = 70.f; // AI-공 70cm 이내면 킥
    constexpr float KickImpulseStrength   = 80000.f;

    const float DistToBall = FVector::Dist2D(MyLoc, Ball);
    if (DistToBall <= KickTriggerDistanceCm)
    {
        if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(BallActor->GetRootComponent()))
        {
            if (Prim->IsSimulatingPhysics())
            {
                // 공을 골 안쪽으로 밀기: 수직 성분 제거(Z=0)로 안정적 밀기
                FVector DirToGoal = Goal - Ball;
                DirToGoal.Z = 0.f;
                if (!DirToGoal.IsNearlyZero())
                {
                    DirToGoal = DirToGoal.GetSafeNormal();
                    Prim->AddImpulseAtLocation(DirToGoal * KickImpulseStrength, Ball);

                    if (bDebug)
                    {
                        DrawDebugDirectionalArrow(GetWorld(), Ball, Ball + DirToGoal * 500.f,
                                                  80.f, FColor::Cyan, false, 0.15f, 0, 6.f);
                        GEngine->AddOnScreenDebugMessage(
                            3001, 0.f, FColor::Green,
                            FString::Printf(TEXT("KICK  Dist=%.1fcm  Imp=%.0f"),
                                            DistToBall, KickImpulseStrength));
                    }
                }
            }
        }
    }
}

