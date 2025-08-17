// WallRideComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WallRideComponent.generated.h"

/*
 * - 벽에 붙을 때 살짝 가속 (a_enter)
 * - 벽을 달리는 동안 마찰/중력 접선 성분 반영
 * - 벽에서 떨어질 때 부드럽게 감속 (a_exit)
 */

UCLASS(ClassGroup=(Movement), meta=(BlueprintSpawnableComponent))
class LUCIOBALL_API UWallRideComponent : public UActorComponent
{
    GENERATED_BODY()

public:
     UWallRideComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;
	void BeginWallRide(const FVector& Vector, const FVector& Vector1, float V0, float Vtarget);


    // ====== 튜닝 파라미터 ======
    UPROPERTY(EditAnywhere, Category="WallRide|Detect")
    float TraceDistance = 120.f;                 // 전방 감지 거리

    UPROPERTY(EditAnywhere, Category="WallRide|Detect")
    float TraceRadiusScale = 50.f;               // “큰 캡슐(×50)” 컨셉의 반경 배수

    UPROPERTY(EditAnywhere, Category="WallRide|Detect")
    float MaxAttachAngleDeg = 50.f;              // 벽에 붙을 수 있는 최대 경사(전방 vs. -법선)

    UPROPERTY(EditAnywhere, Category="WallRide|Motion")
    float EnterAccel = 3000.f;                   // 진입 가속도 크기(원하면 0으로 꺼도 됨)

    UPROPERTY(EditAnywhere, Category="WallRide|Motion")
    float ExitDecel = -6000.f;                   // 이탈 감속도(음수로 입력)

    UPROPERTY(EditAnywhere, Category="WallRide|Motion")
    float MaxWallSpeed = 1500.f;                 // 벽 타기 최대 속도

    UPROPERTY(EditAnywhere, Category="WallRide|Motion")
    float StickStrength = 0.5f;                  // 벽에 살짝 “붙여두는” 흡착 보정(cm)

    // ====== 상태 노출 ======
    UPROPERTY(BlueprintReadOnly, Category="WallRide|State")
    bool bWallRiding = false;                    // 현재 벽타는 중?

    UPROPERTY(BlueprintReadOnly, Category="WallRide|State")
    FVector WallNormal = FVector::ZeroVector;    // 감지된 벽 법선(단위벡터)

    UPROPERTY(BlueprintReadOnly, Category="WallRide|State")
    FVector Velocity = FVector::ZeroVector;      // 현재 이동 속도 벡터(cm/s) — v
    float VMaxWall;
    bool bWallExit;

    TWeakObjectPtr<ACharacter> OwnerChar;
    TWeakObjectPtr<UCharacterMovementComponent> MoveComp;

    bool TryBeginWallRide(float DeltaTime);      // 전방 스윕으로 벽 감지 → 시작 시 true
    void TickWallRide(float DeltaTime);          // 벽타기 중 갱신: v+=a·dt, p+=v·dt
    void TickWallExit(float DeltaTime);          // 이탈 시 감속
    void EndWallRide();                          // 상태 초기화

    bool bExiting = false;                       // 이탈 중?
    FVector ExitDir = FVector::ZeroVector;       // 이탈 진행 방향(마지막 접선)
};
