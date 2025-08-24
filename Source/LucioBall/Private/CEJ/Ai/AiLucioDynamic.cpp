#include "CEJ/Ai/AiLucioDynamic.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/TextRenderComponent.h"
#include "EngineUtils.h"
#include "Navigation/PathFollowingComponent.h"

// BouncyBall 헤더 - 경로에 맞게 수정하세요
#include "OSC/BouncyBall.h"

AAiLucioDynamic::AAiLucioDynamic()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // AI 설정
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AAIController::StaticClass();

    // 캐릭터 이동 설정
    UCharacterMovementComponent* CharMov = GetCharacterMovement();
    if (CharMov)
    {
        CharMov->MaxWalkSpeed = 600.0f;
        CharMov->JumpZVelocity = 800.0f;
        CharMov->GravityScale = 1.0f;
        CharMov->AirControl = 0.6f;
    }

    // 메시 설정
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (MeshComp)
    {
        MeshComp->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
        MeshComp->SetRelativeLocation(FVector(0.f, -10.f, 60.f));
        MeshComp->SetRelativeScale3D(FVector(47.f));

        static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshRef(
            TEXT("SkeletalMesh'/Game/CEJ/Animations/Stay.Stay'")
        );
        
        if (MeshRef.Succeeded())
        {
            MeshComp->SetSkeletalMesh(MeshRef.Object);
        }
    }

    // 역할 텍스트 컴포넌트 생성
    RoleTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("RoleText"));
    if (RoleTextComponent)
    {
        RoleTextComponent->SetupAttachment(RootComponent);
        RoleTextComponent->SetRelativeLocation(FVector(0.0f, 0.0f, TextHeightOffset));
        RoleTextComponent->SetText(FText::FromString(TEXT("DYNAMIC")));
        RoleTextComponent->SetTextRenderColor(FColor::Red);
        RoleTextComponent->SetXScale(3.0f);
        RoleTextComponent->SetYScale(3.0f);
        RoleTextComponent->SetHorizontalAlignment(EHTA_Center);
        RoleTextComponent->SetVerticalAlignment(EVRTA_TextCenter);
        RoleTextComponent->SetWorldSize(50.0f);
        
        // UE5에서 카메라를 항상 바라보도록 설정하는 다른 방법들
        // 옵션 1: 빌보드 효과를 위해 매 프레임 회전 업데이트 (Tick에서 처리)
        // 또는 옵션 2: 컴포넌트 설정으로 처리
        RoleTextComponent->SetGenerateOverlapEvents(false);
    }

    // 이동 애니메이션 로드
    static ConstructorHelpers::FObjectFinder<UAnimSequence> MoveAnimRef(
        TEXT("AnimSequence'/Game/CEJ/Animations/Skateboarding_Anim.Skateboarding_Anim'")
    );
    if (MoveAnimRef.Succeeded())
    {
        MoveAnim = MoveAnimRef.Object;
    }
}

void AAiLucioDynamic::BeginPlay()
{
    Super::BeginPlay();
    
    CachedAI = Cast<AAIController>(GetController());
    EnsureTargets();
    CheckPlayerRole(); // 태그 기반 역할 확인
    UpdateRoleText(); // 역할 텍스트 업데이트
    CurrentState = ELucioDynamicState::Idle;
    
    FString RoleText = bIsAttacker ? TEXT("ATTACKER") : 
                      bIsDefender ? TEXT("DEFENDER") : TEXT("DYNAMIC");
    
    UE_LOG(LogTemp, Log, TEXT("AI Lucio Dynamic Player %d started as %s"), PlayerID, *RoleText);
}

void AAiLucioDynamic::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    EnsureTargets();
    
    if (!BallActor.IsValid()) return;
    
    // 공의 위치에 따라 역할 결정
    DetermineRole();
    
    // 상태 머신 실행
    UpdateStateMachine(DeltaTime);
    
    // 역할 텍스트 업데이트 (고급 이동 상태 반영)
    UpdateRoleText();
    
    // 텍스트가 카메라를 바라보도록 업데이트
    UpdateTextRotation();
    
    // 이동 애니메이션 처리
    HandleMovementAnimation();
    
    // 디버그 정보 표시
    if (bDebug)
    {
        DrawDebugInfo();
    }
}

