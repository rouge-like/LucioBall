#include "CEJ/Ai/AiLucioDynamic.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/TextRenderComponent.h"
#include "EngineUtils.h"
#include "Navigation/PathFollowingComponent.h"

#include "OSC/BouncyBall.h"

AAiLucioDynamic::AAiLucioDynamic()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // AI 설정
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AAIController::StaticClass();

    // 기본 스탯 초기화 (게임 데이터 기반)
    BaseMovementSpeed = 800.0f;
    BaseJumpPower = 700.0f;
    WallRunSpeedMultiplier = 1.3f;
    ESkillSpeedMultiplier = 1.6f;
    UltimateSpeedMultiplier = 2.0f;
    UltimateJumpMultiplier = 1.5f;
    
    // 공격 관련 스탯
    BasicAttackForce = 1000.0f;
    BasicAttackRange = 80.0f;
    BasicAttackCooldown = 0.5f;
    SkillAttackForce = 2000.0f;
    SkillAttackRange = 150.0f;
    SkillAttackCooldown = 1.0f;
    
    // 스킬 쿨타임 및 지속시간
    ESkillCooldown = 6.0f;
    ESkillDuration = 3.0f;
    UltimateCooldown = 30.0f;
    UltimateDuration = 8.0f;
    
    // 스킬 상태 초기화
    bESkillActive = false;
    bUltimateActive = false;
    ESkillEndTime = 0.0f;
    UltimateEndTime = 0.0f;
    LastESkillUse = 0.0f;
    LastUltimateUse = 0.0f;

    // 캐릭터 이동 설정
    UCharacterMovementComponent* CharMov = GetCharacterMovement();
    if (CharMov)
    {
        CharMov->MaxWalkSpeed = BaseMovementSpeed;
        CharMov->JumpZVelocity = BaseJumpPower;
        CharMov->MaxAcceleration = 4000.0f; // 높은 가속도로 설정
        CharMov->BrakingDecelerationWalking = 2000.0f;
        CharMov->GroundFriction = 8.0f;
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
        RoleTextComponent->SetWorldSize(30.0f);
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
    
    // 점프 애니메이션 로드
    static ConstructorHelpers::FObjectFinder<UAnimSequence> JumpAnimRef(
        TEXT("AnimSequence'/Game/CEJ/Animations/Jumping_Anim.Jumping_Anim'")
    );
    if (JumpAnimRef.Succeeded())
    {
        JumpAnim = JumpAnimRef.Object;
    }
}

void AAiLucioDynamic::BeginPlay()
{
    Super::BeginPlay();
    
    CachedAI = Cast<AAIController>(GetController());
    EnsureTargets();
    CheckPlayerRole();
    UpdateRoleText();
    CurrentState = ELucioDynamicState::Idle;
    
    // 물리 계산 초기화
    RunStartTime = 0.0f;
    JumpStartTime = 0.0f;
    InitialRunSpeed = 0.0f;
    InitialJumpVelocity = 0.0f;
    bIsRunning = false;
    bIsJumping = false;
    
    // GetLandLocation 추적 초기화
    PreviousLandLocation = FVector::ZeroVector;
    bIsTrackingLandLocation = false;
    
    FString RoleText = bIsAttacker ? TEXT("ATTACKER") : 
                      bIsDefender ? TEXT("DEFENDER") : TEXT("DYNAMIC");
    
    UE_LOG(LogTemp, Log, TEXT("AI Lucio Dynamic Player %d started as %s"), PlayerID, *RoleText);
}

void AAiLucioDynamic::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    EnsureTargets();
    
    if (!BallActor.IsValid()) return;
    
    // 스킬 상태 업데이트
    UpdateSkillStates();
    
    // GetLandLocation 변화 감지 및 선제적 이동
    CheckAndRespondToLandLocationChange();
    
    // 최적 속도로 GetLandLocation 추적
    OptimizeSpeedForBallChase();
    
    // 공의 위치에 따라 역할 결정
    DetermineRole();
    
    // 상태 머신 실행
    UpdateStateMachine(DeltaTime);
    
    // 역할 텍스트 업데이트
    UpdateRoleText();
    
    // 텍스트가 카메라를 바라보도록 업데이트
    UpdateTextRotation();
    
    // 이동 애니메이션 처리
    HandleMovementAnimation();
    
    // 점프 처리 (공이 높이 있을 때)
    HandleJumpBehavior();
    
    // 물리 계산 및 속도 출력
    CalculateAndDisplayPhysics(DeltaTime);
    
    // 디버그 정보 표시
    if (bDebug)
    {
        DrawDebugInfo();
    }
}

void AAiLucioDynamic::CheckAndRespondToLandLocationChange()
{
    if (!BallActor.IsValid()) return;
    
    FVector CurrentLandLocation = BallActor->GetLandLocation();
    float DistanceThreshold = 50.0f; // 50 유닛 이상 변화시 감지
    
    // 첫 번째 실행이거나 착지 위치가 크게 변경된 경우
    bool bLandLocationChanged = false;
    
    if (!bIsTrackingLandLocation)
    {
        // 첫 번째 추적 시작
        bIsTrackingLandLocation = true;
        bLandLocationChanged = true;
        UE_LOG(LogTemp, Log, TEXT("AI %d started tracking LandLocation: %s"), 
               PlayerID, *CurrentLandLocation.ToString());
    }
    else
    {
        // 기존 위치와 비교
        float LocationDifference = FVector::Dist(PreviousLandLocation, CurrentLandLocation);
        if (LocationDifference > DistanceThreshold)
        {
            bLandLocationChanged = true;
            UE_LOG(LogTemp, Log, TEXT("AI %d detected LandLocation change! Diff: %.1f | Old: %s | New: %s"), 
                   PlayerID, LocationDifference, 
                   *PreviousLandLocation.ToString(), *CurrentLandLocation.ToString());
        }
    }
    
    if (bLandLocationChanged)
    {
        PreviousLandLocation = CurrentLandLocation;
        
        // 역할에 따라 선제적 대응
        RespondToLandLocationChange(CurrentLandLocation);
    }
}

