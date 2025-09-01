// Fill out your copyright notice in the Description page of Project Settings.

#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "AIController.h"
#include "OSC/BouncyBall.h"
#include  "CEJ/Ai/AiLucioDynamic.h" 
#include "CEJ/Ai/LucioBrainComponent.h"

#include "CEJ/Ai/AiLucio.h"

ULucioBrainComponent::ULucioBrainComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // 외부에서 호출
}

void ULucioBrainComponent::BeginPlay()
{
    Super::BeginPlay();
    EnsurePointers();
    EnsureTargets();
    ResolveOwnerRoleTags();
}

void ULucioBrainComponent::EnsurePointers()
{
    OwnerChar = Cast<ACharacter>(GetOwner());
    if (OwnerChar.IsValid())
    {
        MoveComp = OwnerChar->GetCharacterMovement();
        if (MoveComp)
        {
            MoveComp->MaxWalkSpeed  = BaseMovementSpeed;
            MoveComp->JumpZVelocity = BaseJumpPower;
        }
        CachedAI = Cast<AAIController>(OwnerChar->GetController());
    }
}

void ULucioBrainComponent::ForceRefreshTargets()
{
    EnsureTargets();
    ResolveOwnerRoleTags();
}

void ULucioBrainComponent::EnsureTargets()
{
    if (!BallActor.IsValid())
    {
        TArray<AActor*> Found;
        if (BallTag.IsNone())
        {
            for (TActorIterator<ABouncyBall> It(GetWorld()); It; ++It) { BallActor = *It; break; }
        }
        else
        {
            UGameplayStatics::GetAllActorsWithTag(GetWorld(), BallTag, Found);
            if (Found.Num() > 0) BallActor = Cast<ABouncyBall>(Found[0]);
        }
    }

    if (!OppGoalActor.IsValid())
    {
        TArray<AActor*> Goals;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), GoalTag, Goals);
        if (Goals.Num() > 0) OppGoalActor = Goals[0]; // 필요시 규칙 변경
    }
}

void ULucioBrainComponent::ResolveOwnerRoleTags()
{
    bIsAttacker = false;
    bIsDefender = false;

    if (!OwnerChar.IsValid()) return;
    const AActor* O = OwnerChar.Get();

    if (!AttackerTag.IsNone() && O->ActorHasTag(AttackerTag)) bIsAttacker = true;
    if (!DefenderTag.IsNone() && O->ActorHasTag(DefenderTag)) bIsDefender = true;
}

void ULucioBrainComponent::BrainTick(float DeltaSeconds)
{
    EnsurePointers();
    EnsureTargets();
    if (!OwnerChar.IsValid() || !BallActor.IsValid()) return;

    UpdateSkillStates();
    ApplyOptimalSpeedForChase();    // 상황별 속도/점프 적용
    DetermineRoleByBallLandY();     // 공격/수비 모드 결정
    UpdateStateMachine(DeltaSeconds);
}

void ULucioBrainComponent::ApplyConfig()
{
    if (AAiLucioDynamic* Owner = Cast<AAiLucioDynamic>(GetOwner()))
    {
        if (Owner->Config)
        {
            // Config 값들을 컴포넌트 멤버 변수에 복사
            // 예시:
            // BaseMovementSpeed = Owner->Config->BaseMovementSpeed;
            // BaseJumpPower = Owner->Config->BaseJumpPower;
            // 등등...
        }
    }
}

void ULucioBrainComponent::DetermineRoleByBallLandY()
{
    const FVector Land = GetBallLandLocation();
    const float   Y    = Land.Y;

    bool bPrev = bIsInAttackMode;

    if (bIsAttacker)      bIsInAttackMode = (Y >= 0.f); // 공격수는 +Y에서만 작동
    else if (bIsDefender) bIsInAttackMode = (Y <  0.f); // 수비수는 -Y에서만 작동
    else                  bIsInAttackMode = (Y >  0.f); // 동적: +Y면 공격, -Y면 수비

    if (bPrev != bIsInAttackMode) CurrentState = ELucioDynState::Idle;
}

void ULucioBrainComponent::UpdateStateMachine(float DeltaSeconds)
{
    const float LandY = GetBallLandLocation().Y;

    if (bIsAttacker)
    {
        if (LandY >= 0.f) ExecuteAttackBehavior(DeltaSeconds);
        else              CurrentState = ELucioDynState::Idle;
    }
    else if (bIsDefender)
    {
        if (LandY < 0.f)  ExecuteDefenseBehavior(DeltaSeconds);
        else              CurrentState = ELucioDynState::Idle;
    }
    else // 동적
    {
        if (LandY > 0.f)  ExecuteAttackBehavior(DeltaSeconds);
        else if (LandY < 0.f) ExecuteDefenseBehavior(DeltaSeconds);
        else              CurrentState = ELucioDynState::Idle;
    }
}

