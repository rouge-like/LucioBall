// AiCharacter.cpp

#include "CEJ/Ai/AiCharacter.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"

AAiCharacter::AAiCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    AutoPossessAI   = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AAIController::StaticClass();
}

void AAiCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 1) BP_BouncyBall(타깃) 먼저 확보 시도
    TArray<AActor*> FoundBalls;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetTag, FoundBalls);
    if (FoundBalls.Num() > 0)
    {
        TargetActor = FoundBalls[0];
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Tag '%s' 가진 액터(BP_BouncyBall)가 아직 없음"), *TargetTag.ToString());
    }

    // 2) 아주 살짝 지연 후 1회: 벽 감지 → 벽 달리기 → 이후 MoveTo(타깃이 있으면 그쪽으로)
    FTimerHandle Tmp;
    GetWorldTimerManager().SetTimer(
        Tmp, this, &AAiCharacter::PerformBeginWallRunCheck, 0.05f, false
    );
}

void AAiCharacter::PerformBeginWallRunCheck()
{
    // 타깃이 아직 없으면 한번 더 시도
    if (!TargetActor.IsValid())
        TryFindTarget();

    // 벽 감지
    FHitResult Hit;
    if (SphereTraceWall(Hit))
    {
        // 벽 달리기 시도
        if (TryStartWallRun(Hit))
        {
            // 다음 틱에 MoveTo(물리/위치 보정 타이밍 충돌 방지)
            GetWorldTimerManager().SetTimerForNextTick([this]()
            {
                IssueMove();
            });
            UE_LOG(LogTemp, Log, TEXT("Begin: 벽 감지 → 벽달리기 시작 → MoveTo 예약"));
            return;
        }
    }

    // 벽달리기 실패/부적합 → 그냥 MoveTo
    IssueMove();
    UE_LOG(LogTemp, Log, TEXT("Begin: 벽 없음/부적합 → 바로 MoveTo"));
}

bool AAiCharacter::SphereTraceWall(FHitResult& OutHit) const
{
    UCapsuleComponent* Capsule = GetCapsuleComponent();
    const float CapsuleRadius = Capsule ? Capsule->GetScaledCapsuleRadius() : 34.f;

    const FVector Loc   = GetActorLocation();
    const FVector Fwd   = GetActorForwardVector();
    const FVector Right = GetActorRightVector();

    // 시작점을 살짝 앞쪽으로(겹침 시작 방지)
    const float StartOffset = CapsuleRadius + 5.f;
    const float Dist        = FMath::Max(TraceDistance, 300.f);
    const float Radius      = FMath::Max(TraceRadius, 30.f);

    struct FRay { FVector Start; FVector End; };
    TArray<FRay> Rays;

    // 전방 / 좌 / 우 (벽 달리기는 측면에서 잡히는 경우가 많음)
    Rays.Add({ Loc + Fwd    * StartOffset, Loc + Fwd    * (StartOffset + Dist) });
    Rays.Add({ Loc + (-Right)* StartOffset, Loc + (-Right)* (StartOffset + Dist) });
    Rays.Add({ Loc + Right  * StartOffset, Loc + Right  * (StartOffset + Dist) });

    FCollisionQueryParams Params(SCENE_QUERY_STAT(WallRunSweep), false, this);
    FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);

    for (const FRay& R : Rays)
    {
        FHitResult Hit;
        const bool bHit = GetWorld()->SweepSingleByChannel(
            Hit, R.Start, R.End, FQuat::Identity, ECC_Visibility, Sphere, Params);

#if !(UE_BUILD_SHIPPING)
        DrawDebugLine(GetWorld(), R.Start, R.End, bHit ? FColor::Green : FColor::Red, false, 1.f, 0, 1.5f);
        DrawDebugSphere(GetWorld(), bHit ? Hit.ImpactPoint : R.End, Radius, 16, bHit ? FColor::Green : FColor::Red, false, 1.f);
#endif

        if (!bHit) continue;

        // 바닥/천장/완사면 필터
        const float UpDot = FVector::DotProduct(Hit.ImpactNormal, FVector::UpVector);
        if (UpDot > MaxWallNormalUpDot || UpDot < -MinWallNormalUpDot)
            continue;

        // 벽으로 인정(컴포넌트 존재만으로 충분)
        if (Hit.GetComponent())
        {
            OutHit = Hit;
            return true;
        }
    }
    return false;
}