void AAiLucioDynamic::RespondToLandLocationChange(const FVector& NewLandLocation)
{
     if (!BallActor.IsValid()) return;
    
    float NewLandY = NewLandLocation.Y;
    FVector MyLocation = GetActorLocation();
    float DistanceToNewLand = FVector::Dist(MyLocation, NewLandLocation);
    
    // 역할 결정 (수정된 로직)
    bool bShouldRespondAsAttacker = false;
    bool bShouldRespondAsDefender = false;
    
    if (bIsAttacker)
    {
        // 공격수는 Y >= 0일 때만 활성화
        bShouldRespondAsAttacker = (NewLandY >= 0.0f);
    }
    else if (bIsDefender)
    {
        // 수비수는 Y < 0일 때만 활성화
        bShouldRespondAsDefender = (NewLandY < 0.0f);
    }
    else
    {
        // 동적 역할: 착지 위치의 Y 좌표로 판단
        if (NewLandY > 0.0f)
        {
            bShouldRespondAsAttacker = true;
        }
        else if (NewLandY < 0.0f)
        {
            bShouldRespondAsDefender = true;
        }
    }
    
    // 선제적 대응 실행
    if (bShouldRespondAsAttacker || bShouldRespondAsDefender)
    {
        // 긴급 상황 판단
        bool bIsEmergency = (DistanceToNewLand > 300.0f) || 
                           (bShouldRespondAsDefender && NewLandY < -100.0f);
        
        if (bIsEmergency)
        {
            // 스킬 사용으로 최대 속도 확보
            if (CanUseUltimate())
            {
                UseUltimate();
                UE_LOG(LogTemp, Warning, TEXT("AI %d EMERGENCY! Used Ultimate for LandLocation: %s"), 
                       PlayerID, *NewLandLocation.ToString());
            }
            else if (CanUseESkill())
            {
                UseESkill();
                UE_LOG(LogTemp, Warning, TEXT("AI %d EMERGENCY! Used E-Skill for LandLocation: %s"), 
                       PlayerID, *NewLandLocation.ToString());
            }
        }
        
        // 즉시 새로운 착지 위치로 이동 시작
        MoveToLocationWithMaxSpeed(NewLandLocation);
        
        // 상태를 적절히 변경
        if (bShouldRespondAsAttacker)
        {
            CurrentState = ELucioDynamicState::SeekBall;
        }
        else if (bShouldRespondAsDefender)
        {
            CurrentState = ELucioDynamicState::ClearBall;
        }
        
        FString RoleStr = bShouldRespondAsAttacker ? TEXT("ATTACK") : TEXT("DEFEND");
        UE_LOG(LogTemp, Log, TEXT("AI %d responding as %s to LandLocation: %s (Dist: %.1f)"), 
               PlayerID, *RoleStr, *NewLandLocation.ToString(), DistanceToNewLand);
    }
    else
    {
        // 해당 역할의 활성화 조건에 맞지 않으면 대기
        FString WaitReason;
        if (bIsAttacker && NewLandY < 0.0f)
        {
            WaitReason = FString::Printf(TEXT("ATTACKER waiting (Y=%.1f<0)"), NewLandY);
        }
        else if (bIsDefender && NewLandY >= 0.0f)
        {
            WaitReason = FString::Printf(TEXT("DEFENDER waiting (Y=%.1f>=0)"), NewLandY);
        }
        
        if (!WaitReason.IsEmpty())
        {
            UE_LOG(LogTemp, Log, TEXT("AI %d: %s"), PlayerID, *WaitReason);
        }
    }
}

void AAiLucioDynamic::UpdateSkillStates()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // E 스킬 상태 업데이트
    if (bESkillActive && CurrentTime > ESkillEndTime)
    {
        bESkillActive = false;
        UE_LOG(LogTemp, Log, TEXT("AI %d E skill ended"), PlayerID);
    }
    
    // 궁극기 상태 업데이트
    if (bUltimateActive && CurrentTime > UltimateEndTime)
    {
        bUltimateActive = false;
        UE_LOG(LogTemp, Log, TEXT("AI %d Ultimate ended"), PlayerID);
    }
}

