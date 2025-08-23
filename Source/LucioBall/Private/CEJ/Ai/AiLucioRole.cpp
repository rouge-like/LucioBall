#include "CEJ/Ai/AiLucioRole.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Navigation/PathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "OSC/BouncyBall.h"


AAiLucioRole::AAiLucioRole()
{
    PrimaryActorTick.bCanEverTick = true;
    AIControllerClass = AAIController::StaticClass();

    GetMesh()->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
    GetMesh()->SetRelativeLocation(FVector(0.f, -10.f, 60.f));
    GetMesh()->SetRelativeScale3D(FVector(47.f));

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshRef(
        TEXT("SkeletalMesh'/Game/CEJ/Animations/Stay.Stay'")
    );
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> OverlayMatRef(
        TEXT("Material'/Game/CEJ/Animations/lucio_default_color_tga_Mat.lucio_default_color_tga_Mat'")
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

void AAiLucioRole::BeginPlay()
{
    Super::BeginPlay();
    CachedAI = Cast<AAIController>(GetController());
    EnsureTargets();
    State = ELucioState::Idle;
}

void AAiLucioRole::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    EnsureTargets();
    if (!BallActor) return;

    const bool bBallOwn = IsBallInOwnHalf();
    const bool bBallOpp = IsBallInOppHalf();

    switch (AgentRole) 
    {
    case ELucioRole::Offense:
        // ── 공격 FSM ───────────────────────────
        if (bBallOpp)
        {
            // 상대 하프에 공이 있으면 적극 추적 -> 드리블/슛
            switch (State)
            {
            case ELucioState::Idle:
                State = ELucioState::SeekBall;
                break;

            case ELucioState::SeekBall:
            {
                MoveTowardBall(DeltaSeconds);
                const float d = FVector::Dist(GetActorLocation(), BallActor->GetActorLocation());
                if (d <= PossessionRadius)
                    State = ELucioState::Dribble;
                break;
            }
            case ELucioState::Dribble:
            {
                // 공을 상대 골대로 밀기 (약한 임펄스)
                const FVector Goal = GetOppGoalLocation();
                KickTowards(Goal, DribbleKickImpulse);

                // 골과의 거리 가까우면 슛
                const float GoalDist = FVector::Dist(BallActor->GetActorLocation(), Goal);
                if (GoalDist < 1500.f)
                    State = ELucioState::Shoot;
                break;
            }
            case ELucioState::Shoot:
            {
                const FVector Goal = GetOppGoalLocation();
                KickTowards(Goal, KickImpulse_Shoot);
                State = ELucioState::SeekBall; // 다시 추적
                break;
            }
            default:
                State = ELucioState::SeekBall; break;
            }
        }
        else
        {
            // 공이 우리 하프에 있거나 중앙이라면 자리 잡기(대기)
            State = ELucioState::Idle;
            StopChasing();
        }
        break;

    case ELucioRole::Defense:
        // ── 수비 FSM ───────────────────────────
        if (bBallOwn)
        {
            switch (State)
            {
            case ELucioState::Idle:
            case ELucioState::HoldShape:
                State = ELucioState::Intercept;
                break;

            case ELucioState::Intercept:
            {
                // 공의 예상 착지점 선점 (없으면 블록 포인트)
                const FVector P = ComputeInterceptPoint();
                MoveToPoint(P);

                const float d = FVector::Dist(GetActorLocation(), BallActor->GetActorLocation());
                if (d <= PossessionRadius)
                    State = ELucioState::ClearBall;
                break;
            }

            case ELucioState::ClearBall:
            {
                // 우리 하프면 크게 걷어내기(상대 하프로) = 상대 골 방향 대략킥
                const FVector Goal = GetOppGoalLocation();
                KickTowards(Goal, KickImpulse_Clear);
                State = ELucioState::HoldShape;
                break;
            }

            default:
                State = ELucioState::HoldShape; break;
            }
        }
        else
        {
            // 공이 상대 하프에 있으면 골 앞으로 블로킹 포지션 유지
            const FVector P = ComputeBlockPoint();
            MoveToPoint(P);
            State = ELucioState::HoldShape;
        }
        break;
    }

    // 디버그: 하프라인
    if (bDebug)
    {
        const FVector BallLoc = BallActor->GetActorLocation();
        DrawDebugLine(GetWorld(),
            FVector(-5000.f, 0.f, BallLoc.Z),
            FVector( 5000.f, 0.f, BallLoc.Z),
            FColor::Yellow, false, 0.f, 0, 1.5f);
    }
}


void AAiLucioRole::EnsureTargets()
{
    if (!BallActor)
    {
        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), BallTag, Found);
        if (Found.Num() > 0) BallActor = Cast<ABouncyBall>(Found[0]);
        if (!BallActor)
        {
            for (TActorIterator<ABouncyBall> It(GetWorld()); It; ++It) { BallActor = *It; break; }
        }
    }

    if (!OwnGoalActor) OwnGoalActor = FindActorByTagPreferStaticMesh(OwnGoalTag);
    if (!OppGoalActor) OppGoalActor = FindActorByTagPreferStaticMesh(OppGoalTag);
}