void AAiLucioDynamic::EnsureTargets()
{
    // BouncyBall 찾기
    if (!BallActor.IsValid())
    {
        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), BallTag, Found);
        if (Found.Num() > 0)
        {
            BallActor = Cast<ABouncyBall>(Found[0]);
        }
        
        // 태그로 못 찾으면 클래스로 찾기
        if (!BallActor.IsValid())
        {
            for (TActorIterator<ABouncyBall> It(GetWorld()); It; ++It)
            {
                BallActor = *It;
                break;
            }
        }
    }
    
    // AI 골대 찾기 (Y < 0, 'SoccerGoal' 태그)
    if (!OwnGoalActor.IsValid())
    {
        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("SoccerGoal"), Found);
        
        // Y < 0 인 골대를 AI의 목표 골대로 설정
        for (AActor* Actor : Found)
        {
            if (Actor->GetActorLocation().Y < 0.0f)
            {
                OwnGoalActor = Actor;
                break;
            }
        }
    }
    
    // 플레이어 골대 찾기 (Y > 0, 수비해야 할 골대)
    if (!OppGoalActor.IsValid())
    {
        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), OppGoalTag, Found);
        
        // Y > 0 인 골대를 수비해야 할 골대로 설정
        for (AActor* Actor : Found)
        {
            if (Actor->GetActorLocation().Y > 0.0f)
            {
                OppGoalActor = Actor;
                break;
            }
        }
        
        // OppGoalTag로 못 찾으면 SoccerGoal에서 Y > 0 찾기
        if (!OppGoalActor.IsValid())
        {
            UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("SoccerGoal"), Found);
            for (AActor* Actor : Found)
            {
                if (Actor->GetActorLocation().Y > 0.0f)
                {
                    OppGoalActor = Actor;
                    break;
                }
            }
        }
    }
}

void AAiLucioDynamic::CheckPlayerRole()
{
    // 태그 기반으로 역할 확인
    bIsAttacker = ActorHasTag(AttackerTag);
    bIsDefender = ActorHasTag(DefenderTag);
    
    // 둘 다 태그가 있는 경우 공격형 우선
    if (bIsAttacker && bIsDefender)
    {
        bIsDefender = false;
        UE_LOG(LogTemp, Warning, TEXT("Player %d has both Attacker and Defender tags. Using Attacker."), PlayerID);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Player %d role determined: Attacker=%s, Defender=%s"), 
           PlayerID, bIsAttacker ? TEXT("true") : TEXT("false"), bIsDefender ? TEXT("true") : TEXT("false"));
}

bool AAiLucioDynamic::DetermineRole()
{
    if (!BallActor.IsValid()) return false;
    
    bool bPreviousAttackMode = bIsInAttackMode;
    
    // 태그 기반 역할 우선
    if (bIsAttacker)
    {
        // 공격형 태그 - 항상 공격 모드
        bIsInAttackMode = true;
    }
    else if (bIsDefender)
    {
        // 수비형 태그 - 항상 수비 모드
        bIsInAttackMode = false;
    }
    else
    {
        // 태그 없음 - 기본 동적 모드: 공의 Y 좌표에 따라 역할 결정
        // Y > 0: 공격 모드 (AI 골대쪽으로 공을 보내야 함, Y < 0)
        // Y < 0: 수비 모드 (플레이어가 Y > 0 골대에 넣지 못하게 막아야 함)
        bIsInAttackMode = GetBallLocation().Y > 0.0f;
    }
    
    // 역할이 바뀌었을 때 상태 초기화
    if (bPreviousAttackMode != bIsInAttackMode)
    {
        CurrentState = ELucioDynamicState::Idle;
        UpdateRoleText(); // 역할 텍스트 업데이트
        
        FString ModeText;
        if (bIsAttacker)
            ModeText = TEXT("FIXED ATTACKER");
        else if (bIsDefender)
            ModeText = TEXT("FIXED DEFENDER");
        else
            ModeText = bIsInAttackMode ? TEXT("DYNAMIC ATTACK") : TEXT("DYNAMIC DEFENSE");
        
        UE_LOG(LogTemp, Log, TEXT("Player %d mode changed to %s"), PlayerID, *ModeText);
        return true;
    }
    
    return false;
}

void AAiLucioDynamic::UpdateStateMachine(float DeltaTime)
{
    if (bIsInAttackMode)
    {
        ExecuteAttackBehavior(DeltaTime);
    }
    else
    {
        ExecuteDefenseBehavior(DeltaTime);
    }
}