bool AAiCharacter::TryStartWallRun(const FHitResult& Hit)
{
    if (bIsWallRunning) return false;

    // 안전차단(각도)
    const float UpDot = FVector::DotProduct(Hit.ImpactNormal, FVector::UpVector);
    if (UpDot > MaxWallNormalUpDot)  return false; // 바닥 성격
    if (UpDot < -MinWallNormalUpDot) return false; // 천장 성격

    // 벽 방향 계산(왼/오 후보 중 전방에 가까운 쪽)
    const FVector RunDir = ComputeWallRunDir(Hit.ImpactNormal);
    if (RunDir.IsNearlyZero()) return false;

    // Nav 이동 중지 (Pause보다 Stop이 깔끔)
    if (AAIController* AICon = Cast<AAIController>(GetController()))
    {
        AICon->StopMovement();
    }

    // 벽 방향 가속 + 살짝 상승
    const FVector LaunchVel = RunDir * WallRunSpeed + FVector::UpVector * WallRunUpBoost;
    LaunchCharacter(LaunchVel, /*bXYOverride*/true, /*bZOverride*/false);

    bIsWallRunning = true;

    // 일정 시간 후 종료 → MoveTo 재개
    GetWorldTimerManager().SetTimer(
        WallRunTimer, this, &AAiCharacter::EndWallRun, WallRunDuration, false);

    return true;
}

FVector AAiCharacter::ComputeWallRunDir(const FVector& WallNormal) const
{
    // Up × WallNormal = 벽을 따라 이동하는 수평 방향
    const FVector Along1 = FVector::CrossProduct(FVector::UpVector, WallNormal).GetSafeNormal();
    const FVector Along2 = -Along1;

    const FVector Fwd = GetActorForwardVector();
    return (FVector::DotProduct(Fwd, Along1) >= FVector::DotProduct(Fwd, Along2)) ? Along1 : Along2;
}

void AAiCharacter::EndWallRun()
{
    bIsWallRunning = false;
    IssueMove(); // 벽 달리기 종료 후 타깃 추적 재개
}

void AAiCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    TimeSinceCmd += DeltaSeconds;

    // 벽 달리기 중에는 MoveTo 재명령 로직 잠시 중단
    if (bIsWallRunning)
        return;

    // 타깃 없으면 주기적으로 재탐색
    if (!TargetActor.IsValid())
    {
        TimeSinceFindTry += DeltaSeconds;
        if (TimeSinceFindTry >= ReacquireInterval)
        {
            TimeSinceFindTry = 0.f;
            TryFindTarget();
            if (TargetActor.IsValid())
            {
                // 재획득 직후에도 벽 먼저 체크
                FHitResult WallHit;
                if (SphereTraceWall(WallHit) && TryStartWallRun(WallHit))
                    return;

                IssueMove();
            }
        }
        return;
    }

    // 이동 재명령 판단(타깃이 의미있게 이동했거나, 멈춤 상태면 재명령)
    AAIController* AICon = Cast<AAIController>(GetController());
    if (!AICon) return;

    const auto* PFC = AICon->GetPathFollowingComponent();
    const EPathFollowingStatus::Type Status = PFC ? PFC->GetStatus() : EPathFollowingStatus::Idle;

    const FVector CurrentTargetLoc = TargetActor->GetActorLocation();
    const float Moved2D = FVector::Dist2D(CurrentTargetLoc, LastIssuedGoal);

    const bool bNeedReissueByMove   = (Moved2D >= ReissueThreshold);
    const bool bNeedReissueByStatus = (Status == EPathFollowingStatus::Idle || Status == EPathFollowingStatus::Paused);

    if (TimeSinceCmd >= CommandInterval && (bNeedReissueByMove || bNeedReissueByStatus))
    {
        // 재명령 직전에 벽 체크(바로 앞에 벽 있으면 우선 벽 달리기)
        FHitResult WallHit;
        if (SphereTraceWall(WallHit) && TryStartWallRun(WallHit))
            return;

        IssueMove();
    }
}

void AAiCharacter::IssueMove()
{
    if (!TargetActor.IsValid()) return;

    if (AAIController* AICon = Cast<AAIController>(GetController()))
    {
        AICon->MoveToActor(
            TargetActor.Get(),
            AcceptanceRadius,
            /*bStopOnOverlap*/ true,
            /*bUsePathfinding*/ true,
            /*bCanStrafe*/ true,
            nullptr,
            /*bAllowPartialPath*/ true
        );

        LastIssuedGoal = TargetActor->GetActorLocation();
        TimeSinceCmd = 0.f;
    }
}

void AAiCharacter::TryFindTarget()
{
    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetTag, Found);
    if (Found.Num() > 0)
    {
        TargetActor = Found[0];
        UE_LOG(LogTemp, Log, TEXT("목표 재획득: %s"), *TargetActor->GetName());
    }
}
