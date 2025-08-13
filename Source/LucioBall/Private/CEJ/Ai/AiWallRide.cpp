#include "CEJ/Ai/AiWallRide.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UWallRideComponent::UWallRideComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UWallRideComponent::BeginPlay()
{
    Super::BeginPlay();
}

FVector UWallRideComponent::GetGravityVector() const
{
    const UWorld* World = GetWorld();
    const float Gz = World ? World->GetGravityZ() : -980.f; // cm/s^2 (UE 기본 -980)
    return FVector(0.f, 0.f, Gz);
}

void UWallRideComponent::BeginWallRide(const FVector& InWallNormal, const FVector& InTangent, float CurrentSpeed, float TargetSpeed)
{
    WallNormal   = InWallNormal.GetSafeNormal();
    WallTangent  = InTangent.GetSafeNormal();
    Speed        = CurrentSpeed;

    // 1) 진입 가속도: a_enter = (Vtarget − V0) / T_enter
    const float SafeTEnter = FMath::Max(0.01f, TEnter);
    AEnter = (TargetSpeed - CurrentSpeed) / SafeTEnter;

    // 2) 이탈 감속도는 이탈 시점에 다시 설정(그때의 V0 기반)하는 편이 안전
    AExit = 0.f;

    bWallRiding = true;
    bEnteringPhase = true;
    bWallExit = false;
}

void UWallRideComponent::EndWallRide()
{
    // 이탈 직전 속도 기준: a_exit = −V0 / T_exit
    const float SafeTExit = FMath::Max(0.01f, TExit);
    AExit = -Speed / SafeTExit;

    bWallExit = true;
    bWallRiding = false;
    bEnteringPhase = false;
}

void UWallRideComponent::TickComponent(float dt, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(dt, TickType, ThisTickFunction);
    if (dt <= KINDA_SMALL_NUMBER) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    if (bWallRiding)
    {
        // 1) 가속도 구성: 진입 a_enter or 슬라이드(a_friction + 중력 접선성분)
        const FVector Tangent = ComputeWallTangent();          // 벽 접선 단위벡터
        const FVector Gravity = GetGravityVector();            // (0,0, -g)
        const float   aGravTangent = FVector::DotProduct(Gravity, Tangent); // 중력의 접선 성분(부호 포함, cm/s^2)

        const float   aSlide = WallFrictionA + aGravTangent;   // 맵/재질 감각에 맞게 WallFrictionA를 살짝 음수로 두면 감속
        const float   a = bEnteringPhase ? AEnter : aSlide;

        // 2) 속도 적분 + 클램프
        Speed = FMath::Clamp(Speed + a * dt, 0.f, VMaxWall);

        // 3) 방향/속도 벡터 갱신 (벽 접선 방향으로 이동)
        Velocity = Tangent * Speed;

        // 4) 위치 적분(스윕 true: 충돌 반영)
        Owner->AddActorWorldOffset(Velocity * dt, true);

        // 5) 진입 페이즈 종료 판단(목표 속도 근접)
        if (bEnteringPhase)
        {
            const bool Reached = (AEnter >= 0.f) ? (Speed >= VMaxWall * 0.98f) : (Speed <= VMaxWall * 0.98f);
            // 혹은 일정 시간 경과/속도차 임계값으로 전환
            if (Reached) bEnteringPhase = false;
        }

        // 6) 선택: 벽에 "붙는" 느낌(법선 방향 보정)
        if (StickyNormalForce != 0.f)
        {
            // 벽 쪽으로 미세 보정(감각용). 충돌로 튕기는걸 줄이고, 너무 크면 박힘.
            const FVector Nudge = -WallNormal * StickyNormalForce * dt * dt; // s = 1/2 a t^2 느낌
            Owner->AddActorWorldOffset(Nudge, true);
        }
    }
    else if (bWallExit)
    {
        // 1) 이탈 감속: aExit < 0,  v = max(0, v + aExit * dt)
        Speed = FMath::Max(0.f, Speed + AExit * dt);

        // 2) 이탈 방향(지상 이동/입력 방향 등) — 여기선 현 Velocity 방향 유지 or 캐릭터 전방 사용
        const FVector ExitDir = Velocity.IsNearlyZero() ? Owner->GetActorForwardVector().GetSafeNormal() : Velocity.GetSafeNormal();
        Velocity = ExitDir * Speed;

        // 3) 위치 적분
        Owner->AddActorWorldOffset(Velocity * dt, true);

        // 4) 멈추면 종료
        if (Speed <= KINDA_SMALL_NUMBER)
        {
            bWallExit = false;
            Velocity = FVector::ZeroVector;
        }
    }
}