// ===== 공격 =====
void ULucioBrainComponent::ExecuteAttackBehavior(float DeltaSeconds)
{
    const FVector BallLoc = GetBallLocation();
    const FVector MyLoc   = OwnerChar->GetActorLocation();
    const float   Dist    = FVector::Dist(MyLoc, BallLoc);
    const FVector Pred    = GetBallLandLocation();

    switch (CurrentState)
    {
    case ELucioDynState::Idle:
        CurrentState = ELucioDynState::SeekBall;
        break;

    case ELucioDynState::SeekBall:
        MoveToLocation(Pred);
        if (Dist <= PossessionRadius)
            CurrentState = ELucioDynState::AttackBall;
        break;

    case ELucioDynState::AttackBall:
        if (Dist <= PossessionRadius && OppGoalActor.IsValid())
        {
            FVector KickDir = (OppGoalActor->GetActorLocation() - BallLoc).GetSafeNormal();
            KickDir.Z = 0.f;
            KickDir.Normalize();

            const FVector Impulse = KickDir * AttackKickImpulse;
            BallActor->SetBouncyBallVelocity(Impulse, OwnerChar.Get());
            CurrentState = ELucioDynState::SeekBall; // 다시 추적
        }
        else
        {
            MoveToLocation(BallLoc);
            if (Dist > PossessionRadius * 1.5f)
                CurrentState = ELucioDynState::SeekBall;
        }
        break;

    default:
        CurrentState = ELucioDynState::Idle;
        break;
    }
}

// ===== 수비 =====
void ULucioBrainComponent::ExecuteDefenseBehavior(float DeltaSeconds)
{
    const FVector BallLoc = GetBallLocation();
    const FVector MyLoc   = OwnerChar->GetActorLocation();
    const float   Dist    = FVector::Dist(MyLoc, BallLoc);
    const FVector Pred    = GetBallLandLocation();
    const bool    bDefenseSide = (Pred.Y < 0.f);

    // 수비 사이드일 때 이동속도 가산
    if (MoveComp)
        MoveComp->MaxWalkSpeed = bDefenseSide ? BaseMovementSpeed * 1.6f : BaseMovementSpeed;

    switch (CurrentState)
    {
    case ELucioDynState::Idle:
        CurrentState = bDefenseSide ? ELucioDynState::ClearBall : ELucioDynState::DefendGoal;
        break;

    case ELucioDynState::ClearBall:
        if (!bDefenseSide) { CurrentState = ELucioDynState::DefendGoal; break; }

        MoveToLocation(Pred);

        if (Dist <= PossessionRadius)
        {
            FVector ClearImpulse;
            if (OppGoalActor.IsValid())
            {
                const FVector GoalDir = (OppGoalActor->GetActorLocation() - BallLoc).GetSafeNormal();
                ClearImpulse = (GoalDir + FVector(0,0,0.3f)).GetSafeNormal() * AttackKickImpulse * 0.9f;
            }
            else
            {
                ClearImpulse = (OwnerChar->GetActorForwardVector() + FVector(0,0,0.3f)).GetSafeNormal() * AttackKickImpulse;
            }
            BallActor->SetBouncyBallVelocity(ClearImpulse, OwnerChar.Get());
            CurrentState = ELucioDynState::DefendGoal;
        }
        break;

    case ELucioDynState::DefendGoal:
        if (bDefenseSide && Dist <= PossessionRadius * 2.f)
        {
            CurrentState = ELucioDynState::ClearBall;
        }
        else if (bDefenseSide && Dist <= PossessionRadius)
        {
            FVector Imp;
            if (OppGoalActor.IsValid())
            {
                const FVector GoalDir = (OppGoalActor->GetActorLocation() - BallLoc).GetSafeNormal();
                Imp = (GoalDir + FVector(0,0,0.3f)).GetSafeNormal() * AttackKickImpulse * 0.9f;
            }
            else
            {
                Imp = FVector(0,0,1).GetSafeNormal() * AttackKickImpulse * 0.6f;
            }
            BallActor->SetBouncyBallVelocity(Imp, OwnerChar.Get());
        }
        else
        {
            // 필요시 대기/포지셔닝
        }
        break;

    default:
        CurrentState = ELucioDynState::Idle;
        break;
    }
}

