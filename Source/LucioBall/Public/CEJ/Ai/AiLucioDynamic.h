#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Animation/AnimSequence.h"
#include "CEJ/Ai/LucioBrainComponent.h"
#include "NiagaraSystem.h"
#include "CEJ/Ai/LucioDataAsset.h"
#include "AiLucioDynamic.generated.h"

// Forward declarations
class AAIController;
class ABouncyBall;
class ULucioDataAsset;
class ULucioBrainComponent;

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
    virtual void OnConstruction(const FTransform& Transform) override;
    
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config", meta=(ExposeOnSpawn="true"))
    ULucioDataAsset* Config = nullptr;

    /*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lucio")
    TObjectPtr<ULucioBrainComponent> BrainComp = nullptr;
    */

    float BaseMovementSpeed;
    float BaseJumpPower;

    UFUNCTION(CallInEditor, Category="Config")
    void ApplyConfig();
    
    UPROPERTY() UMaterialInstanceDynamic* DynMat0 = nullptr;
    UPROPERTY() UMaterialInstanceDynamic* DynMat1 = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Skills|Runtime")
    bool bESkillActive = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Skills|Runtime")
    bool bUltimateActive = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Skills|Runtime")
    float ESkillEndTime = 0.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Skills|Runtime")
    float UltimateEndTime = 0.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Skills|Runtime")
    float LastESkillUse = 0.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Skills|Runtime")
    float LastUltimateUse = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    int32 PlayerID = 1;
    
    /*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    bool bDebug = false;*/

    // 역할 태그 설정 (이 태그들을 가진 경우 해당 역할로 고정)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Tags")
    FName AttackerTag = FName("Attacker");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Tags")
    FName DefenderTag = FName("Defender");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Tags")
    FName BallTag = FName("BouncyBall");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Tags")
    FName GoalTag = FName("SoccerGoal");

    /*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    FName AttackerTag; // 공격형

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    FName DefenderTag; // 수비형

    // ========== 타겟 탐지 설정 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Settings")
    FName BallTag;*/

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Settings")
    FName OwnGoalTag; // AI가 넣어야 할 골대 (Y < 0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Settings")
    FName OppGoalTag = TEXT("PlayerGoal"); // 플레이어 골대 (Y > 0, AI가 수비)

    // ========== 행동 설정 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float PossessionRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float AttackDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float DefenseDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float AcceptanceRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float AttackKickImpulse = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float DefenseKickImpulse = 2500.0f;

    /*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float BaseMovementSpeed = 800.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float BaseJumpPower = 700.0f;*/
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float WallRunSpeedMultiplier = 1.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ESkillSpeedMultiplier = 1.6f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float UltimateSpeedMultiplier = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float UltimateJumpMultiplier = 1.5f;
    
    // === 공격 관련 스탯 ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float BasicAttackForce = 1000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float BasicAttackRange = 80.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float BasicAttackCooldown = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float SkillAttackForce = 2000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float SkillAttackRange = 150.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float SkillAttackCooldown = 1.0f;
    
    // === 스킬 쿨타임 및 지속시간 ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    float ESkillCooldown = 6.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    float ESkillDuration = 3.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    float UltimateCooldown = 30.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    float UltimateDuration = 8.0f;
    
    // === 스킬 상태 변수들 ===
    /*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skills")
    bool bESkillActive = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skills")
    bool bUltimateActive = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skills")
    float ESkillEndTime = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skills")
    float UltimateEndTime = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skills")
    float LastESkillUse = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skills")
    float LastUltimateUse = 0.0f;*/

   
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Lucio")
    class ULucioBrainComponent* BrainComp;
    
    // 점프 관련 변수들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* JumpAnim;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    bool bIsPlayingJumpAnim = false;

    UPROPERTY(EditAnywhere, Category="Lucio|Dash") bool  bJumpDashEnabled = true;
    UPROPERTY(EditAnywhere, Category="Lucio|Dash") float JumpDashStrength = 1000.f;
    UPROPERTY(EditAnywhere, Category="Lucio|Dash") float JumpDashUpBoost  = 0.f;
    UPROPERTY(EditAnywhere, Category="Lucio|Dash") float DashBallZThreshold = 700.f;
    UPROPERTY(EditAnywhere, Category="Lucio|Dash") float JumpDashCooldown = 1.5f;
    UPROPERTY(EditAnywhere, Category="Lucio|Dash") UNiagaraSystem* DashVFX = nullptr;

    FTimerHandle JumpDashCDTimer;
    bool bJumpDashReady = true;

    bool CanUseJumpDash() const; 
    
    
    // 타이머 핸들
    FTimerHandle JumpAnimResetTimer;
    
    // 물리 계산을 위한 변수들
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
    float RunStartTime = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
    float JumpStartTime = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
    float InitialRunSpeed = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
    float InitialJumpVelocity = 0.8f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
    bool bIsRunning = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
    bool bIsJumping = false;
    
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

    // === GetLandLocation 추적 시스템 ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BallTracking")
    FVector PreviousLandLocation = FVector::ZeroVector;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BallTracking")
    bool bIsTrackingLandLocation = false;
    
    


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
    bool bIsInAttackMode;
    bool bIsAttacker;  // 공격형인지 여부 (태그 기반)
    bool bIsDefender;  // 수비형인지 여부 (태그 기반)
    
    // 고급 이동 상태
    bool bUsingAdvancedMovement = true;
    float CurrentAdvancedSpeed = 0.0f;
    FVector AdvancedMovementDirection = FVector::ZeroVector;
    
    void EnsureTargets();
    void CheckPlayerRole(); // 태그 기반 역할 확인
    bool DetermineRole();
    void UpdateStateMachine(float DeltaTime);

    //움직임 관련
    void ExecuteAttackBehavior(float DeltaTime);
    void ExecuteDefenseBehavior(float DeltaTime);
    void MoveToLocation(const FVector& Location);

    //애니메이션
    void HandleMovementAnimation();

    //ui 텍스트
    void UpdateRoleText();
    void UpdateTextRotation(); // 텍스트가 카메라를 바라보도록 업데이트

    //위치
    FVector GetBallLocation() const;
    FVector GetBallLandLocation() const;
    
    /*FVector GetAIGoalLocation() const;      // AI가 넣어야 할 골대 (Y < 0)
    FVector GetPlayerGoalLocation() const;  // 플레이어 골대 (Y > 0, 수비 대상)
    
    FVector GetDefensePosition() const;
    FVector GetAttackPosition() const;*/

    //bool IsBallNearby(float Distance = 0.0f) const;
    bool IsBallNearby(float Threshold) const;
    bool IsBallClose() const;

    void DrawDebugInfo() const;