void AAiLucioDynamic::ExecuteAttackBehavior(float DeltaTime)
{
    const FVector BallLoc = GetBallLocation();
    const FVector MyLoc = GetActorLocation();
    const float DistToBall = FVector::Dist(MyLoc, BallLoc);
    
    // 공의 착지 예상 위치 계산
    FVector PredictedBallPos = PredictBallLandingPosition();
    float BallLandingTime = CalculateBallLandingTime();
    float TimeToReachBall = CalculateTimeToReachTarget(PredictedBallPos);
    
    switch (CurrentState)
    {
    case ELucioDynamicState::Idle:
        CurrentState = ELucioDynamicState::SeekBall;
        break;
        
    case ELucioDynamicState::SeekBall:
    {
        FVector TargetLocation;
        bool bUseAdvanced = false;
        
        // 공이 공중에 있고 고급 이동이 필요한지 판단
        if (bUseAdvancedMovement && BallLandingTime > 0.1f)
        {
            TargetLocation = PredictedBallPos;
            bUseAdvanced = ShouldUseAdvancedMovement(PredictedBallPos, BallLandingTime);
        }
        else
        {
            TargetLocation = BallLoc; // 일반적인 공 추적
        }
        
        // 이동 실행
        if (bUseAdvanced && bUsingAdvancedMovement)
        {
            ExecuteAdvancedMovement(TargetLocation, DeltaTime);
        }
        else
        {
            MoveToLocation(TargetLocation);
        }
        
        // 공에 도달했는지 확인
        float CheckDistance = bUsingAdvancedMovement ? PossessionRadius * 1.5f : PossessionRadius;
        if (FVector::Dist(MyLoc, TargetLocation) <= CheckDistance)
        {
            CurrentState = ELucioDynamicState::AttackBall;
            bUsingAdvancedMovement = false; // 고급 이동 종료
        }
        
        break;
    }
    
    case ELucioDynamicState::AttackBall:
    {
        // 공 근처에서 AI 골대(Y < 0)로 킥
        if (DistToBall <= AttackDistance)
        {
            FVector AIGoal = GetAIGoalLocation();
            KickBallTowards(AIGoal, AttackKickImpulse);
        }
        
        // 공을 계속 추적
        MoveToLocation(BallLoc);
        
        // 공이 멀어지면 다시 추적 상태로
        if (DistToBall > PossessionRadius * 1.5f)
        {
            CurrentState = ELucioDynamicState::SeekBall;
        }
        break;
    }
    
    default:
        CurrentState = ELucioDynamicState::Idle;
        break;
    }
}

void AAiLucioDynamic::ExecuteDefenseBehavior(float DeltaTime)
{
    const FVector BallLoc = GetBallLocation();
    const FVector MyLoc = GetActorLocation();
    const float DistToBall = FVector::Dist(MyLoc, BallLoc);
    
    switch (CurrentState)
    {
    case ELucioDynamicState::Idle:
        CurrentState = ELucioDynamicState::DefendGoal;
        break;
        
    case ELucioDynamicState::DefendGoal:
    {
        // 플레이어 골대(Y > 0) 수비 포지션으로 이동
        FVector DefensePos = GetDefensePosition();
        MoveToLocation(DefensePos);
        
        // 공이 가까이 오면 클리어 시도
        if (DistToBall <= PossessionRadius)
        {
            CurrentState = ELucioDynamicState::ClearBall;
        }
        break;
    }
    
    case ELucioDynamicState::ClearBall:
    {
        // 공을 AI 골대 쪽으로 강하게 걷어내기
        if (DistToBall <= AttackDistance)
        {
            FVector AIGoal = GetAIGoalLocation();
            KickBallTowards(AIGoal, DefenseKickImpulse);
        }
        
        // 공이 멀어지면 다시 수비 포지션으로
        if (DistToBall > PossessionRadius * 1.5f)
        {
            CurrentState = ELucioDynamicState::DefendGoal;
        }
        break;
    }
    
    default:
        CurrentState = ELucioDynamicState::Idle;
        break;
    }
}

void AAiLucioDynamic::MoveToLocation(const FVector& Location)
{
    if (!CachedAI.IsValid()) return;
    
    FAIMoveRequest MoveReq;
    MoveReq.SetGoalLocation(Location);
    MoveReq.SetAcceptanceRadius(AcceptanceRadius);
    
    FPathFollowingRequestResult Result = CachedAI->MoveTo(MoveReq);
    
    if (bDebug && Result.Code != EPathFollowingRequestResult::RequestSuccessful)
    {
        UE_LOG(LogTemp, Warning, TEXT("MoveTo failed for Player %d, Result: %d"), PlayerID, (int32)Result.Code);
    }
}

