/*
==================전체 흐름도=================
        if (벽타는 중)
        {
            // 가속도 적용해서 속도 업데이트
            Speed += a * dt;

            // 벽의 진행 방향 구하기
            Velocity = 벽접선방향 * Speed;

            // 위치 이동
            AddActorWorldOffset(Velocity * dt);
        }
        else if (벽에서 떨어지는 중)
        {
            // 감속 적용
            Speed += aExit * dt;

            // 떨어지는 방향으로 이동
            Velocity = ExitDir * Speed;
            AddActorWorldOffset(Velocity * dt);
        }
        
==================요약=============================
        속도 공식 = 얼마나 빨라지는지
        위치 공식 = 그 속도로 얼마나 갔는지
        a_enter = 벽 붙을 때 가속
        a_exit = 벽 떨어질 때 감속
        a_gravity_tangent = 경사 때문에 미끄러지는 힘
        
 */

#include "CEJ/Ai/WallRideComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"

UWallRideComponent::UWallRideComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UWallRideComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerChar = Cast<ACharacter>(GetOwner());
    if (OwnerChar.IsValid())
    {
        MoveComp = OwnerChar->GetCharacterMovement();
    }
}

void UWallRideComponent::TickComponent(float dt, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(dt, TickType, ThisTickFunction);
    if (!OwnerChar.IsValid() || !MoveComp.IsValid()) return;

    // 1) 이미 벽타는 중이면 계속 업데이트
    if (bWallRiding)
    {
        TickWallRide(dt);
        return;
    }

    // 2) 이탈 중이면 감속/이동 후 종료 판단
    if (bExiting)
    {
        TickWallExit(dt);
        return;
    }

    // 3) 평상시 → 전방 스윕으로 벽 탐지 시도
    TryBeginWallRide(dt);
}

void UWallRideComponent::BeginWallRide(const FVector& Vector, const FVector& Vector1, float V0, float Vtarget)
{
    // Example implementation — fill with your logic
    WallNormal  = Vector.GetSafeNormal();
    const FVector Tangent = Vector1.GetSafeNormal();

    // you probably store speed somewhere (add a member if needed)
    // CurrentSpeed = V0; TargetSpeed = Vtarget;

    bWallRiding = true;
    // ... rest of your setup
}

bool UWallRideComponent::TryBeginWallRide(float dt)
{
    //collision 캡슐로 벽감지
    UCapsuleComponent* Capsule = OwnerChar->GetCapsuleComponent();
    if (!Capsule) return false;

    const float BaseRadius = Capsule->GetScaledCapsuleRadius();  // 현재 캡슐 반경
    const float SweepRadius = BaseRadius * TraceRadiusScale;     // ×50 등 배수 적용

    const FVector Start = OwnerChar->GetActorLocation();         // 스윕 시작 지점
    const FVector Fwd   = OwnerChar->GetActorForwardVector();    // 캐릭터 전방
    const FVector End   = Start + Fwd * TraceDistance;           // 스윕 끝 지점

    FCollisionQueryParams Params(SCENE_QUERY_STAT(WallRideDetect), false, OwnerChar.Get());
    FHitResult Hit;

    // 구(스피어) 스윕: 크고 둥글게 훑어서 가까운 벽 잡는다
    const bool bHit = GetWorld()->SweepSingleByChannel(
        Hit, Start, End, FQuat::Identity,
        ECC_Visibility, FCollisionShape::MakeSphere(SweepRadius), Params);

    // 디버그 시각화
    // DrawDebugSphere(GetWorld(), End, SweepRadius, 16, bHit?FColor::Green:FColor::Red, false, 0.05f);

    if (!bHit) return false;

    // 벽 각도 판정: 전방 vs. -법선이 충분히 정면이어야 붙을 수 있다
    const FVector N = Hit.Normal.GetSafeNormal();                 // 벽 법선
    const float Facing = FMath::RadiansToDegrees( FMath::Acos( FMath::Clamp(FVector::DotProduct(Fwd, -N), -1.f, 1.f) ) );
    if (Facing > MaxAttachAngleDeg) return false;                 // 너무 비껴가면 시작 안 함

    // 벽타기 시작
    bWallRiding = true;
    bExiting    = false;
    WallNormal  = N;

    // 이동 모드 변경: 우리가 v,p 직접 적분하므로 중력/지상마찰 영향 제거
    MoveComp->SetMovementMode(MOVE_Flying);
    MoveComp->StopMovementImmediately();

    // 초기 속도 = 현재 속도의 벽 평면 접선 성분
    const FVector CurrVel = OwnerChar->GetVelocity();
    Velocity = FVector::VectorPlaneProject(CurrVel, WallNormal);

    // 붙이기: 벽 쪽으로 약간 흡착(너무 크면 떨림)
    OwnerChar->AddActorWorldOffset(-WallNormal * StickStrength, false);

    return true;
}