public:

    UFUNCTION(BlueprintCallable, Category = "BallTracking")
    void CheckAndRespondToLandLocationChange();
    
    UFUNCTION(BlueprintCallable, Category = "BallTracking")
    void RespondToLandLocationChange(const FVector& NewLandLocation);
    
    // === 스킬 관련 함수들 ===
    UFUNCTION(BlueprintCallable, Category = "Skills")
    void UpdateSkillStates();
    
    UFUNCTION(BlueprintCallable, Category = "Skills")
    void OptimizeSpeedForBallChase();
    
    UFUNCTION(BlueprintCallable, Category = "Skills")
    bool CanUseESkill() const;
    
    UFUNCTION(BlueprintCallable, Category = "Skills")
    bool CanUseUltimate() const;
    
    UFUNCTION(BlueprintCallable, Category = "Skills")
    void UseESkill();
    
    UFUNCTION(BlueprintCallable, Category = "Skills")
    void UseUltimate();
    
    // === 이동 관련 함수들 ===
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveToLocationWithMaxSpeed(const FVector& Location);
    
    // 점프 관련 함수들
    UFUNCTION()
    void HandleJumpBehavior();
    
    UFUNCTION()
    void ResetJumpAnimFlag();
    
    // 물리 계산 함수
    UFUNCTION()
    void CalculateAndDisplayPhysics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "AI Info")
    bool IsInAttackMode() const { return bIsInAttackMode; }

    UFUNCTION(BlueprintCallable, Category = "AI Info")
    ELucioDynamicState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "AI Info")
    FVector GetCurrentBallLocation() const { return GetBallLocation(); }

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

    // UI 제어 
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