// ===== 이동 & 유틸 =====
void ULucioBrainComponent::MoveToLocation(const FVector& Location)
{
    if (!CachedAI.IsValid() || !OwnerChar.IsValid()) return;

    FAIMoveRequest Req;
    Req.SetGoalLocation(Location);
    Req.SetAcceptanceRadius(AcceptanceRadius);
    Req.SetUsePathfinding(true);
    Req.SetAllowPartialPath(true);

    // 착지 지점에 더 정확히 붙고 싶으면 반경 축소
    if (BallActor.IsValid())
    {
        const FVector Land = GetBallLandLocation();
        if (FVector::Dist(Land, Location) < 50.f)
            Req.SetAcceptanceRadius(30.f);
    }

    if (AAIController* AI = Cast<AAIController>(GetOwner()->GetInstigatorController()))
    {
        //CachedAI->MoveTo(Req);
    }
    
    // 보조 입력(좀 더 강하게 밀어주기)
    const FVector Dir = (Location - OwnerChar->GetActorLocation()).GetSafeNormal();
    OwnerChar->AddMovementInput(Dir, 1.f);
}

void ULucioBrainComponent::MoveToLocationWithBoost(const FVector& Location)
{
    MoveToLocation(Location);
    if (!OwnerChar.IsValid() || !MoveComp) return;

    const FVector Dir = (Location - OwnerChar->GetActorLocation()).GetSafeNormal();
    const FVector Desired = Dir * MoveComp->MaxWalkSpeed;
    const FVector Cur = OwnerChar->GetVelocity();

    if (Cur.Size2D() < MoveComp->MaxWalkSpeed * 0.8f)
    {
        FVector Boost = (Desired - Cur) * 0.1f;
        Boost.Z = 0.f;
        OwnerChar->LaunchCharacter(Boost, false, false);
    }
}

FVector ULucioBrainComponent::GetBallLocation() const
{
    return BallActor.IsValid() ? BallActor->GetActorLocation() : FVector::ZeroVector;
}

FVector ULucioBrainComponent::GetBallLandLocation() const
{
    return BallActor.IsValid() ? BallActor->GetLandLocation() : GetBallLocation();
}

// ===== 스킬 =====
void ULucioBrainComponent::UpdateSkillStates()
{
    const float Now = GetWorld()->GetTimeSeconds();

    if (bESkillActive && Now > ESkillEndTime)     bESkillActive = false;
    if (bUltimateActive && Now > UltimateEndTime) bUltimateActive = false;
}

bool ULucioBrainComponent::CanUseESkill() const
{
    if (bESkillActive) return false;
    const float Now = GetWorld()->GetTimeSeconds();
    return (Now - LastESkillUse) >= ESkillCooldown;
}

bool ULucioBrainComponent::CanUseUltimate() const
{
    if (bUltimateActive) return false;
    const float Now = GetWorld()->GetTimeSeconds();
    return (Now - LastUltimateUse) >= UltimateCooldown;
}

void ULucioBrainComponent::UseESkill()
{
    if (!CanUseESkill()) return;
    const float Now = GetWorld()->GetTimeSeconds();
    bESkillActive   = true;
    ESkillEndTime   = Now + ESkillDuration;
    LastESkillUse   = Now;
}

void ULucioBrainComponent::UseUltimate()
{
    if (!CanUseUltimate()) return;
    const float Now = GetWorld()->GetTimeSeconds();
    bUltimateActive   = true;
    UltimateEndTime   = Now + UltimateDuration;
    LastUltimateUse   = Now;
}

// 상황에 따른 최적 속도/점프 적용
void ULucioBrainComponent::ApplyOptimalSpeedForChase()
{
    if (!MoveComp || !OwnerChar.IsValid() || !BallActor.IsValid()) return;

    const FVector Land = GetBallLandLocation();
    const float   LandY = Land.Y;
    const float   Dist  = FVector::Dist(OwnerChar->GetActorLocation(), Land);

    float Desired = BaseMovementSpeed;

    if (bUltimateActive)
    {
        Desired = BaseMovementSpeed * UltimateSpeedMultiplier;
        MoveComp->JumpZVelocity = BaseJumpPower * UltimateJumpMultiplier;
    }
    else if (bESkillActive)
    {
        Desired = BaseMovementSpeed * ESkillSpeedMultiplier;
        MoveComp->JumpZVelocity = BaseJumpPower;
    }
    else
    {
        bool bEmergency = false;

        if ((bIsDefender || (!bIsAttacker && LandY < 0.f)) && Dist > 300.f) bEmergency = true;
        else if ((bIsAttacker || (!bIsDefender && LandY > 0.f)) && Dist > 400.f) bEmergency = true;

        if (bEmergency)
        {
            if (CanUseUltimate()) { UseUltimate(); Desired = BaseMovementSpeed * UltimateSpeedMultiplier; MoveComp->JumpZVelocity = BaseJumpPower * UltimateJumpMultiplier; }
            else if (CanUseESkill()) { UseESkill(); Desired = BaseMovementSpeed * ESkillSpeedMultiplier; }
            else { Desired = BaseMovementSpeed * 1.3f; } // 미세 부스트
        }
    }

    MoveComp->MaxWalkSpeed    = Desired;
    MoveComp->MaxAcceleration = 5000.f;
}
