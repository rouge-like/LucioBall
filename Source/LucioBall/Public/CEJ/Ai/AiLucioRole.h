#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AiLucioRole.generated.h"

class AAIController;
class ABouncyBall;
class AStaticMeshActor;

UENUM(BlueprintType)
enum class ELucioRole : uint8
{
    Offense UMETA(DisplayName="Offense"),
    Defense UMETA(DisplayName="Defense")
};

UENUM(BlueprintType)
enum class ELucioState : uint8
{
    Idle,
    SeekBall,      // 공 추적(공격)
    Dribble,       // 드리블(공격)
    Shoot,         // 슛(공격)
    HoldShape,     // 자리잡기(수비)
    Intercept,     // 차단/예상지점 선점(수비)
    ClearBall      // 걷어내기(수비)
};

UCLASS()
class LUCIOBALL_API AAiLucioRole : public ACharacter
{
    GENERATED_BODY()

public:
    AAiLucioRole();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Role", meta=(DisplayName="Role"))
    ELucioRole AgentRole = ELucioRole::Defense; //Defense: 공격, Offense: 수비

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Team")
    bool bOwnHalfIsNegativeY = true; // 우리 하프가 Y<0 면 true (맵 좌표계에 맞춰 조정)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tags")
    FName BallTag = "BouncyBall";

    // 골대 태그: 수비는 OwnGoal, 공격은 OppGoal을 사용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tags")
    FName OwnGoalTag = "OwnGoal";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tags")
    FName OppGoalTag = "SoccerGoal"; // 상대 골대(예: "SoccerGoal" 로 태깅)

    // 이동/킥 파라미터
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Move")
    float AcceptanceRadius = 120.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Move")
    float PossessionRadius = 160.f; // 이 거리 이내면 '볼 소유'로 간주

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kick")
    float KickImpulse_Shoot = 4200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kick")
    float KickImpulse_Clear = 3600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kick")
    float DribbleKickImpulse = 900.f; // 적당히 밀어주는 약한 킥

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Defense")
    float BlockDistanceFromGoal = 600.f; // 골대에서 앞으로 나와 블로킹할 거리

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Defense")
    float InterceptLead = 100.f; // 예상지점 앞쪽 선점량

    // 디버그
    UPROPERTY(EditAnywhere, Category="Debug")
    bool bDebug = true;

    UPROPERTY(EditAnywhere, Category="Debug|Arrow")
    float DebugArrowSize = 40.f;

    UPROPERTY(EditAnywhere, Category="Debug|Arrow")
    float DebugArrowThickness = 3.f;

    UPROPERTY(EditAnywhere, Category="Debug|Arrow")
    float DebugArrowDuration = 1.5f;

protected:
    // 공/골대 보장
    void EnsureTargets();
    AActor* FindActorByTagPreferStaticMesh(const FName Tag) const;

    // 하프 판정
    bool IsBallInOwnHalf() const;
    bool IsBallInOppHalf() const;

    // 이동 헬퍼
    void MoveToPoint(const FVector& P);
    void MoveTowardBall(float DeltaSeconds);
    void StopChasing();

    // 슛/클리어 공용
    void KickTowards(const FVector& Target, float Impulse);
    void DrawKickDebug(const FVector& From, const FVector& Dir, float Len, const FColor& Color) const;

    // 수비용: 블로킹 포지션 / 인터셉트 포지션
    FVector ComputeBlockPoint() const;
    FVector ComputeInterceptPoint() const; // 공의 GetLandLocation() 사용

    // 공격용: 슛 or 드리블 목표
    FVector GetOppGoalLocation() const;

protected:
    ELucioState State = ELucioState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Refs")
    ABouncyBall* BallActor = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Refs")
    AActor* OwnGoalActor = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Refs")
    AActor* OppGoalActor = nullptr;

    AAIController* CachedAI = nullptr;
};