bool AAiLucioDynamic::ShouldUseAdvancedMovement(const FVector& TargetLocation, float RequiredTime)
{
    if (!bUseAdvancedMovement) return false;
    
    const FVector MyLoc = GetActorLocation();
    const float DistanceToTarget = FVector::Dist(MyLoc, TargetLocation);
    
    // 일반 속도로 도달 시간 계산
    UCharacterMovementComponent* CharMov = GetCharacterMovement();
    if (!CharMov) return false;
    
    float NormalSpeed = CharMov->MaxWalkSpeed;
    float NormalTime = DistanceToTarget / NormalSpeed;
    
    // 일반 이동으로 늦을 경우 고급 이동 사용
    bool bNeedAdvanced = NormalTime > (RequiredTime - InterceptThreshold);
    
    if (bNeedAdvanced && !bUsingAdvancedMovement)
    {
        bUsingAdvancedMovement = true;
        CurrentAdvancedSpeed = NormalSpeed;
        AdvancedMovementDirection = (TargetLocation - MyLoc).GetSafeNormal();
        
        UE_LOG(LogTemp, Log, TEXT("Player %d activating advanced movement! Distance: %.1f, Required Time: %.2f, Normal Time: %.2f"), 
               PlayerID, DistanceToTarget, RequiredTime, NormalTime);
    }
    
    return bNeedAdvanced;
}

void AAiLucioDynamic::ExecuteAdvancedMovement(const FVector& TargetLocation, float DeltaTime)
{
    if (!bUsingAdvancedMovement) return;
    
    UCharacterMovementComponent* CharMov = GetCharacterMovement();
    if (!CharMov) return;
    
    const FVector MyLoc = GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLoc).GetSafeNormal();
    
    // 방향 업데이트
    AdvancedMovementDirection = FMath::VInterpTo(AdvancedMovementDirection, DirectionToTarget, DeltaTime, 5.0f);
    
    // 가속도 계산 (중력 가속도 + 스킬 가속도)
    float TotalAcceleration = WallRunAcceleration + DiveAcceleration;
    
    // v(t) = v₀ + a*t 공식 적용
    CurrentAdvancedSpeed += TotalAcceleration * DeltaTime;
    CurrentAdvancedSpeed = FMath::Clamp(CurrentAdvancedSpeed, CharMov->MaxWalkSpeed, MaxAdvancedSpeed);
    
    // 이동 속도 적용
    CharMov->MaxWalkSpeed = CurrentAdvancedSpeed;
    
    // 목표 지점으로 이동
    MoveToLocation(TargetLocation);
    
    if (bDebug)
    {
        DrawDebugDirectionalArrow(GetWorld(), MyLoc, MyLoc + AdvancedMovementDirection * 300.0f,
                                  30.0f, FColor::Purple, false, 0.1f, 0, 5.0f);
        
        GEngine->AddOnScreenDebugMessage(
            PlayerID + 200, 0.1f, FColor::Purple,
            FString::Printf(TEXT("Player %d ADVANCED MOVE! Speed: %.0f"), PlayerID, CurrentAdvancedSpeed));
    }
    
    // 목표 근처 도달 시 고급 이동 종료
    if (FVector::Dist(MyLoc, TargetLocation) <= AcceptanceRadius * 2.0f)
    {
        bUsingAdvancedMovement = false;
        CharMov->MaxWalkSpeed = 600.0f; // 원래 속도로 복원
        CurrentAdvancedSpeed = 0.0f;
    }
}

