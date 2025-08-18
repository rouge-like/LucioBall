
#include "CEJ/Ai/AiLucioShooter.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "CEJ/Ai/TurretProjectile.h"

class ATurretProjectile;

AAiLucioShooter::AAiLucioShooter()
{
    PrimaryActorTick.bCanEverTick = true;

    Muzzle = CreateDefaultSubobject<UArrowComponent>(TEXT("Muzzle"));
    Muzzle->SetupAttachment(GetMesh());
    Muzzle->SetRelativeLocation(FVector(0,0,100));
}

void AAiLucioShooter::BeginPlay()
{
    Super::BeginPlay();

    // 0.25초마다 공 찾기 
    FindBall();
    GetWorldTimerManager().SetTimer(FindBallTimer, this, &AAiLucioShooter::FindBall, 0.25f, true, 0.25f);
}

void AAiLucioShooter::FindBall()
{
    if (Ball.IsValid()) return;

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("BouncyBall"), Found); // 태그 정확히

    if (Found.Num() > 0)
    {
        Ball = Found[0];
        GetWorldTimerManager().ClearTimer(FindBallTimer);      // 더 이상 찾지 않음
        StartFiring();                                         // 연사 시작
    }
}


void AAiLucioShooter::StartFiring()
{
    if (!ProjectileClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProjectileClass not set"));
        return;
    }

    // 루프 타이머: 1초마다 FireOnce 
    GetWorldTimerManager().SetTimer(
        FireTimer,
        this, &AAiLucioShooter::FireOnce,
        FireRate,
        /*bLoop*/true,
        /*FirstDelay*/0.f
    );
}

void AAiLucioShooter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!Ball.IsValid()) return;
    AimAtTarget(DeltaSeconds);
}

void AAiLucioShooter::AimAtTarget(float DeltaSeconds)
{
    const FVector S = Muzzle->GetComponentLocation();
    const FVector G = Ball->GetActorLocation();
    const FRotator Look = UKismetMathLibrary::FindLookAtRotation(S, G);
    const FRotator New  = FMath::RInterpTo(Muzzle->GetComponentRotation(), Look, DeltaSeconds, 12.f);
    Muzzle->SetWorldRotation(New);

    // 디버그
    DrawDebugDirectionalArrow(GetWorld(), S, S + (G - S).GetSafeNormal()*300.f, 40.f, FColor::Green, false, 0.f, 0, 2.f);
}

void AAiLucioShooter::FireOnce()
{
    if (!Ball.IsValid() || !ProjectileClass) return;

    const FVector MuzzleLoc = Muzzle->GetComponentLocation();
    const FVector AimDir    = (Ball->GetActorLocation() - MuzzleLoc).GetSafeNormal();
    const FRotator SpawnRot = AimDir.Rotation();

    // 스폰 오프셋: 총구 앞쪽으로 약간 빼서 겹침/자폭 방지
    const float SpawnOffset = 50.f; // 
    const FVector SpawnLoc  = MuzzleLoc + AimDir * SpawnOffset;

    FActorSpawnParameters P;
    P.Owner = this;
    P.Instigator = GetInstigator();
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    if (ATurretProjectile* Proj = GetWorld()->SpawnActor<ATurretProjectile>(ProjectileClass, SpawnLoc, SpawnRot, P))
    {
        Proj->InitLaunchDir(AimDir); // ← 발사 순간의 조준 벡터 주입 (매 1초마다 새로 계산)
    }

    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Yellow, TEXT("FireOnce()"));
}
