// AAiFSM.cpp
#include "CEJ/Ai/AiFSM.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Navigation/PathFollowingComponent.h"

#include "OSC/BouncyBall.h"

AAiFSM::AAiFSM()
{
    PrimaryActorTick.bCanEverTick = true;

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

    bBallCrossedHalfLine = false;
    HalfLineCheckRadius = 100.f;
}

void AAiFSM::BeginPlay()
{
    Super::BeginPlay();

    CachedAI = Cast<AAIController>(GetController());

    // 시작 시 1회 타깃 보장
    EnsureTargets();

    // 시작값은 0 기준으로
    LastBallY = 0.f;

    State = ESimpleAIState::TrackBall;
}

void AAiFSM::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // 계속 추적: 매 틱마다 타깃 보장
    EnsureTargets();
    if (!BallActor) return;

    const bool bBallInOurHalf = IsBallInOurHalf();

    // 공이 우리 하프에 없으면 Idle + 이동중지
    if (!bBallInOurHalf)
    {
        State = ESimpleAIState::Idle;
        StopChasing();
        if (bDebug) DrawDebugInfo();
        return;
    }

    switch (State)
    {
    case ESimpleAIState::Idle:
        {
            State = ESimpleAIState::TrackBall;
            break;
        }

    case ESimpleAIState::TrackBall:
        {
            MoveTowardBall(DeltaSeconds);

            const float Dist = FVector::Dist(GetActorLocation(), BallActor->GetActorLocation());
            if (Dist <= KickDistance * 1.2f)
            {
                State = ESimpleAIState::KickToGoal;
            }
            break;
        }

    case ESimpleAIState::KickToGoal:
        {
            // 킥 도중 공이 하프 밖으로 나가면 중지
            if (!IsBallInOurHalf())
            {
                State = ESimpleAIState::Idle;
                StopChasing();
                break;
            }

            TryKickBall();                 // 임펄스 적용
            State = ESimpleAIState::TrackBall; // 다시 추적 루프
            break;
        }

    default:
        break;
    }

    if (bDebug) DrawDebugInfo();
}

void AAiFSM::EnsureTargets()
{
    // 공: 없으면 한 번만 찾아 캐시 (공은 보통 1개라 매틱 탐색까지는 비권장)
    if (!BallActor)
    {
        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), BallTag, Found);
        if (Found.Num() > 0)
        {
            BallActor = Cast<ABouncyBall>(Found[0]);
        }
        else
        {
            for (TActorIterator<ABouncyBall> It(GetWorld()); It; ++It)
            {
                BallActor = *It;
                break;
            }
        }

        if (!BallActor)
        {
            UE_LOG(LogTemp, Warning, TEXT("BouncyBall 못찾음"));
        }
    }

    // 골대: 태그 "SoccerGoal" 로 **항상** 재확보 (StaticMeshActor 우선)
    {
        AActor* NewGoal = nullptr;

        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), GoalTag, Found);
        if (Found.Num() > 0)
        {
            // StaticMeshActor 우선 선택
            for (AActor* A : Found)
            {
                if (AStaticMeshActor* S = Cast<AStaticMeshActor>(A))
                {
                    NewGoal = S;
                    break;
                }
            }
            if (!NewGoal) NewGoal = Found[0];
        }

        GoalActor = NewGoal;

        if (!GoalActor)
        {
            UE_LOG(LogTemp, Warning, TEXT("GoalActor(태그=%s) 못찾음"), *GoalTag.ToString());
        }
    }
}

void AAiFSM::UpdateHalfLineCrossing(float CurrentBallY)
{
    // 조건: 시작은 0이었고, Y가 음수로 넘어간 최초 순간 플래그 ON
    if (!bBallCrossedHalfLine)
    {
        if (LastBallY >= 0.f && CurrentBallY < 0.f)
        {
            bBallCrossedHalfLine = true;
            UE_LOG(LogTemp, Log, TEXT("Ball crossed half-line: Y %f -> %f"), LastBallY, CurrentBallY);
        }
    }
}

void AAiFSM::MoveTowardBall(float /*DeltaSeconds*/)
{
    if (!CachedAI || !BallActor) return;

    const FVector Target = BallActor->GetActorLocation();

    // MoveTo: AcceptanceRadius 안이면 완료
    FAIMoveRequest Req;
    Req.SetGoalLocation(Target);
    Req.SetAcceptanceRadius(AcceptanceRadius);
    CachedAI->MoveTo(Req);
}

void AAiFSM::TryKickBall()
{
    if (!BallActor || !GoalActor) return;
    if (!IsBallInOurHalf()) return; // 우리 하프 아닐 땐 킥 금지

    const float Dist = FVector::Dist(GetActorLocation(), BallActor->GetActorLocation());
    if (Dist > KickDistance)
    {
        State = ESimpleAIState::TrackBall;
        return;
    }

    KickBallTowardsGoal();
}