AActor* AAiLucioRole::FindActorByTagPreferStaticMesh(const FName Tag) const
{
    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag, Found);
    if (Found.Num() == 0) return nullptr;

    for (AActor* A : Found)
        if (Cast<AStaticMeshActor>(A)) return A;
    return Found[0];
}


bool AAiLucioRole::IsBallInOwnHalf() const
{
    if (!BallActor) return false;
    const float Y = BallActor->GetActorLocation().Y;
    return bOwnHalfIsNegativeY ? (Y < 0.f) : (Y > 0.f);
}

bool AAiLucioRole::IsBallInOppHalf() const
{
    if (!BallActor) return false;
    const float Y = BallActor->GetActorLocation().Y;
    return bOwnHalfIsNegativeY ? (Y > 0.f) : (Y < 0.f);
}

void AAiLucioRole::MoveToPoint(const FVector& P)
{
    if (!CachedAI) return;
    FAIMoveRequest Req; Req.SetGoalLocation(P); Req.SetAcceptanceRadius(AcceptanceRadius);
    (void)CachedAI->MoveTo(Req);
}

void AAiLucioRole::MoveTowardBall(float /*DeltaSeconds*/)
{
    if (!BallActor) return;
    MoveToPoint(BallActor->GetActorLocation());
}

void AAiLucioRole::StopChasing()
{
    if (CachedAI) CachedAI->StopMovement();
}

void AAiLucioRole::KickTowards(const FVector& Target, float Impulse)
{
    if (!BallActor) return;

    const FVector BallLoc = BallActor->GetActorLocation();
    const FVector Dir = (Target - BallLoc).GetSafeNormal();

    UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(BallActor->GetRootComponent());
    if (Prim && Prim->IsSimulatingPhysics())
    {
        Prim->AddImpulseAtLocation(Dir * Impulse, BallLoc);

        if (bDebug)
        {
            const float len = FMath::Clamp(Impulse * 0.08f, 250.f, 900.f);
            DrawKickDebug(BallLoc, Dir, len, FColor::Green);
        }
    }
}

void AAiLucioRole::DrawKickDebug(const FVector& From, const FVector& Dir, float Len, const FColor& Color) const
{
    DrawDebugDirectionalArrow(GetWorld(), From, From + Dir * Len,
        DebugArrowSize, Color, false, DebugArrowDuration, 0, DebugArrowThickness);
}

// 골대 앞 블로킹 포인트(골대에서 공 쪽으로 BlockDistance 만큼 전진)
FVector AAiLucioRole::ComputeBlockPoint() const
{
    if (!OwnGoalActor || !BallActor) return GetActorLocation();
    const FVector G = OwnGoalActor->GetActorLocation();
    const FVector B = BallActor->GetActorLocation();
    const FVector Dir = (B - G).GetSafeNormal();
    return G + Dir * BlockDistanceFromGoal;
}

// 공의 착지 예상점(있다면) + 약간 앞선 포인트
FVector AAiLucioRole::ComputeInterceptPoint() const
{
    if (const ABouncyBall* BB = Cast<ABouncyBall>(BallActor))
    {
        const FVector Land = BB->GetLandLocation();
        const FVector G = OwnGoalActor ? OwnGoalActor->GetActorLocation() : FVector::ZeroVector;
        const FVector DirFromGoal = (Land - G).GetSafeNormal();
        return Land - DirFromGoal * InterceptLead;
    }
    // 없으면 블록 포인트 사용
    return ComputeBlockPoint();
}

FVector AAiLucioRole::GetOppGoalLocation() const
{
    return OppGoalActor ? OppGoalActor->GetActorLocation()
                        : FVector(0, bOwnHalfIsNegativeY ? 5000.f : -5000.f, 0); // 폴백
}