void AAiLucioDynamic::OptimizeSpeedForBallChase()
{
    if (!BallActor.IsValid()) return;
    
    FVector BallLandLocation = BallActor->GetLandLocation();
    FVector MyLocation = GetActorLocation();
    float DistanceToLandLocation = FVector::Dist(MyLocation, BallLandLocation);
    float BallLandY = BallLandLocation.Y;
    
    // 최적 속도 계산
    float OptimalSpeed = BaseMovementSpeed;
    bool bShouldUseSkill = false;
    
    UCharacterMovementComponent* CharMov = GetCharacterMovement();
    if (!CharMov) return;
    
    // 상황별 최적 속도 결정
    if (bUltimateActive)
    {
        // 궁극기 활성화 시 최대 속도
        OptimalSpeed = BaseMovementSpeed * UltimateSpeedMultiplier;
        CharMov->JumpZVelocity = BaseJumpPower * UltimateJumpMultiplier;
    }
    else if (bESkillActive)
    {
        // E 스킬 활성화 시
        OptimalSpeed = BaseMovementSpeed * ESkillSpeedMultiplier;
    }
    else
    {
        // 긴급 상황 판단
        bool bEmergencySituation = false;
        
        // 수비 상황 (공이 우리 진영으로 오는 경우)
        if ((bIsDefender || (!bIsAttacker && BallLandY < 0.0f)) && DistanceToLandLocation > 300.0f)
        {
            bEmergencySituation = true;
        }
        // 공격 상황 (공을 놓칠 위험이 있는 경우)
        else if ((bIsAttacker || (!bIsDefender && BallLandY > 0.0f)) && DistanceToLandLocation > 400.0f)
        {
            bEmergencySituation = true;
        }
        
        if (bEmergencySituation)
        {
            // 스킬 사용 가능한지 확인
            float CurrentTime = GetWorld()->GetTimeSeconds();
            
            // 궁극기 우선 사용 (가장 강력)
            if (CanUseUltimate())
            {
                UseUltimate();
                OptimalSpeed = BaseMovementSpeed * UltimateSpeedMultiplier;
                CharMov->JumpZVelocity = BaseJumpPower * UltimateJumpMultiplier;
            }
            // E 스킬 사용
            else if (CanUseESkill())
            {
                UseESkill();
                OptimalSpeed = BaseMovementSpeed * ESkillSpeedMultiplier;
            }
            // 스킬 사용 불가 시에도 기본 부스트
            else
            {
                OptimalSpeed = BaseMovementSpeed * 1.3f; // 기본 부스트
            }
        }
    }
    
    // 속도 적용
    CharMov->MaxWalkSpeed = OptimalSpeed;
    CharMov->MaxAcceleration = 5000.0f; // 매우 높은 가속도
    
    // 디버그 정보
    if (bDebug)
    {
        FString SpeedInfo = FString::Printf(TEXT("Speed: %.0f (%.1fx) | Dist: %.0f | Skills: E=%s U=%s"), 
                                          OptimalSpeed, OptimalSpeed / BaseMovementSpeed, 
                                          DistanceToLandLocation,
                                          bESkillActive ? TEXT("ON") : TEXT("OFF"),
                                          bUltimateActive ? TEXT("ON") : TEXT("OFF"));
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(PlayerID + 400, 0.0f, FColor::Orange, SpeedInfo);
        }
    }
}

bool AAiLucioDynamic::CanUseESkill() const
{
    if (bESkillActive) return false;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastESkillUse) >= ESkillCooldown;
}

bool AAiLucioDynamic::CanUseUltimate() const
{
    if (bUltimateActive) return false;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastUltimateUse) >= UltimateCooldown;
}

void AAiLucioDynamic::UseESkill()
{
    if (!CanUseESkill()) return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    bESkillActive = true;
    ESkillEndTime = CurrentTime + ESkillDuration;
    LastESkillUse = CurrentTime;
    
    UE_LOG(LogTemp, Log, TEXT("AI %d used E Skill! Speed boost to %.0f for %.1fs"), 
           PlayerID, BaseMovementSpeed * ESkillSpeedMultiplier, ESkillDuration);
}

void AAiLucioDynamic::UseUltimate()
{
    if (!CanUseUltimate()) return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    bUltimateActive = true;
    UltimateEndTime = CurrentTime + UltimateDuration;
    LastUltimateUse = CurrentTime;
    
    UE_LOG(LogTemp, Log, TEXT("AI %d used ULTIMATE! Speed: %.0f, Jump: %.0f for %.1fs"), 
           PlayerID, BaseMovementSpeed * UltimateSpeedMultiplier, 
           BaseJumpPower * UltimateJumpMultiplier, UltimateDuration);
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
        
        // 태그로 못 찾으면 클래스로
        if (!BallActor.IsValid())
        {
            for (TActorIterator<ABouncyBall> It(GetWorld()); It; ++It)
            {
                BallActor = *It;
                break;
            }
        }
    }
    
    // SoccerGoal 태그를 가진 골대들 찾기
    if (!OppGoalActor.IsValid())
    {
        TArray<AActor*> Goals;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("SoccerGoal"), Goals);
        
        if (Goals.Num() > 0)
        {
            // 첫 번째 골대를 상대 골대로 설정 (필요시 Y 좌표로 구분 가능)
            OppGoalActor = Goals[0];
            UE_LOG(LogTemp, Log, TEXT("Found Soccer Goal at location: %s"), 
                   *OppGoalActor->GetActorLocation().ToString());
        }
    }
}

void AAiLucioDynamic::CheckPlayerRole()
{
    // 태그 기반으로 역할 확인
    bIsAttacker = ActorHasTag(AttackerTag);
    bIsDefender = ActorHasTag(DefenderTag);
    
    UE_LOG(LogTemp, Log, TEXT("Player %d role determined: Attacker=%s, Defender=%s"), 
           PlayerID, bIsAttacker ? TEXT("true") : TEXT("false"), bIsDefender ? TEXT("true") : TEXT("false"));
}

bool AAiLucioDynamic::DetermineRole()
{
   
    if (!BallActor.IsValid()) return false;
    
    bool bPreviousAttackMode = bIsInAttackMode;
    
    // BouncyBall의 GetLandLocation() 사용
    FVector BallLandLocation = BallActor->GetLandLocation();
    float BallLandY = BallLandLocation.Y;
    
    // 태그 기반 역할 구분 - 조건부 활성화
    if (bIsAttacker)
    {
        // 공격수는 Y >= 0일 때만 활성화
        bIsInAttackMode = (BallLandY >= 0.0f);
    }
    else if (bIsDefender)
    {
        // 수비수는 Y < 0일 때만 활성화
        bIsInAttackMode = (BallLandY < 0.0f);
    }
    else
    {
        // 동적 모드: 공의 착지 예상 Y 좌표에 따라 역할 결정
        bIsInAttackMode = (BallLandY > 0.0f);
    }
    
    // 역할이 바뀌었을 때 상태 초기화
    if (bPreviousAttackMode != bIsInAttackMode)
    {
        CurrentState = ELucioDynamicState::Idle;
        UpdateRoleText();
        
        FString ModeText;
        if (bIsAttacker)
            ModeText = bIsInAttackMode ? 
                       FString::Printf(TEXT("ATTACKER ACTIVE (Y=%.1f>=0)"), BallLandY) : 
                       FString::Printf(TEXT("ATTACKER WAITING (Y=%.1f<0)"), BallLandY);
        else if (bIsDefender)
            ModeText = bIsInAttackMode ? 
                       FString::Printf(TEXT("DEFENDER ACTIVE (Y=%.1f<0)"), BallLandY) : 
                       FString::Printf(TEXT("DEFENDER WAITING (Y=%.1f>=0)"), BallLandY);
        else
            ModeText = bIsInAttackMode ? 
                       FString::Printf(TEXT("DYNAMIC ATTACK (LandY=%.1f)"), BallLandY) : 
                       FString::Printf(TEXT("DYNAMIC DEFENSE (LandY=%.1f)"), BallLandY);
        
        UE_LOG(LogTemp, Log, TEXT("Player %d role changed: %s"), PlayerID, *ModeText);
        return true;
    }
    
    return false;
}

