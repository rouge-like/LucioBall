// Fill out your copyright notice in the Description page of Project Settings.

/**
루시우 연속 움직임 AI
(점프포인트 → 벽달리기 → 볼 킥)
*/

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "AiLucio.generated.h"

UENUM(BlueprintType)
enum class ELucioAIState : uint8
{
	SeekJumpPoint,
	Jumping,
	WallRun,
	FallingFast,
	BallKick
};

UCLASS()
class LUCIOBALL_API AAiLucio : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAiLucio();

	virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
	virtual void PossessedBy(AController* NewController) override;


protected:
    // 검색 
    void FindJumpPointOnce();
    void FindBallOnce();
    void FindGoalOnce();

    // 상태 전이
    void GotoState(ELucioAIState NewState);
    void Tick_SeekJumpPoint(float Dt);
    void Tick_Jumping(float Dt);
    void Tick_WallRun(float Dt);
    void Tick_FallingFast(float Dt);
    void Tick_BallKick(float Dt);

    bool MoveToActorSmart(AActor* Target, float Radius=120.f);
    bool MoveToLocationSmart(const FVector& Dest, float Radius=120.f);

    // 벽 탐지/계산
    bool TraceForWall(const FVector& From, const FVector& Dir, float Length, FHitResult& OutHit) const;
    FVector ComputeWallTangent(const FVector& WallNormal, const FVector& Up = FVector::UpVector) const;

    // 볼 킥 유틸
    void ComputeBallApproach(const FVector& Ball, const FVector& Goal,
                             FVector& OutApproach, FVector& OutKick, FVector& OutDirToGoal) const;
	

public:

    UPROPERTY(EditAnywhere, Category="Tags")
    FName JumpPointTag = TEXT("JumpPoint4");

    UPROPERTY(EditAnywhere, Category="Tags")
    FName WallTag = TEXT("Wall");

    UPROPERTY(EditAnywhere, Category="Tags")
    FName BallTag = TEXT("BouncyBall");

    UPROPERTY(EditAnywhere, Category="Tags")
    FName GoalTag = TEXT("SoccerGoal");

    UPROPERTY(EditAnywhere, Category="Move")
    float AcceptanceRadius = 0.1f;

    // 점프
    UPROPERTY(EditAnywhere, Category="Jump")
    float JumpZVelocity = 800.f;

    // 벽 탐지/주행
    UPROPERTY(EditAnywhere, Category="WallRun")
    float WallTraceLength = 120.f;

    UPROPERTY(EditAnywhere, Category="WallRun")
    float WallTraceRadius = 30.f;      // 스피어 트레이스 반경

    UPROPERTY(EditAnywhere, Category="WallRun")
    float MaxWallSpeed = 1500.f;

    UPROPERTY(EditAnywhere, Category="WallRun")
    float StickStrength = 30.f;        // 벽 쪽으로 살짝 붙이는 보정(cm)

    UPROPERTY(EditAnywhere, Category="WallRun")
    float WallEndProbe = 200.f;        // 진행 방향으로 벽이 더 있는지 확인 길이

    // 하강 가속
    UPROPERTY(EditAnywhere, Category="Fall")
    float FastFallGravityScale = 3.0f;

    UPROPERTY(EditAnywhere, Category="Fall")
    float NormalGravityScale = 1.0f;

    // 볼 킥
    UPROPERTY(EditAnywhere, Category="BallKick")
    float BehindBallDistance = 220.f;

    UPROPERTY(EditAnywhere, Category="BallKick")
    float KickThroughDistance = 300.f;

    UPROPERTY(EditAnywhere, Category="BallKick")
    float ApproachTolerance = 120.f;

    // 디버그
    UPROPERTY(EditAnywhere, Category="Debug")
    bool bDebug = true;

	UPROPERTY()
	UAnimSequence* MoveAnim = nullptr;

protected:
    // 상태/타깃 핸들
    ELucioAIState State = ELucioAIState::SeekJumpPoint;

    TWeakObjectPtr<AActor> JumpPoint;
    TWeakObjectPtr<AActor> BallActor;
    TWeakObjectPtr<AActor> GoalActor;

    // 벽 상태
    bool bHasWall = false;
    FVector WallNormal = FVector::ZeroVector;
    FVector WallTangent = FVector::ZeroVector;

    // 호출 최적화용 쿨다운
    float MoveCmdCooldown = 0.f;
    UPROPERTY(EditAnywhere, Category="Perf")
    float MoveCmdInterval = 0.2f; // 과도한 MoveTo 호출 방지
	
	AAIController* CachedAI;

	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	bool bIsPlayingJumpAnim = false;
};
