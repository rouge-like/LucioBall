#include "CEJ/Ai/AiFindBall.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Animation/AnimInstance.h"

AAiFindBall::AAiFindBall()
{
   PrimaryActorTick.bCanEverTick = true;

    AutoPossessAI    = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AAIController::StaticClass();

    GetMesh()->SetupAttachment(GetCapsuleComponent());
    GetMesh()->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
    GetMesh()->SetRelativeLocation(FVector(0.f, -10.f, 60.f));
    GetMesh()->SetRelativeScale3D(FVector(47.f));

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshRef(
        TEXT("SkeletalMesh'/Game/CEJ/Animations/Skateboarding.Skateboarding'")
    );
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> OverlayMatRef(
        TEXT("Material'/Game/CEJ/Asset/lucio_default_color_tga_Mat.lucio_default_color_tga_Mat'")
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

void AAiFindBall::BeginPlay()
{
    Super::BeginPlay();
    FindTargetByTagOnce(); 
    FindGoalByTagOnce();  
}

void AAiFindBall::FindTargetByTagOnce()
{
    if (TargetActor.IsValid()) return;

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetTag, Found);

    if (Found.Num() > 0)
    {
        TargetActor = Found[0];
        UE_LOG(LogTemp, Log, TEXT("타깃(Ball) 획득: %s (Tag: %s)"),
            *TargetActor->GetName(), *TargetTag.ToString());
    }
    else if (!bLoggedNotFound)
    {
        bLoggedNotFound = true;
        UE_LOG(LogTemp, Warning, TEXT("Tag '%s' 가진 액터(BP_BallTest) 없음"), *TargetTag.ToString());
    }
}

void AAiFindBall::FindGoalByTagOnce()
{
    if (GoalActor.IsValid()) return;

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), GoalTag, Found);

    if (Found.Num() > 0)
    {
        GoalActor = Found[0];
        UE_LOG(LogTemp, Log, TEXT("골대(Goal) 획득: %s (Tag: %s)"),
            *GoalActor->GetName(), *GoalTag.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Tag '%s' 가진 골대 액터 없음"), *GoalTag.ToString());
    }
}

void AAiFindBall::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!TargetActor.IsValid())
    {
        FindTargetByTagOnce();
        if (!TargetActor.IsValid()) return;
    }
    if (!GoalActor.IsValid())
    {
        FindGoalByTagOnce();
        if (!GoalActor.IsValid())
        {
            // 골대가 없으면 볼만 추적(이전 동작 유지)
            if (AAIController* AICon = Cast<AAIController>(GetController()))
            {
                AICon->MoveToActor(TargetActor.Get(), AcceptanceRadius, true, true, true, nullptr, true);
            }
            return;
        }
    }

    MoveLogicTowardGoal(DeltaSeconds);
}

void AAiFindBall::MoveLogicTowardGoal(float /*DeltaSeconds*/)
{
    // 현재 볼/골대 위치
    const FVector BallLoc = TargetActor->GetActorLocation();
    const FVector GoalLoc = GoalActor->GetActorLocation();

    // 볼→골대 방향 (정규화)
    FVector DirToGoal = GoalLoc - BallLoc;
    const float Len = DirToGoal.Size();
    if (Len < KINDA_SMALL_NUMBER) return;
    DirToGoal /= Len;

    // 1) 볼 뒤 어프로치 지점: 볼에서 골대 반대 방향으로 이동
    const FVector ApproachPos = BallLoc - DirToGoal * BehindBallDistance;

    // 2) 킥 지점: 볼을 통과해 골대 방향으로 전진
    const FVector KickPos = BallLoc + DirToGoal * KickThroughDistance;

    // 디버그 표시(볼/어프로치/킥 지점 + 보조선)
    if (bDebugDraw)
    {
        DrawDebugSphere(GetWorld(), BallLoc, 30.f, 16, FColor::Yellow, false, 0.f, 0, 1.5f);
        DrawDebugSphere(GetWorld(), ApproachPos, 20.f, 12, FColor::Cyan,   false, 0.f, 0, 1.5f);
        DrawDebugSphere(GetWorld(), KickPos,     20.f, 12, FColor::Green,  false, 0.f, 0, 1.5f);
        DrawDebugLine(GetWorld(), BallLoc, GoalLoc, FColor::Green, false, 0.f, 0, 1.5f);
        DrawDebugLine(GetWorld(), ApproachPos, BallLoc, FColor::Cyan, false, 0.f, 0, 1.5f);
        DrawDebugLine(GetWorld(), BallLoc, KickPos, FColor::Green, false, 0.f, 0, 1.5f);
    }

    // 방향선/시야 확인: 볼에서 골대 방향으로 트레이스
    {
        FHitResult Hit;
        const bool bBlocked = DoGoalDirectionTrace(BallLoc, DirToGoal, Hit);
        if (bBlocked)
        {
            UE_LOG(LogTemp, Verbose, TEXT("GoalDir blocked by: %s"),
                *GetNameSafe(Hit.GetActor()));
        }
    }

    // 이동 상태 결정
    const FVector MyLoc = GetActorLocation();
    const float DistToApproach = FVector::Dist2D(MyLoc, ApproachPos);

    if (AAIController* AICon = Cast<AAIController>(GetController()))
    {
        if (DistToApproach > ApproachTolerance)
        {
            // 1단계: 볼 뒤로 먼저 돌아가 포지셔닝
            AICon->MoveToLocation(ApproachPos, AcceptanceRadius, true, true, true, true, nullptr, true);
        }
        else
        {
            // 2단계: 볼을 관통해 골대로 전진
            AICon->MoveToLocation(KickPos, AcceptanceRadius, true, true, true, true, nullptr, true);
        }
    }
}

bool AAiFindBall::DoGoalDirectionTrace(const FVector& Start, const FVector& Dir, FHitResult& OutHit)
{
    UWorld* World = GetWorld();
    if (!World) return false;

    const FVector End = Start + Dir * TraceLength;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(GoalDirTrace), /*bTraceComplex*/ true, this);
    Params.AddIgnoredActor(this);
    if (TargetActor.IsValid()) Params.AddIgnoredActor(TargetActor.Get());

    bool bHit = false;

    if (!bUseSphereTrace)
    {
        // 라인 트레이스
        bHit = World->LineTraceSingleByChannel(
            OutHit,
            Start, End,
            ECC_Visibility,
            Params
        );
    }
    else
    {
        // 스피어 트레이스 (더 두껍게 검사)
        bHit = UKismetSystemLibrary::SphereTraceSingle(
            this,
            Start, End,
            TraceRadius,
            UEngineTypes::ConvertToTraceType(ECC_Visibility),
            /*bTraceComplex*/ true,
            { this, TargetActor.Get() }, // 무시 목록
            EDrawDebugTrace::None,
            OutHit,
            /*bIgnoreSelf*/ true
        );
    }

    // 확인용 디버그: 히트면 빨강(임팩트까지), 미히트면 초록(끝까지)
    if (bDebugDraw)
    {
        if (bHit)
        {
            DrawDebugLine(World, Start, OutHit.ImpactPoint, FColor::Red, false, 0.f, 0, 2.f);
            DrawDebugPoint(World, OutHit.ImpactPoint, 16.f, FColor::Red, false, 0.f);
        }
        else
        {
            DrawDebugLine(World, Start, End, FColor::Green, false, 0.f, 0, 2.f);
        }
    }

    return bHit;
}