float AAiLucioDynamic::CalculateBallLandingTime()
{
    if (!BallActor.IsValid()) return 0.0f;
    
    FVector BallVelocity = BallActor->GetBouncyBallVelocity();
    FVector BallLocation = GetBallLocation();
    
    // 공이 떨어지고 있는지 확인 (Z 속도가 음수)
    if (BallVelocity.Z >= 0.0f) return 0.0f;
    
    // 간단한 포물선 운동 계산으로 착지 시간 예측
    // h = h₀ + v₀t - 0.5*g*t²
    // 지면(Z=0)에 닿을 때까지의 시간 계산
    float Gravity = GetWorld()->GetGravityZ() * -1.0f; // 중력을 양수로
    float InitialHeight = BallLocation.Z;
    float InitialVelocityZ = BallVelocity.Z * -1.0f; // 음수를 양수로 변환
    
    if (InitialHeight <= 0.0f) return 0.0f;
    
    // 이차 방정식 해법: -0.5*g*t² + v₀*t + h₀ = 0
    float a = -0.5f * Gravity;
    float b = InitialVelocityZ;
    float c = InitialHeight;
    
    float discriminant = b*b - 4*a*c;
    if (discriminant < 0.0f) return 0.0f;
    
    float t1 = (-b + FMath::Sqrt(discriminant)) / (2*a);
    float t2 = (-b - FMath::Sqrt(discriminant)) / (2*a);
    
    // 양수인 시간을 선택
    float landingTime = (t1 > 0.0f) ? t1 : t2;
    return FMath::Max(0.0f, landingTime);
}

float AAiLucioDynamic::CalculateTimeToReachTarget(const FVector& TargetLocation)
{
    const FVector MyLoc = GetActorLocation();
    const float Distance = FVector::Dist(MyLoc, TargetLocation);
    
    UCharacterMovementComponent* CharMov = GetCharacterMovement();
    if (!CharMov) return 0.0f;
    
    float Speed = bUsingAdvancedMovement ? CurrentAdvancedSpeed : CharMov->MaxWalkSpeed;
    return Distance / FMath::Max(Speed, 1.0f);
}

FVector AAiLucioDynamic::PredictBallLandingPosition(float TimeAhead)
{
    if (!BallActor.IsValid()) return GetBallLocation();
    
    // BouncyBall의 GetLandLocation() 함수 사용
    FVector LandLocation = BallActor->GetLandLocation();
    
    // 추가 시간을 고려한 예측이 필요한 경우
    if (TimeAhead > 0.0f)
    {
        FVector BallVelocity = BallActor->GetBouncyBallVelocity();
        FVector HorizontalVelocity = FVector(BallVelocity.X, BallVelocity.Y, 0.0f);
        
        // 수평 방향으로 추가 이동 계산
        LandLocation += HorizontalVelocity * TimeAhead;
    }
    
    return LandLocation;
}

void AAiLucioDynamic::HandleMovementAnimation()
{
    if (!MoveAnim) return;
    
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp) return;
    
    // 이동 속도 체크
    FVector Velocity = GetVelocity();
    bool bIsMoving = Velocity.Size() > 10.0f; // 최소 이동 속도 임계값
    
    if (bIsMoving && !bIsPlayingMoveAnim)
    {
        // 이동 애니메이션 재생
        MeshComp->PlayAnimation(MoveAnim, true); // true = 루프
        bIsPlayingMoveAnim = true;
    }
    else if (!bIsMoving && bIsPlayingMoveAnim)
    {
        // 정지 시 애니메이션 중지
        MeshComp->Stop();
        bIsPlayingMoveAnim = false;
    }
}

void AAiLucioDynamic::KickBallTowards(const FVector& Target, float Impulse)
{
    if (!BallActor.IsValid()) return;
    
    const FVector BallLoc = GetBallLocation();
    const FVector MyLoc = GetActorLocation();
    const float DistToBall = FVector::Dist(MyLoc, BallLoc);
    
    // 공과 충분히 가까울 때만 킥
    if (DistToBall > AttackDistance) return;
    
    UPrimitiveComponent* BallPrim = Cast<UPrimitiveComponent>(BallActor->GetRootComponent());
    if (!BallPrim || !BallPrim->IsSimulatingPhysics()) return;
    
    FVector KickDir = (Target - BallLoc).GetSafeNormal();
    KickDir.Z = 0.0f; // 수평으로만 킥
    
    BallPrim->AddImpulseAtLocation(KickDir * Impulse, BallLoc);
    
    if (bDebug)
    {
        DrawDebugDirectionalArrow(GetWorld(), BallLoc, BallLoc + KickDir * 500.0f,
                                  50.0f, FColor::Red, false, 0.5f, 0, 3.0f);
        
        GEngine->AddOnScreenDebugMessage(
            PlayerID + 100, 0.5f, FColor::Yellow,
            FString::Printf(TEXT("Player %d KICK! Mode: %s, Target: %s, Impulse: %.0f"),
                          PlayerID, bIsInAttackMode ? TEXT("ATTACK") : TEXT("DEFENSE"),
                          bIsInAttackMode ? TEXT("AI Goal(Y<0)") : TEXT("AI Goal(Y<0)"), Impulse));
    }
}

