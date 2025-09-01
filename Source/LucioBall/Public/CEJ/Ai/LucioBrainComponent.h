// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "LucioBrainComponent.generated.h"

class ABouncyBall;
class AAIController;

UENUM(BlueprintType)
enum class ELucioDynState : uint8
{
    Idle        UMETA(DisplayName="Idle"),
    SeekBall    UMETA(DisplayName="SeekBall"),
    AttackBall  UMETA(DisplayName="AttackBall"),
    DefendGoal  UMETA(DisplayName="DefendGoal"),
    ClearBall   UMETA(DisplayName="ClearBall")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LUCIOBALL_API ULucioBrainComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULucioBrainComponent();

    // 외부(액터)에서 원하는 경우 강제 갱신
    UFUNCTION(BlueprintCallable, Category="Lucio|Brain")
    void ForceRefreshTargets();

    // 액터 Tick에서 이 컴포넌트만 호출하면 됨
    UFUNCTION(BlueprintCallable, Category="Lucio|Brain")
    void BrainTick(float DeltaSeconds);
    void ApplyConfig();

    // 역할 태그 세팅(맵에서 붙인 태그를 액터가 읽었다면 그대로 전달)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Tags")
    FName AttackerTag = FName("Attacker");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Tags")
    FName DefenderTag = FName("Defender");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Tags")
    FName BallTag = FName("BouncyBall");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Tags")
    FName GoalTag = FName("SoccerGoal");

    // 거리/속도/임펄스 등 게임 밸런싱 파라미터
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Tuning")
    float AcceptanceRadius = 80.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Tuning")
    float PossessionRadius = 150.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Tuning")
    float AttackKickImpulse = 2000.f;

    // 기본 이동/점프 (필요시 액터에서 초기 값 주입 가능)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Move")
    float BaseMovementSpeed = 800.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Move")
    float BaseJumpPower = 700.f;

    // 스킬 배수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Skill")
    float ESkillSpeedMultiplier = 1.6f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Skill")
    float UltimateSpeedMultiplier = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Skill")
    float UltimateJumpMultiplier = 1.5f;

    // 스킬 쿨/지속
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Skill")
    float ESkillCooldown = 6.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Skill")
    float ESkillDuration = 3.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Skill")
    float UltimateCooldown = 30.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Skill")
    float UltimateDuration = 8.f;

    // 디버그 스위치
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Debug")
    bool bDebug = false;

private:
    // 내부 상태
    TWeakObjectPtr<AAIController> CachedAI;
    TWeakObjectPtr<ABouncyBall>   BallActor;
    TWeakObjectPtr<AActor>        OppGoalActor;

    TWeakObjectPtr<ACharacter>    OwnerChar;
    UCharacterMovementComponent*  MoveComp = nullptr;

    bool bIsAttacker = false;
    bool bIsDefender = false;
    bool bIsInAttackMode = false;

    bool bESkillActive = false;
    bool bUltimateActive = false;
    float ESkillEndTime = 0.f;
    float UltimateEndTime = 0.f;
    float LastESkillUse = 0.f;
    float LastUltimateUse = 0.f;

    ELucioDynState CurrentState = ELucioDynState::Idle;

    // ====== 내부 흐름 ======
    virtual void BeginPlay() override;

    void EnsurePointers();
    void EnsureTargets();
    void DetermineRoleByBallLandY();      // bIsInAttackMode 갱신
    void UpdateStateMachine(float DeltaSeconds);

    // 행동들
    void ExecuteAttackBehavior(float DeltaSeconds);
    void ExecuteDefenseBehavior(float DeltaSeconds);

    // 공통 유틸
    void MoveToLocation(const FVector& Location);
    void MoveToLocationWithBoost(const FVector& Location); // 긴급시 가속 보장
    FVector GetBallLocation() const;
    FVector GetBallLandLocation() const;

    // 스킬
    void UpdateSkillStates();
    bool CanUseESkill() const;
    bool CanUseUltimate() const;
    void UseESkill();
    void UseUltimate();

    // 속도 최적화
    void ApplyOptimalSpeedForChase();

    // 역할 태그 판정
    void ResolveOwnerRoleTags();
};