void AAiFSM::KickBallTowardsGoal()
{
    // 공의 속도/착지 예상점도 가져와서 방향 보정 가능
    const FVector BallLoc   = BallActor->GetActorLocation();
    const FVector GoalLoc   = GoalActor ? GoalActor->GetActorLocation() : FVector::ZeroVector;

    // 골대가 넓다면 골대 중앙에서 살짝 “안쪽” 으로 오프셋을 주는 것도 좋음
    const FVector Dir = (GoalLoc - BallLoc).GetSafeNormal();

    // 물리 컴포넌트 찾아 임펄스 적용
    /*UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(BallActor->GetRootComponent());
    if (Prim && Prim->IsSimulatingPhysics())
    {
        const FVector Impulse = Dir * KickImpulse;
        Prim->AddImpulseAtLocation(Impulse, BallLoc);

        if (bDebug)
        {
            DrawDebugDirectionalArrow(GetWorld(), BallLoc, BallLoc + Dir * 300.f, 40.f,
                                      FColor::Green, false, 1.5f, 0, 3.f);
        }

        UE_LOG(LogTemp, Log, TEXT("Kick! impulse=%s"), *Impulse.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BallActor가 물리 시뮬레이션 안 함(또는 Primitive 아님). Kick 실패"));
    }*/

    if (!BallActor || !GoalActor) return;

    // 1) 에임 방향(공 → 골) : 노란색
    const FVector AimDir = (GoalLoc - BallLoc).GetSafeNormal();

    // 2) 실제 적용할 임펄스 방향(기본은 에임과 동일, 필요시 보정 가능)
    const FVector KickDir = AimDir;

    // 시각화 길이(가독성용): 킥 임펄스 크기에 따라 가변, 적당히 클램프
    const float AimVizLen  = 300.f;
    const float ImpVizLen  = FMath::Clamp(KickImpulse * 0.08f, 300.f, 900.f);

    // 3) 공의 현재 속도(있으면): 청록색
    FVector CurVel = FVector::ZeroVector;
    if (ABouncyBall* BB = Cast<ABouncyBall>(BallActor))
    {
        CurVel = BB->GetBouncyBallVelocity();
    }
    const float VelVizLen = FMath::Clamp(CurVel.Size() * 0.15f, 200.f, 800.f);
    const FVector VelEnd  = BallLoc + (CurVel.IsNearlyZero() ? FVector::ZeroVector : CurVel.GetSafeNormal() * VelVizLen);

    // ── 디버그 화살표들 ──────────────────────────────────────────────
    if (bDebug)
    {
        // [의도] 골 방향 (노란색)
        DrawDebugDirectionalArrow(
            GetWorld(),
            BallLoc,
            BallLoc + AimDir * AimVizLen,
            DebugArrowSize,
            FColor::Yellow,
            false,
            DebugArrowDuration,
            0,
            DebugArrowThickness
        );

        // [현재 속도] 공 벡터 (청록색)
        if (!CurVel.IsNearlyZero())
        {
            DrawDebugDirectionalArrow(
                GetWorld(),
                BallLoc,
                VelEnd,
                DebugArrowSize,
                FColor::Cyan,
                false,
                DebugArrowDuration,
                0,
                DebugArrowThickness
            );
        }
    }

    // ── 실제 임펄스 적용 ────────────────────────────────────────────
    UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(BallActor->GetRootComponent());
    if (Prim && Prim->IsSimulatingPhysics())
    {
        const FVector Impulse = KickDir * KickImpulse;
        Prim->AddImpulseAtLocation(Impulse, BallLoc);

        if (bDebug)
        {
            // [실제 킥] 적용 임펄스 (초록색, 가장 굵게)
            DrawDebugDirectionalArrow(
                GetWorld(),
                BallLoc,
                BallLoc + KickDir * ImpVizLen,
                DebugArrowSize * 1.2f,
                FColor::Green,
                false,
                DebugArrowDuration,
                0,
                DebugArrowThickness * 1.5f
            );
        }

        UE_LOG(LogTemp, Log, TEXT("Kick! impulse=%s"), *Impulse.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BallActor가 물리 시뮬레이션 안 함(또는 Primitive 아님). Kick 실패"));
    }
}

void AAiFSM::DrawDebugInfo()
{
#if WITH_EDITOR
    if (!BallActor) return;

    const FVector BallLoc = BallActor->GetActorLocation();

    // 하프라인( Y=0 ) 시각화
    DrawDebugLine(GetWorld(),
        FVector(-5000.f, 0.f, BallLoc.Z),
        FVector( 5000.f, 0.f, BallLoc.Z),
        FColor::Yellow, false, 0.f, 0, 1.5f);

    // 현재 공 위치
    DrawDebugSphere(GetWorld(), BallLoc, 20.f, 12, FColor::Cyan, false, 0.f, 0, 1.5f);

    // 목표 골
    if (GoalActor)
    {
        DrawDebugSphere(GetWorld(), GoalActor->GetActorLocation(), 30.f, 12, FColor::Red, false, 0.f, 0, 2.f);
        DrawDebugDirectionalArrow(GetWorld(), BallLoc, GoalActor->GetActorLocation(), 60.f, FColor::Red, false, 0.f, 0, 2.5f);
    }
#endif
}

bool AAiFSM::IsBallInOurHalf() const
{
    return BallActor && (BallActor->GetActorLocation().Y < 0.f);
}

void AAiFSM::StopChasing()
{
    if (CachedAI)
    {
        CachedAI->StopMovement();
    }
}