FVector AAiLucioDynamic::GetBallLocation() const
{
    return BallActor.IsValid() ? BallActor->GetActorLocation() : FVector::ZeroVector;
}

FVector AAiLucioDynamic::GetBallLandLocation() const
{
    if (BallActor.IsValid())
    {
        return BallActor->GetLandLocation();
    }
    return GetBallLocation();
}

// AI가 골을 넣어야 하는 골대 (Y < 0, 'SoccerGoal' 태그)
FVector AAiLucioDynamic::GetAIGoalLocation() const
{
    return OwnGoalActor.IsValid() ? OwnGoalActor->GetActorLocation() : FVector(0, -3000, 0);
}

// 플레이어가 골을 넣으려는 골대 (Y > 0, AI가 수비해야 함)
FVector AAiLucioDynamic::GetPlayerGoalLocation() const
{
    return OppGoalActor.IsValid() ? OppGoalActor->GetActorLocation() : FVector(0, 3000, 0);
}

// 이전 함수들과의 호환성 유지
FVector AAiLucioDynamic::GetOwnGoalLocation() const
{
    return GetAIGoalLocation();
}

FVector AAiLucioDynamic::GetOppGoalLocation() const
{
    return GetPlayerGoalLocation();
}

FVector AAiLucioDynamic::GetDefensePosition() const
{
    FVector PlayerGoal = GetPlayerGoalLocation(); // 수비해야 할 골대 (Y > 0)
    FVector BallLoc = GetBallLandLocation(); // 착지 예상 지점 사용
    
    FVector DirFromGoal = (BallLoc - PlayerGoal).GetSafeNormal();
    return PlayerGoal + DirFromGoal * DefenseDistance;
}

FVector AAiLucioDynamic::GetAttackPosition() const
{
    FVector BallLoc = GetBallLocation();
    FVector AIGoal = GetAIGoalLocation(); // AI가 넣어야 할 골대 (Y < 0)
    
    // 공과 AI 골 사이에 위치
    FVector DirToBall = (BallLoc - AIGoal).GetSafeNormal();
    return BallLoc - DirToBall * AttackDistance;
}

bool AAiLucioDynamic::IsBallNearby(float Distance) const
{
    if (!BallActor.IsValid()) return false;
    
    if (Distance <= 0.0f) Distance = PossessionRadius;
    
    const float DistToBall = FVector::Dist(GetActorLocation(), GetBallLocation());
    return DistToBall <= Distance;
}

bool AAiLucioDynamic::IsBallInNegativeY() const
{
    if (!BallActor.IsValid()) return false;
    
    return GetBallLocation().Y < 0.0f;
}

void AAiLucioDynamic::UpdateRoleText()
{
    if (!RoleTextComponent || !bShowRoleText) return;
    
    FString RoleString;
    FColor TextColor;
    
    if (bIsAttacker)
    {
        if (bUsingAdvancedMovement)
        {
            RoleString = TEXT("공격(가속)");
            TextColor = FColor::Purple;
        }
        else
        {
            RoleString = bIsInAttackMode ? TEXT("공격") : TEXT("공격(대기)");
            TextColor = FColor::Red;
        }
    }
    else if (bIsDefender)
    {
        if (bUsingAdvancedMovement)
        {
            RoleString = TEXT("수비(가속)");
            TextColor = FColor::Purple;
        }
        else
        {
            RoleString = bIsInAttackMode ? TEXT("수비(진출)") : TEXT("수비");
            TextColor = FColor::Blue;
        }
    }
    else
    {
        if (bUsingAdvancedMovement)
        {
            RoleString = TEXT("가속모드");
            TextColor = FColor::Purple;
        }
        else
        {
            RoleString = bIsInAttackMode ? TEXT("공격모드") : TEXT("수비모드");
            TextColor = bIsInAttackMode ? FColor::Orange : FColor::Cyan;
        }
    }
    
    RoleTextComponent->SetText(FText::FromString(RoleString));
    RoleTextComponent->SetTextRenderColor(TextColor);
    RoleTextComponent->SetRelativeLocation(FVector(0.0f, 0.0f, TextHeightOffset));
}

