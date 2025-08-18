// Fill out your copyright notice in the Description page of Project Settings.


#include "CEJ/Ai/AiLucio.h"
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

    GetMesh()->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
    GetMesh()->SetRelativeLocation(FVector(0.f, -10.f, 60.f));
    GetMesh()->SetRelativeScale3D(FVector(47.f));

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

void AAiLucio::BeginPlay()
{
    Super::BeginPlay();

    CachedAI = Cast<AAIController>(GetController());  // 아직 없을 수도 있음(=null)
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

        // 필요시 보조 런치(과하면 주석 처리)
        // LaunchCharacter(FVector(0,0,GetCharacterMovement()->JumpZVelocity), false, true);

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
    if (!BallActor.IsValid()) FindBallOnce();
    if (!GoalActor.IsValid()) FindGoalOnce();

    if (!BallActor.IsValid())
    {
        // 볼이 없으면 다시 점프포인트 찾기 루프로
        GotoState(ELucioAIState::SeekJumpPoint);
        return;
    }

    const FVector Ball = BallActor->GetActorLocation();
    const FVector Goal = GoalActor.IsValid()
        ? GoalActor->GetActorLocation()
        : (Ball + GetActorForwardVector() * 10.f);

    FVector Approach, Kick, DirToGoal;
    ComputeBallApproach(Ball, Goal, Approach, Kick, DirToGoal);

    if (bDebug)
    {
        DrawDebugLine(GetWorld(), Ball, Goal, FColor::Green, false, 0.f, 0, 2.f);
        DrawDebugSphere(GetWorld(), Approach, 18.f, 12, FColor::Cyan, false, 0.f, 0, 1.5f);
        DrawDebugSphere(GetWorld(), Kick,     18.f, 12, FColor::Green,false, 0.f, 0, 1.5f);
    }

    const float DistToApproach = FVector::Dist2D(GetActorLocation(), Approach);
    if (DistToApproach > ApproachTolerance)
    {
        MoveToLocationSmart(Approach, AcceptanceRadius);
    }
    else
    {
        MoveToLocationSmart(Kick, AcceptanceRadius);

        // 볼 근접 시 임펄스(물리 ON 전제)
        if (FVector::Dist2D(GetActorLocation(), Ball) < 10.f)
        {
            if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(BallActor->GetRootComponent()))
            {
                if (Prim->IsSimulatingPhysics())
                {
                    Prim->AddImpulse(DirToGoal * 80000.f); // 맵 스케일에 맞게 조정
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
        AICon->MoveToActor(Target, Radius, /*StopOnOverlap*/ true,
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

