#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AiFSM.generated.h"

// 전방 선언 (OSC/BouncyBall.h의 클래스명 가정)
class ABouncyBall;

UENUM(BlueprintType)
enum class ESimpleAIState : uint8
{
    Idle        UMETA(DisplayName="Idle"),
    TrackBall   UMETA(DisplayName="TrackBall"),
    KickToGoal  UMETA(DisplayName="KickToGoal")
};

UCLASS()
class LUCIOBALL_API AAiFSM : public ACharacter
{
    GENERATED_BODY()

public:
    AAiFSM();

    virtual void Tick(float DeltaSeconds) override;

protected:
    virtual void BeginPlay() override;

    /** 공, 골대 찾기 */
    void EnsureTargets();

    /** 하프라인( Y==0 ) 기준 공의 부호 변화 감지 */
    void UpdateHalfLineCrossing(float CurrentBallY);

    /** 공 쪽으로 이동 (AIController MoveTo 사용) */
    void MoveTowardBall(float DeltaSeconds);

    /** 충분히 가까우면 공을 골대로 차기 */
    void TryKickBall();

    /** 실제 임펄스 적용 */
    void KickBallTowardsGoal();

    /** 디버그 표시 */
    void DrawDebugInfo();

private:
    /** 간단 FSM 상태 */
    UPROPERTY(VisibleAnywhere, Category="FSM")
    ESimpleAIState State = ESimpleAIState::Idle;

public:
    /** BouncyBall & Goal 참조 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Refs")
    ABouncyBall* BallActor = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Refs")
    AActor* GoalActor = nullptr;

    bool IsBallInOurHalf() const;     
    void StopChasing();

    UPROPERTY(EditAnywhere, Category="Debug|Arrow")
    float DebugArrowSize = 40.f;

    UPROPERTY(EditAnywhere, Category="Debug|Arrow")
    float DebugArrowThickness = 3.f;

    UPROPERTY(EditAnywhere, Category="Debug|Arrow")
    float DebugArrowDuration = 1.5f;
    
    /** 태그/이름 설정 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config|Tags")
    FName BallTag = "BouncyBall";

    /** GoalTag 를 쓰거나, GoalName 을 직접 써도 됨 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config|Tags")
    FName GoalTag = "Goal";

    /** 특정 이름의 골대 찾고 싶으면 여기 지정 (“Soccer_Goal_1”) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config|Names")
    FName GoalName = "SoccerGoal";

    /** 킥 파라미터 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kick")
    float KickDistance = 180.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kick")
    float KickImpulse = 3500.f;

    /** Move 목적지에 도착했다고 간주하는 반경 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Move")
    float AcceptanceRadius = 120.f;

    /** 하프라인 교차 감지용 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="HalfLine")
    bool bBallCrossedHalfLine = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HalfLine")
    float HalfLineCheckRadius = 100.f; // 사용처 여지

private:
    /** 직전 프레임 Ball Y (부호 변화를 잡기 위한 저장값) */
    float LastBallY = 0.f;

    /** AIController 캐시 */
    class AAIController* CachedAI = nullptr;

    /** 디버깅 */
    UPROPERTY(EditAnywhere, Category="Debug")
    bool bDebug = true;
};