void AAiLucioDynamic::UpdateTextRotation()
{
    if (!RoleTextComponent) return;
    
    // 플레이어 카메라 찾기
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;
    
    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;
    
    // 카메라 위치 가져오기 (플레이어 위치 사용)
    FVector CameraLocation = PlayerPawn->GetActorLocation();
    FVector TextLocation = RoleTextComponent->GetComponentLocation();
    
    // 카메라를 바라보는 방향 계산
    FVector LookDirection = (CameraLocation - TextLocation).GetSafeNormal();
    
    // 텍스트가 카메라를 바라보도록 회전 설정
    FRotator LookRotation = FRotationMatrix::MakeFromX(LookDirection).Rotator();
    
    // Y축 회전을 180도 추가하여 텍스트가 올바른 방향으로 보이게 함
    LookRotation.Yaw += 180.0f;
    
    RoleTextComponent->SetWorldRotation(LookRotation);
}

void AAiLucioDynamic::DrawDebugInfo() const
{
    const FVector MyLoc = GetActorLocation();
    const FVector BallLoc = GetBallLocation();
    
    // 중앙선 표시
    DrawDebugLine(GetWorld(),
                  FVector(-5000.0f, 0.0f, MyLoc.Z),
                  FVector(5000.0f, 0.0f, MyLoc.Z),
                  FColor::White, false, 0.0f, 0, 2.0f);
    
    // AI 상태 표시
    FColor StateColor = bUsingAdvancedMovement ? FColor::Purple : 
                       (bIsInAttackMode ? FColor::Red : FColor::Blue);
    FString StateText = bUsingAdvancedMovement ? TEXT("ADVANCED") :
                       (bIsInAttackMode ? TEXT("ATTACK") : TEXT("DEFENSE"));
    
    DrawDebugSphere(GetWorld(), MyLoc + FVector(0, 0, 200), 50.0f, 12, StateColor, false, 0.0f, 0, 2.0f);
    
    // 예상 착지 지점 표시
    if (BallActor.IsValid())
    {
        FVector PredictedLanding = const_cast<AAiLucioDynamic*>(this)->PredictBallLandingPosition();
        DrawDebugSphere(GetWorld(), PredictedLanding, 100.0f, 12, FColor::Yellow, false, 0.0f, 0, 2.0f);
        
        // 착지 지점까지의 연결선
        if (bIsInAttackMode)
        {
            DrawDebugLine(GetWorld(), MyLoc, PredictedLanding, FColor::Green, false, 0.0f, 0, 3.0f);
        }
        
        // 공의 궤적 표시
        DrawDebugLine(GetWorld(), BallLoc, PredictedLanding, FColor::Yellow, false, 0.0f, 0, 1.0f);
    }
    
    // 골대 위치 표시
    FVector AIGoal = GetAIGoalLocation();
    FVector PlayerGoal = GetPlayerGoalLocation();
    
    DrawDebugSphere(GetWorld(), AIGoal + FVector(0, 0, 100), 100.0f, 12, FColor::Green, false, 0.0f, 0, 3.0f);
    DrawDebugSphere(GetWorld(), PlayerGoal + FVector(0, 0, 100), 100.0f, 12, FColor::Orange, false, 0.0f, 0, 3.0f);
    
    // 화면에 상태 정보 표시
    if (GEngine)
    {
        FString ModeText;
        if (bIsAttacker)
            ModeText = FString::Printf(TEXT("ATTACKER (%s)"), *StateText);
        else if (bIsDefender)
            ModeText = FString::Printf(TEXT("DEFENDER (%s)"), *StateText);
        else
            ModeText = FString::Printf(TEXT("DYNAMIC (%s)"), *StateText);
        
        // 추가 정보 표시
        float BallLandingTime = const_cast<AAiLucioDynamic*>(this)->CalculateBallLandingTime();
        float TimeToReach = const_cast<AAiLucioDynamic*>(this)->CalculateTimeToReachTarget(
            const_cast<AAiLucioDynamic*>(this)->PredictBallLandingPosition());
        
        FString DebugText = FString::Printf(
            TEXT("Player %d: %s | State: %d | Ball Land T: %.1fs | Reach T: %.1fs | Speed: %.0f"),
            PlayerID, *ModeText, (int32)CurrentState, BallLandingTime, TimeToReach, 
            bUsingAdvancedMovement ? CurrentAdvancedSpeed : 600.0f);
            
        GEngine->AddOnScreenDebugMessage(
            PlayerID, 0.0f, StateColor, DebugText);
    }
}