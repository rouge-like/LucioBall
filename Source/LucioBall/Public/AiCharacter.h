// AiCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AiCharacter.generated.h"

UCLASS()
class AAiCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AAiCharacter();

protected:
    // Unreal 기본 수명주기
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    // ====== 시작 시 1회 벽 감지/벽달리기 → 이후 MoveTo ======
    UFUNCTION()
    void PerformBeginWallRunCheck();              // BeginPlay 직후(0.05s) 1회 실행
    bool SphereTraceWall(struct FHitResult& OutHit) const;   // 스피어 스윕 감지(전/좌/우)
    bool TryStartWallRun(const struct FHitResult& Hit);      // 조건 충족 시 벽 달리기 시작
    void EndWallRun();                                        // 일정 시간 뒤 종료
    FVector ComputeWallRunDir(const FVector& WallNormal) const; // 벽을 따라 달릴 방향

    // ====== 타깃 추적 ======
    void IssueMove();            // MoveToActor(타깃: BP_BouncyBall)
    void TryFindTarget();        // Tag로 대상 재탐색

    // ====== 벽 달리기 파라미터 ======
    UPROPERTY(EditAnywhere, Category="WallRun|Trace")
    float TraceDistance = 350.f;        // 전방/측면 감지 거리

    UPROPERTY(EditAnywhere, Category="WallRun|Trace")
    float TraceRadius = 30.f;           // 스윕 반경(캡슐 반경과 유사)

    // 벽 각도 필터 (Dot(WallNormal, Up))
    UPROPERTY(EditAnywhere, Category="WallRun|Filter")
    float MinWallNormalUpDot = 0.05f;   // 너무 천장에 가까우면 제외(음수 큰 값)
    UPROPERTY(EditAnywhere, Category="WallRun|Filter")
    float MaxWallNormalUpDot = 0.6f;    // 바닥/완만 경사 제외

    UPROPERTY(EditAnywhere, Category="WallRun|Motion")
    float WallRunSpeed   = 900.f;       // 벽을 따라가는 수평 속도 성분
    UPROPERTY(EditAnywhere, Category="WallRun|Motion")
    float WallRunUpBoost = 180.f;       // 살짝 띄워주는 상향 성분
    UPROPERTY(EditAnywhere, Category="WallRun|Motion")
    float WallRunDuration = 0.6f;       // 유지 시간

    bool bIsWallRunning = false;
    FTimerHandle WallRunTimer;

    // ====== 타깃 태그 ======
public:
    UPROPERTY(EditAnywhere, Category="AI|Target")
    FName TargetTag = "BP_BouncyBall";  // BP_BouncyBall에 이 태그를 달아두세요

    // ====== Move 재명령 제어 ======
    UPROPERTY(EditAnywhere, Category="AI|Move")
    float AcceptanceRadius   = 120.f;

    UPROPERTY(EditAnywhere, Category="AI|Move")
    float ReacquireInterval  = 1.0f;    // 타깃 분실 시 재탐색 주기

    UPROPERTY(EditAnywhere, Category="AI|Move")
    float ReissueThreshold   = 100.f;   // 타깃이 이만큼 이동하면 재명령

    UPROPERTY(EditAnywhere, Category="AI|Move")
    float CommandInterval    = 0.3f;    // 너무 잦은 재명령 방지

private:
    TWeakObjectPtr<AActor> TargetActor;
    FVector LastIssuedGoal = FVector::ZeroVector;
    float TimeSinceCmd = 0.f;
    float TimeSinceFindTry = 0.f;
};
