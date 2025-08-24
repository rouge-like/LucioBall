#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Animation/AnimSequence.h"
#include "AiLucioDynamic.generated.h"

// Forward declarations
class AAIController;
class ABouncyBall;

UENUM(BlueprintType)
enum class ELucioDynamicState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    SeekBall    UMETA(DisplayName = "SeekBall"),
    AttackBall  UMETA(DisplayName = "AttackBall"),
    DefendGoal  UMETA(DisplayName = "DefendGoal"),
    ClearBall   UMETA(DisplayName = "ClearBall")
};

UCLASS()
class LUCIOBALL_API AAiLucioDynamic : public ACharacter
{
    GENERATED_BODY()

public:
    AAiLucioDynamic();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ========== AI 설정 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    int32 PlayerID = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    bool bDebug = true;

    // 역할 태그 설정 (이 태그들을 가진 경우 해당 역할로 고정)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    FName AttackerTag = TEXT("Attacker"); // 이 태그가 있으면 공격형

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    FName DefenderTag = TEXT("Defender"); // 이 태그가 있으면 수비형

    // ========== 타겟 탐지 설정 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Settings")
    FName BallTag = TEXT("Ball");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Settings")
    FName OwnGoalTag = TEXT("SoccerGoal"); // AI가 넣어야 할 골대 (Y < 0)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Settings")
    FName OppGoalTag = TEXT("PlayerGoal"); // 플레이어 골대 (Y > 0, AI가 수비)

    // ========== 행동 설정 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float PossessionRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float AttackDistance = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float DefenseDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float AcceptanceRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float AttackKickImpulse = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float DefenseKickImpulse = 2500.0f;

    // ========== 고급 이동 설정 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Movement")
    float WallRunAcceleration = 200.0f; // 벽달리기 가속도

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Movement")
    float DiveAcceleration = 200.0f; // 급강하 가속도

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Movement")
    float MaxAdvancedSpeed = 1200.0f; // 고급 이동 최대 속도

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Movement")
    float InterceptThreshold = 2.0f; // 공을 가로챌 수 있는 시간 여유분 (초)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Movement")
    bool bUseAdvancedMovement = true; // 고급 이동 사용 여부

    // ========== 애니메이션 설정 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimSequence* MoveAnim;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsPlayingMoveAnim = false;

    // ========== UI 설정 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    UTextRenderComponent* RoleTextComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    bool bShowRoleText = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    float TextHeightOffset = 150.0f;

private:
    // ========== 캐시된 참조 ==========
    UPROPERTY()
    TWeakObjectPtr<AAIController> CachedAI;

    UPROPERTY()
    TWeakObjectPtr<ABouncyBall> BallActor;

    UPROPERTY()
    TWeakObjectPtr<AActor> OwnGoalActor; // AI가 넣어야 할 골대

    UPROPERTY()
    TWeakObjectPtr<AActor> OppGoalActor; // 플레이어 골대 (수비 대상)

    // ========== 상태 관리 ==========
    ELucioDynamicState CurrentState = ELucioDynamicState::Idle;
    bool bIsInAttackMode = false;
    bool bIsAttacker = false;  // 공격형인지 여부 (태그 기반)
    bool bIsDefender = false;  // 수비형인지 여부 (태그 기반)
    
    // 고급 이동 상태
    bool bUsingAdvancedMovement = false;
    float CurrentAdvancedSpeed = 0.0f;
    FVector AdvancedMovementDirection = FVector::ZeroVector;

    // ========== 핵심 함수 ==========
    void EnsureTargets();
    void CheckPlayerRole(); // 태그 기반 역할 확인
    bool DetermineRole();
    void UpdateStateMachine(float DeltaTime);

    // ========== 행동 함수 ==========
    void ExecuteAttackBehavior(float DeltaTime);
    void ExecuteDefenseBehavior(float DeltaTime);
    void MoveToLocation(const FVector& Location);
    void KickBallTowards(const FVector& Target, float Impulse);

    // ========== 고급 이동 함수 ==========
    bool ShouldUseAdvancedMovement(const FVector& TargetLocation, float RequiredTime);
    void ExecuteAdvancedMovement(const FVector& TargetLocation, float DeltaTime);
    float CalculateBallLandingTime();
    float CalculateTimeToReachTarget(const FVector& TargetLocation);
    FVector PredictBallLandingPosition(float TimeAhead = 0.0f);

    // ========== 애니메이션 함수 ==========
    void HandleMovementAnimation();

    // ========== UI 함수 ==========
    void UpdateRoleText();
    void UpdateTextRotation(); // 텍스트가 카메라를 바라보도록 업데이트

    // ========== 위치 계산 함수 ==========
    FVector GetBallLocation() const;
    FVector GetBallLandLocation() const;
    
    // 새로운 명확한 함수들
    FVector GetAIGoalLocation() const;      // AI가 넣어야 할 골대 (Y < 0)
    FVector GetPlayerGoalLocation() const;  // 플레이어 골대 (Y > 0, 수비 대상)
    
    // 이전 함수들과의 호환성 유지
    FVector GetOwnGoalLocation() const;
    FVector GetOppGoalLocation() const;
    
    FVector GetDefensePosition() const;
    FVector GetAttackPosition() const;

    // ========== 유틸리티 함수 ==========
    bool IsBallNearby(float Distance = 0.0f) const;
    bool IsBallInNegativeY() const;

    // ========== 디버그 함수 ==========
    void DrawDebugInfo() const;

public:
    // ========== Blueprint 접근 가능한 함수 ==========
    UFUNCTION(BlueprintCallable, Category = "AI Info")
    bool IsInAttackMode() const { return bIsInAttackMode; }

    UFUNCTION(BlueprintCallable, Category = "AI Info")
    ELucioDynamicState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "AI Info")
    FVector GetCurrentBallLocation() const { return GetBallLocation(); }

    UFUNCTION(BlueprintCallable, Category = "AI Info")
    bool IsBallClose() const { return IsBallNearby(); }

    UFUNCTION(BlueprintCallable, Category = "AI Info")
    bool IsAttacker() const { return bIsAttacker; }

    UFUNCTION(BlueprintCallable, Category = "AI Info")
    bool IsDefender() const { return bIsDefender; }

    UFUNCTION(BlueprintCallable, Category = "AI Info")
    bool IsUsingAdvancedMovement() const { return bUsingAdvancedMovement; }

    UFUNCTION(BlueprintCallable, Category = "AI Info")
    float GetCurrentSpeed() const { return bUsingAdvancedMovement ? CurrentAdvancedSpeed : 600.0f; }

    UFUNCTION(BlueprintCallable, Category = "AI Info")
    FString GetPlayerRole() const 
    { 
        if (bIsAttacker) return TEXT("Attacker");
        if (bIsDefender) return TEXT("Defender");
        return TEXT("Dynamic");
    }

    // UI 제어 함수들
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetRoleTextVisibility(bool bVisible) 
    { 
        bShowRoleText = bVisible; 
        if (RoleTextComponent)
        {
            RoleTextComponent->SetVisibility(bVisible);
        }
    }

    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetRoleTextHeight(float Height) 
    { 
        TextHeightOffset = Height;
        if (RoleTextComponent)
        {
            RoleTextComponent->SetRelativeLocation(FVector(0.0f, 0.0f, TextHeightOffset));
        }
    }
};