void AAiLucioDynamic::UpdateStateMachine(float DeltaTime)
{
    // 공의 착지 예상 Y 위치에 따라 행동 결정
    FVector BallLandLocation = GetBallLandLocation();
    float BallLandY = BallLandLocation.Y;
    
    // 역할별 행동 결정
    bool bShouldAct = false;
    
    if (bIsAttacker)
    {
        // 공격수: Y >= 0일 때만 활성화
        if (BallLandY >= 0.0f)
        {
            bShouldAct = true;
            ExecuteAttackBehavior(DeltaTime);
        }
        else
        {
            // Y < 0일 때는 대기
            CurrentState = ELucioDynamicState::Idle;
        }
    }
    else if (bIsDefender)
    {
        // 수비수: Y < 0일 때만 활성화
        if (BallLandY < 0.0f)
        {
            bShouldAct = true;
            ExecuteDefenseBehavior(DeltaTime);
        }
        else
        {
            // Y >= 0일 때는 대기
            CurrentState = ELucioDynamicState::Idle;
        }
    }
    else
    {
        // 동적 캐릭터: 기존 로직 유지
        if (BallLandY > 0.0f)
        {
            bShouldAct = true;
            ExecuteAttackBehavior(DeltaTime);
        }
        else if (BallLandY < 0.0f)
        {
            bShouldAct = true;
            ExecuteDefenseBehavior(DeltaTime);
        }
        else
        {
            CurrentState = ELucioDynamicState::Idle;
        }
    }
    
    if (!bShouldAct)
    {
        // 대기 상태
        CurrentState = ELucioDynamicState::Idle;
    }
}