void UWallRideComponent::TickWallRide(float dt)
{
    // ====== 등가속 원리 ======
    // v += a * Δt
    // p += v * Δt
    //
    // a = (0, 0, Gz) 를 벽 평면으로 '투영'해서 접선 성분만 사용
    //  - UE는 +Z가 위, GetGravityZ()는 음수(-980)
    const float Gz = GetWorld() ? GetWorld()->GetGravityZ() : -980.f;   // 보통 -980 cm/s^2
    const FVector WorldGravity(0.f, 0.f, Gz);                           // 사용자가 말한 a=(0,-g,0)과 동일 개념이지만
                                                                         // UE 좌표계에 맞춰 Z축으로 적용
    const FVector TangentGravity = FVector::VectorPlaneProject(WorldGravity, WallNormal);
    FVector a = TangentGravity;                                          // 기본: 경사 때문에 미끄러지는 힘

    // (선택) 진입 가속: 목표 속도까지 당기고 싶다면 접선 방향으로 가속 추가
    if (!Velocity.IsNearlyZero() && EnterAccel > 0.f)
    {
        const FVector TangentDir = Velocity.GetSafeNormal();
        a += TangentDir * EnterAccel;
    }

    // 1) 속도 적분: v ← v + a·dt
    Velocity += a * dt;

    // 2) 벽 안쪽 성분 제거(혹시라도 수치 오차로 벽 안으로 파고드는 것을 방지)
    Velocity = FVector::VectorPlaneProject(Velocity, WallNormal);

    // 3) 최대 속도 제한
    const float Speed = Velocity.Size();
    if (Speed > MaxWallSpeed)
    {
        Velocity = Velocity * (MaxWallSpeed / Speed);
    }

    // 4) 위치 적분: p ← p + v·dt  (스윕으로 충돌 검사)
    const FVector Delta = Velocity * dt;

    FHitResult Hit;
    OwnerChar->AddActorWorldOffset(Delta, true, &Hit);                   // sweep=true
    if (Hit.IsValidBlockingHit())
    {
        // 막히면 남은 거리로 표면을 따라 슬라이드
        const FVector Remainder = Delta * (1.f - Hit.Time);
        const FVector SlideDelta = FVector::VectorPlaneProject(Remainder, Hit.Normal);
        OwnerChar->AddActorWorldOffset(SlideDelta, true);
    }

    // 5) 계속 붙여주기(약한 흡착) — 너무 크면 떨림 유발
    OwnerChar->AddActorWorldOffset(-WallNormal * StickStrength, false);

    // 6) 벽이 끝났는지 재확인(더 이상 감지가 안 되면 이탈로 전환)
    //    벽과의 접촉이 끊기거나 전방 각도가 나빠지면 Exit
    //    — 간단하게 “앞으로 한 번 더 스윕”으로 판단
    const FVector Start = OwnerChar->GetActorLocation();
    const FVector Fwd   = OwnerChar->GetActorForwardVector();
    const FVector End   = Start + Fwd * TraceDistance;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(WallRideFollow), false, OwnerChar.Get());
    FHitResult FollowHit;
    const bool bStillHit = GetWorld()->SweepSingleByChannel(
        FollowHit, Start, End, FQuat::Identity,
        ECC_Visibility, FCollisionShape::MakeSphere(OwnerChar->GetCapsuleComponent()->GetScaledCapsuleRadius() * TraceRadiusScale), Params);

    if (!bStillHit)
    {
        // 마지막 접선 방향 저장 → 이탈 단계에서 감속하며 진행
        ExitDir = Velocity.IsNearlyZero() ? Fwd : Velocity.GetSafeNormal();
        bExiting = true;
        EndWallRide(); // 내부에서 bWallRiding=false, 모드/중력 복구는 아래에서
    }
}

void UWallRideComponent::TickWallExit(float dt)
{
    // 이탈 시: 감속만 적용 (요구 요약: a_exit = 벽 떨어질 때 감속)
    if (!OwnerChar.IsValid()) return;

    // 이동 모드: 낙하/중력 복구
    MoveComp->SetMovementMode(MOVE_Falling);
    // 속도는 ExitDir을 따라 감속
    const float Speed = FVector::DotProduct(Velocity, ExitDir);          // 접선 속도 성분
    const float NewSpeed = FMath::Max(0.f, Speed + ExitDecel * dt);      // ExitDecel < 0

    Velocity = ExitDir * NewSpeed;

    const FVector Delta = Velocity * dt;
    FHitResult Hit;
    OwnerChar->AddActorWorldOffset(Delta, true, &Hit);

    // 충분히 느려지면 종료
    if (NewSpeed <= KINDA_SMALL_NUMBER)
    {
        bExiting = false;
        Velocity = FVector::ZeroVector;
    }
}

auto UWallRideComponent::EndWallRide() -> void
{
    // 벽타기 종료: 상태만 끄고, 실제 이동모드는 TickWallExit에서 Falling으로
    bWallRiding = false;
}