void AAiLucioDynamic::ExecuteAttackBehavior(float DeltaTime)
{
    const FVector BallLoc = GetBallLocation();
    const FVector MyLoc = GetActorLocation();
    const float DistToBall = FVector::Dist(MyLoc, BallLoc);
    
    // 공의 착지 예상 위치 계산
    FVector PredictedBallPos = BallActor->GetLandLocation();
    
    switch (CurrentState)
    {
        case ELucioDynamicState::Idle:
            CurrentState = ELucioDynamicState::SeekBall;
            UE_LOG(LogTemp, Log, TEXT("Attack AI %d: Idle -> SeekBall"), PlayerID);
            break;
            
        case ELucioDynamicState::SeekBall:
        {
            // 공의 착지 예상 위치로 이동
            MoveToLocation(PredictedBallPos);

            // 디버그: 클리어 의도 방향 미리보기 (SoccerGoal 방향 + Z+300)
            if (bDebug)
            {
                FVector IntendedClearDirection;
                
                if (OppGoalActor.IsValid())
                {
                    // SoccerGoal 방향의 X,Y + Z 300 의도 표시
                    FVector GoalLocation = OppGoalActor->GetActorLocation();
                    FVector GoalDirection = (GoalLocation - BallLoc).GetSafeNormal();
                    IntendedClearDirection = FVector(GoalDirection.X, GoalDirection.Y, 1.0f).GetSafeNormal();
                }
                else
                {
                    // 순수 Z 방향
                    IntendedClearDirection = FVector(0.0f, 0.0f, 1.0f);
                }
                
                DrawDebugDirectionalArrow(GetWorld(), 
                                        BallLoc, 
                                        BallLoc + (IntendedClearDirection * 250.0f), 
                                        25.0f, 
                                        FColor::Purple, 
                                        false, 
                                        0.0f, 
                                        0, 
                                        5.0f);
            }
            
            // 공에 충분히 가까워졌는지 확인
            if (DistToBall <= PossessionRadius)
            {
                CurrentState = ELucioDynamicState::AttackBall;
                UE_LOG(LogTemp, Log, TEXT("Attack AI %d: SeekBall -> AttackBall"), PlayerID);
            }
            break;
        }
        
        case ELucioDynamicState::AttackBall:
        {
            // 공 근처에서 SoccerGoal 태그를 가진 골대 방향으로 킥
            if (DistToBall <= AttackDistance && OppGoalActor.IsValid())
            {
                // 골대 방향 계산
                FVector GoalLocation = OppGoalActor->GetActorLocation();
                FVector KickDirection = (GoalLocation - BallLoc).GetSafeNormal();
                
                // Z 값을 조정하여 공이 골대로 향하도록 함
                KickDirection.Z = 0.3f; // 약간의 상향 각도로 증가
                KickDirection.Normalize();
                
                // 공에 임펄스 적용
                FVector Impulse = KickDirection * AttackKickImpulse;
                BallActor->BouncyBallAddImpulse(Impulse, this);
                
                // 디버그: 공격 방향 화살표 표시
                DrawDebugDirectionalArrow(GetWorld(), 
                                        BallLoc, 
                                        BallLoc + (KickDirection * 500.0f), 
                                        50.0f, 
                                        FColor::Red, 
                                        false, 
                                        2.0f, 
                                        0, 
                                        10.0f);
                
                UE_LOG(LogTemp, Log, TEXT("Attack AI %d kicked ball towards SoccerGoal! Impulse: %s"), 
                       PlayerID, *Impulse.ToString());
                
                // 짧은 딜레이 후 다시 추적 상태로
                CurrentState = ELucioDynamicState::SeekBall;
            }
            else
            {
                // 공을 계속 추적
                MoveToLocation(BallLoc);
                
                // 디버그: 공격 의도 방향 미리보기 (골대가 있을 때)
                if (OppGoalActor.IsValid() && bDebug)
                {
                    FVector GoalLocation = OppGoalActor->GetActorLocation();
                    FVector IntendedDirection = (GoalLocation - BallLoc).GetSafeNormal();
                    IntendedDirection.Z = 0.3f;
                    IntendedDirection.Normalize();
                    
                    DrawDebugDirectionalArrow(GetWorld(), 
                                            BallLoc, 
                                            BallLoc + (IntendedDirection * 300.0f), 
                                            30.0f, 
                                            FColor::Orange, 
                                            false, 
                                            0.0f, 
                                            0, 
                                            5.0f);
                }
            }
            
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
    
    // 공의 착지 예상 위치
    FVector PredictedBallPos = BallActor->GetLandLocation();
    const bool bBallOnDefenseSide = (PredictedBallPos.Y < 0.0f);
    
    // 수비 상황에서 이동속도 증가 (Y < 0일 때)
    UCharacterMovementComponent* CharMov = GetCharacterMovement();
    if (CharMov && bBallOnDefenseSide)
    {
        // 기본 속도 800 * 1.6 = 1280으로 증가
        float DefenseSpeedMultiplier = 1.6f;
        float BaseSpeed = 800.0f;
        CharMov->MaxWalkSpeed = BaseSpeed * DefenseSpeedMultiplier;
        
        UE_LOG(LogTemp, Log, TEXT("Defense AI %d speed boosted to %.1f (Y<0 emergency)"), 
               PlayerID, CharMov->MaxWalkSpeed);
    }
    else if (CharMov)
    {
        // 수비 상황이 아니면 기본 속도로 복원
        CharMov->MaxWalkSpeed = 800.0f;
    }

    switch (CurrentState)
    {
        case ELucioDynamicState::Idle:
            // 수비 모드 시작
            if (bBallOnDefenseSide)
            {
                CurrentState = ELucioDynamicState::ClearBall;
                UE_LOG(LogTemp, Log, TEXT("Defense AI %d: Idle -> ClearBall"), PlayerID);
            }
            else
            {
                CurrentState = ELucioDynamicState::DefendGoal;
                UE_LOG(LogTemp, Log, TEXT("Defense AI %d: Idle -> DefendGoal"), PlayerID);
            }
            break;

        case ELucioDynamicState::ClearBall:
        {
            // Y가 음수가 아니면 클리어 필요 없음
            if (!bBallOnDefenseSide)
            {
                UE_LOG(LogTemp, Log, TEXT("Defense AI %d: Ball not on defense side (Y=%.1f)"), 
                       PlayerID, PredictedBallPos.Y);
                CurrentState = ELucioDynamicState::DefendGoal;
                break;
            }

            // 공의 착지 예상 위치로 이동
            MoveToLocation(PredictedBallPos);

            // 공이 가까이 오면 SoccerGoal 방향 + Z+300으로 클리어
            if (DistToBall <= PossessionRadius)
            {
                FVector ClearImpulse;
                
                if (OppGoalActor.IsValid())
                {
                    // SoccerGoal 방향의 X,Y + Z 300 조합
                    FVector GoalLocation = OppGoalActor->GetActorLocation();
                    FVector GoalDirection = (GoalLocation - BallLoc).GetSafeNormal();
                    
                    // X,Y는 골대 방향, Z는 +300
                    ClearImpulse = FVector(GoalDirection.X, GoalDirection.Y, 1.0f).GetSafeNormal() * AttackKickImpulse * 0.9f;
                    ClearImpulse.Z = 300.0f; // Z 값을 명시적으로 300으로 설정
                }
                else
                {
                    // 골대를 찾지 못한 경우 기본 Z+300 클리어
                    ClearImpulse = FVector(0.0f, 0.0f, 300.0f);
                }
                
                BallActor->BouncyBallAddImpulse(ClearImpulse, this);

                // 디버그: 클리어 방향 화살표 표시 (SoccerGoal 방향 + Z+300)
                DrawDebugDirectionalArrow(GetWorld(), 
                                        BallLoc, 
                                        BallLoc + ClearImpulse.GetSafeNormal() * 500.0f, 
                                        50.0f, 
                                        FColor::Blue, 
                                        false, 
                                        2.0f, 
                                        0, 
                                        10.0f);

                UE_LOG(LogTemp, Log, TEXT("Defense AI %d cleared ball towards SoccerGoal + Z300! Impulse: %s, Speed was %.1f"), 
                       PlayerID, *ClearImpulse.ToString(), CharMov ? CharMov->MaxWalkSpeed : 0.0f);

                CurrentState = ELucioDynamicState::DefendGoal;
            }
            break;
        }

        case ELucioDynamicState::DefendGoal:
        {
            // 우리 진영(Y<0)에 공이 다시 오면 클리어 준비
            if (bBallOnDefenseSide && DistToBall <= PossessionRadius * 2.0f)
            {
                CurrentState = ELucioDynamicState::ClearBall;
            }
            else if (bBallOnDefenseSide && DistToBall <= PossessionRadius)
            {
                // 긴급 SoccerGoal 방향 + Z+300 클리어
                FVector ClearImpulse;
                
                if (OppGoalActor.IsValid())
                {
                    // SoccerGoal 방향의 X,Y + Z 300 조합
                    FVector GoalLocation = OppGoalActor->GetActorLocation();
                    FVector GoalDirection = (GoalLocation - BallLoc).GetSafeNormal();
                    
                    // X,Y는 골대 방향, Z는 +300
                    ClearImpulse = FVector(GoalDirection.X, GoalDirection.Y, 1.0f).GetSafeNormal() * AttackKickImpulse * 0.9f;
                    ClearImpulse.Z = 300.0f; // Z 값을 명시적으로 300으로 설정
                }
                else
                {
                    // 골대를 찾지 못한 경우 기본 Z+300 클리어
                    ClearImpulse = FVector(0.0f, 0.0f, 300.0f);
                }
                
                BallActor->BouncyBallAddImpulse(ClearImpulse, this);
                
                // 디버그: 긴급 클리어 방향 화살표 표시 (SoccerGoal 방향 + Z+300)
                DrawDebugDirectionalArrow(GetWorld(), 
                                        BallLoc, 
                                        BallLoc + ClearImpulse.GetSafeNormal() * 400.0f, 
                                        40.0f, 
                                        FColor::Cyan, 
                                        false, 
                                        1.5f, 
                                        0, 
                                        8.0f);
                
                UE_LOG(LogTemp, Log, TEXT("Defense AI %d emergency SoccerGoal + Z300 clear! Impulse: %s, Speed: %.1f"), 
                       PlayerID, *ClearImpulse.ToString(), CharMov ? CharMov->MaxWalkSpeed : 0.0f);
            }
            else
            {
                // 수비 포지션 유지 (필요시 특정 위치로 이동)
                // 예: 골대 앞이나 중앙선 근처로 이동
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
    MoveReq.SetUsePathfinding(true);
    MoveReq.SetAllowPartialPath(true);
    
    // 긴급 상황에서는 더 작은 수용 반경 사용
    FVector MyLoc = GetActorLocation();
    float DistToTarget = FVector::Dist(MyLoc, Location);
    
    // 공의 착지 위치로 이동할 때는 더 정확하게
    if (BallActor.IsValid())
    {
        FVector BallLandLoc = BallActor->GetLandLocation();
        if (FVector::Dist(Location, BallLandLoc) < 50.0f) // 착지 위치와 비슷한 목표
        {
            MoveReq.SetAcceptanceRadius(30.0f); // 더 가까이 접근
            
            // AI 이동 요청
            EPathFollowingRequestResult::Type MoveResult = CachedAI->MoveTo(MoveReq);
            
            // 추가적으로 캐릭터에게 직접 이동 명령
            if (UCharacterMovementComponent* CharMov = GetCharacterMovement())
            {
                FVector MoveDirection = (Location - MyLoc).GetSafeNormal();
                AddMovementInput(MoveDirection, 1.0f);
                
                // 디버그 로그
                UE_LOG(LogTemp, VeryVerbose, TEXT("AI %d rushing to ball land location! Distance: %.1f, Speed: %.1f, Result: %d"), 
                       PlayerID, DistToTarget, CharMov->MaxWalkSpeed, (int32)MoveResult);
            }
            return;
        }
    }
    
    // 일반 이동
    CachedAI->MoveTo(MoveReq);
}

void AAiLucioDynamic::HandleMovementAnimation()
{
    if (!MoveAnim) return;
    
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp) return;
    
    FVector Velocity = GetVelocity();
    bool bIsMoving = Velocity.Size() > 10.0f;
    bool bIsInAir = !GetCharacterMovement()->IsMovingOnGround();
    
    // 달리기 상태 추적
    if (bIsMoving && !bIsInAir && !bIsRunning)
    {
        // 달리기 시작
        bIsRunning = true;
        RunStartTime = GetWorld()->GetTimeSeconds();
        InitialRunSpeed = Velocity.Size2D(); // 2D 속도로 초기값 설정
        
        UE_LOG(LogTemp, Log, TEXT("AI %d started running - Initial Speed: %.1f"), PlayerID, InitialRunSpeed);
    }
    else if ((!bIsMoving || bIsInAir) && bIsRunning)
    {
        // 달리기 종료
        bIsRunning = false;
        UE_LOG(LogTemp, Log, TEXT("AI %d stopped running"), PlayerID);
    }
    
    // 공중에 있거나 점프 애니메이션이 재생 중이면 이동 애니메이션 중단
    if (bIsInAir || bIsPlayingJumpAnim)
    {
        if (bIsPlayingMoveAnim)
        {
            MeshComp->Stop();
            bIsPlayingMoveAnim = false;
        }
        return;
    }
    
    if (bIsMoving && !bIsPlayingMoveAnim)
    {
        MeshComp->PlayAnimation(MoveAnim, true);
        bIsPlayingMoveAnim = true;
    }
    else if (!bIsMoving && bIsPlayingMoveAnim)
    {
        MeshComp->Stop();
        bIsPlayingMoveAnim = false;
    }
}

void AAiLucioDynamic::HandleJumpBehavior()
{
    if (!BallActor.IsValid()) return;
    
    const FVector BallLoc = GetBallLocation();
    const FVector MyLoc = GetActorLocation();
    const float DistToBall = FVector::Dist2D(MyLoc, BallLoc); // 2D 거리로 계산
    const float BallHeight = BallLoc.Z;
    const float MyHeight = MyLoc.Z;
    const float HeightDifference = BallHeight - MyHeight;
    
    // 점프 상태 추적 (로컬 변수명을 다르게 사용)
    bool bPreviousJumpingState = bIsJumping;
    bIsJumping = !GetCharacterMovement()->IsMovingOnGround();
    
    // 점프 조건 확인
    bool bShouldJump = false;
    
    // 1. 공이 충분히 높이 있고 (100 유닛 이상)
    // 2. 가로 거리가 가깝고 (200 유닛 이내)
    // 3. 땅에 있고, 점프 중이 아닐 때
    if (HeightDifference > 100.0f && 
        DistToBall <= 200.0f && 
        GetCharacterMovement()->IsMovingOnGround() && 
        !bIsPlayingJumpAnim)
    {
        bShouldJump = true;
        
        // 점프 시작 시간과 초기 속도 기록
        JumpStartTime = GetWorld()->GetTimeSeconds();
        InitialJumpVelocity = 700.0f; // JumpZVelocity 값
        
        // 점프력 700 적용
        Jump();
        
        // 점프 애니메이션 재생
        if (JumpAnim)
        {
            USkeletalMeshComponent* MeshComp = GetMesh();
            if (MeshComp)
            {
                MeshComp->PlayAnimation(JumpAnim, false); // 한 번만 재생
                bIsPlayingJumpAnim = true;
                
                // 일정 시간 후 점프 애니메이션 플래그 리셋 (점프 애니메이션 길이에 맞춰 조정)
                GetWorldTimerManager().SetTimer(JumpAnimResetTimer, this, &AAiLucioDynamic::ResetJumpAnimFlag, 1.5f, false);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("AI %d jumped for ball! Ball height: %.1f, Distance: %.1f, Initial Jump Velocity: %.1f"), 
               PlayerID, BallHeight, DistToBall, InitialJumpVelocity);
               
       
    }
}

void AAiLucioDynamic::CalculateAndDisplayPhysics(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    FVector CurrentVelocity = GetVelocity();
    
    // 달리기 물리 계산: v(t) = v₀ + at
    if (bIsRunning)
    {
        float RunTime = CurrentTime - RunStartTime;
        float RunAcceleration = 200.0f; // 가속도 값 (단위: UU/s²)
        
        // 이론적 속도 계산
        float TheoreticalRunSpeed = InitialRunSpeed + (RunAcceleration * RunTime);
        float ActualRunSpeed = CurrentVelocity.Size2D();
        
        // 최대 속도 제한 적용
        UCharacterMovementComponent* CharMov = GetCharacterMovement();
        float MaxSpeed = CharMov ? CharMov->MaxWalkSpeed : 800.0f;
        TheoreticalRunSpeed = FMath::Min(TheoreticalRunSpeed, MaxSpeed);
        
        if (GEngine && bDebug)
        {
            FString RunPhysicsText = FString::Printf(
                TEXT("RUN PHYSICS | Time: %.1fs | v₀: %.1f | a: %.1f | v(t): %.1f | Actual: %.1f"),
                RunTime, InitialRunSpeed, RunAcceleration, TheoreticalRunSpeed, ActualRunSpeed);
            
            GEngine->AddOnScreenDebugMessage(
                PlayerID + 100, 0.0f, FColor::Green, RunPhysicsText);
        }
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("AI %d Run Physics - Time: %.1fs, Theoretical: %.1f, Actual: %.1f"), 
               PlayerID, RunTime, TheoreticalRunSpeed, ActualRunSpeed);
    }
    
    // 점프 물리 계산: v(t) = v₀ + (-g)t
    if (bIsJumping && JumpStartTime > 0.0f)
    {
        float JumpTime = CurrentTime - JumpStartTime;
        float Gravity = 980.0f; // 중력 가속도 (단위: UU/s²)
        
        // 이론적 수직 속도 계산 (아래 방향이 음수)
        float TheoreticalJumpVelocity = InitialJumpVelocity - (Gravity * JumpTime);
        float ActualVerticalVelocity = CurrentVelocity.Z;
        
        if (GEngine && bDebug)
        {
            FString JumpPhysicsText = FString::Printf(
                TEXT("JUMP PHYSICS | Time: %.1fs | v₀: %.1f | g: %.1f | v(t): %.1f | Actual: %.1f"),
                JumpTime, InitialJumpVelocity, Gravity, TheoreticalJumpVelocity, ActualVerticalVelocity);
            
            GEngine->AddOnScreenDebugMessage(
                PlayerID + 200, 0.0f, FColor::Magenta, JumpPhysicsText);
        }
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("AI %d Jump Physics - Time: %.1fs, Theoretical: %.1f, Actual: %.1f"), 
               PlayerID, JumpTime, TheoreticalJumpVelocity, ActualVerticalVelocity);
        
        // 착지 감지
        if (GetCharacterMovement()->IsMovingOnGround() && JumpTime > 0.1f) // 0.1초 후부터 착지 감지
        {
            bIsJumping = false;
            float TotalJumpTime = JumpTime;
            
            UE_LOG(LogTemp, Log, TEXT("AI %d landed! Total jump time: %.2fs"), PlayerID, TotalJumpTime);
            
            if (GEngine && bDebug)
            {
                FString LandingText = FString::Printf(TEXT("LANDED! Total jump time: %.2fs"), TotalJumpTime);
                GEngine->AddOnScreenDebugMessage(PlayerID + 300, 2.0f, FColor::Yellow, LandingText);
            }
        }
    }
}

void AAiLucioDynamic::MoveToLocationWithMaxSpeed(const FVector& Location)
{
    // 일반 AI 이동
    MoveToLocation(Location);
    
    // 추가적인 직접 이동 입력으로 최대 속력 보장
    FVector MyLoc = GetActorLocation();
    FVector MoveDirection = (Location - MyLoc).GetSafeNormal();
    
    // 강력한 이동 입력 적용
    AddMovementInput(MoveDirection, 1.0f);
    
    // 물리적으로도 캐릭터를 목표 방향으로 밀어줌
    if (UCharacterMovementComponent* CharMov = GetCharacterMovement())
    {
        FVector CurrentVelocity = GetVelocity();
        FVector DesiredVelocity = MoveDirection * CharMov->MaxWalkSpeed;
        
        // 속도가 부족하면 추가 가속
        if (CurrentVelocity.Size2D() < CharMov->MaxWalkSpeed * 0.8f)
        {
            FVector AccelerationForce = (DesiredVelocity - CurrentVelocity) * 0.1f;
            AccelerationForce.Z = 0.0f; // Z축 가속은 제외
            LaunchCharacter(AccelerationForce, false, false);
        }
        
        // 디버그 로그
        UE_LOG(LogTemp, VeryVerbose, TEXT("AI %d max speed move! Current: %.1f, Target: %.1f, Distance: %.1f"), 
               PlayerID, CurrentVelocity.Size2D(), CharMov->MaxWalkSpeed, FVector::Dist(MyLoc, Location));
    }
}

void AAiLucioDynamic::ResetJumpAnimFlag()
{
    bIsPlayingJumpAnim = false;
    UE_LOG(LogTemp, Log, TEXT("AI %d jump animation completed"), PlayerID);
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

void AAiLucioDynamic::UpdateRoleText()
{
    if (!RoleTextComponent || !bShowRoleText) return;
    
    FString RoleString;
    FColor TextColor;
    
    // GetLandLocation() 사용
    FVector BallLandLocation = GetBallLandLocation();
    float BallLandY = BallLandLocation.Y;
    
    if (bIsAttacker)
    {
        RoleString = TEXT("ATTACKER");
        TextColor = FColor::Red;
    }
    else if (bIsDefender)
    {
        RoleString = TEXT("DEFENDER");
        TextColor = FColor::Blue;
    }
    else
    {
        // 동적 모드 - 착지 예상 Y 위치 표시
        if (BallLandY > 0.0f)
        {
            RoleString = FString::Printf(TEXT("ATTACK (LY:%.0f)"), BallLandY);
            TextColor = FColor::Orange;
        }
        else if (BallLandY < 0.0f)
        {
            RoleString = FString::Printf(TEXT("DEFEND (LY:%.0f)"), BallLandY);
            TextColor = FColor::Cyan;
        }
        else
        {
            RoleString = FString::Printf(TEXT("WAIT (LY:%.0f)"), BallLandY);
            TextColor = FColor::Yellow;
        }
    }
    
    RoleTextComponent->SetText(FText::FromString(RoleString));
    RoleTextComponent->SetTextRenderColor(TextColor);
    RoleTextComponent->SetRelativeLocation(FVector(0.0f, 0.0f, TextHeightOffset));
}

void AAiLucioDynamic::UpdateTextRotation()
{
    if (!RoleTextComponent) return;
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;
    
    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;
    
    FVector CameraLocation = PlayerPawn->GetActorLocation();
    FVector TextLocation = RoleTextComponent->GetComponentLocation();
    
    FVector LookDirection = (CameraLocation - TextLocation).GetSafeNormal();
    FRotator LookRotation = FRotationMatrix::MakeFromX(LookDirection).Rotator();
    LookRotation.Yaw += 180.0f;
    
    RoleTextComponent->SetWorldRotation(LookRotation);
}

bool AAiLucioDynamic::IsBallNearby(float Threshold) const
{
    if (!BallActor.IsValid()) return false;

    const FVector MyPos   = GetActorLocation();
    const FVector BallPos = BallActor->GetActorLocation();
    const float   Dist    = FVector::Dist(MyPos, BallPos);
    return Dist <= Threshold;
}

bool AAiLucioDynamic::IsBallClose() const
{
    return IsBallNearby(PossessionRadius);
}
void AAiLucioDynamic::DrawDebugInfo() const
{
    const FVector MyLoc = GetActorLocation();
    const FVector BallLoc = GetBallLocation();
    const FVector BallLandLoc = GetBallLandLocation();
    float BallLandY = BallLandLoc.Y;
    
    // Y=0 중앙선 표시
    DrawDebugLine(GetWorld(),
                  FVector(-5000.0f, 0.0f, 100.0f),
                  FVector(5000.0f, 0.0f, 100.0f),
                  FColor::White, false, 0.0f, 0, 5.0f);
    
    // AI 상태 표시
    FColor StateColor = (BallLandY > 0.0f) ? FColor::Red : 
                       (BallLandY < 0.0f) ? FColor::Blue : FColor::Yellow;
    FString StateText = (BallLandY > 0.0f) ? TEXT("ATTACK MODE") : 
                       (BallLandY < 0.0f) ? TEXT("DEFENSE MODE") : TEXT("WAIT MODE");
    
    
    // 공의 착지 예상 지점 표시 (GetLandLocation 사용)
    if (BallActor.IsValid())
    {
        DrawDebugSphere(GetWorld(), BallLandLoc, 100.0f, 12, FColor::Yellow, false, 0.0f, 0, 2.0f);
        
        // 착지 지점까지의 연결선
        DrawDebugLine(GetWorld(), MyLoc, BallLandLoc, FColor::Green, false, 0.0f, 0, 3.0f);
        
        // 공의 궤적 표시
        DrawDebugLine(GetWorld(), BallLoc, BallLandLoc, FColor::Yellow, false, 0.0f, 0, 1.0f);
    }
    
    // 화면에 상태 정보 표시
    if (GEngine)
    {
        // 현재 이동속도 정보 추가
        UCharacterMovementComponent* CharMov = GetCharacterMovement();
        float CurrentSpeed = CharMov ? CharMov->MaxWalkSpeed : 0.0f;
        bool bIsInAir = CharMov ? !CharMov->IsMovingOnGround() : false;
        
        FString JumpStatus = bIsPlayingJumpAnim ? TEXT("JUMP") : (bIsInAir ? TEXT("AIR") : TEXT("GROUND"));
        
        FString DebugText = FString::Printf(
            TEXT("Player %d: %s | LandY: %.1f | State: %d | Dist: %.1f | Speed: %.0f | %s"),
            PlayerID, *StateText, BallLandY, (int32)CurrentState, 
            FVector::Dist(MyLoc, BallLoc), CurrentSpeed, *JumpStatus);
            
        GEngine->AddOnScreenDebugMessage(
            PlayerID, 0.0f, StateColor, DebugText);
    }
}