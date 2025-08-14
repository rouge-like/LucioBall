#include "CEJ/Ai/AiFindBall.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Animation/AnimInstance.h"

AAiFindBall::AAiFindBall()
{
    PrimaryActorTick.bCanEverTick = true;

    AutoPossessAI   = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AAIController::StaticClass();

    // 스켈메시 붙이기/위치/스케일 (필요 시 캡슐 사이즈도 조정)
    GetMesh()->SetupAttachment(GetCapsuleComponent());
    GetMesh()->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
    GetMesh()->SetRelativeLocation(FVector(0.f, -10.f, 60.f));
    GetMesh()->SetRelativeScale3D(FVector(47.f));

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshRef(
        TEXT("SkeletalMesh'/Game/CEJ/Animations/Skateboarding.Skateboarding'")
    );
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> OverlayMatRef(
        TEXT("Material'/Game/CEJ/Asset/lucio_default_color_tga_Mat.lucio_default_color_tga_Mat'")
    );

    if (MeshRef.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(MeshRef.Object);
        if (OverlayMatRef.Succeeded())
        {
            GetMesh()->SetOverlayMaterial(OverlayMatRef.Object);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("mesh material 경로 확인 필요"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("mesh 없음~ mesh 경로 확인!"));
    }
}

void AAiFindBall::BeginPlay()
{
    Super::BeginPlay();
    FindTargetByTagOnce();
}

void AAiFindBall::FindTargetByTagOnce()
{
    if (TargetActor.IsValid()) return;

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetTag, Found);

    if (Found.Num() > 0)
    {
        TargetActor = Found[0];
        UE_LOG(LogTemp, Log, TEXT("타깃 획득: %s (Tag: %s)"),
            *TargetActor->GetName(), *TargetTag.ToString());
    }
    else if (!bLoggedNotFound)
    {
        bLoggedNotFound = true;
        UE_LOG(LogTemp, Warning, TEXT("Tag '%s' 가진 액터(BP_BallTest) 없음"),
            *TargetTag.ToString());
    }
}

void AAiFindBall::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // 타깃 없으면 한 번 더 시도(레벨에서 늦게 스폰될 수 있으니)
    if (!TargetActor.IsValid())
    {
        FindTargetByTagOnce();
        return;
    }

    // 매 프레임 MoveToActor 호출 (요청사항)
    if (AAIController* AICon = Cast<AAIController>(GetController()))
    {
        AICon->MoveToActor(
            TargetActor.Get(),
            AcceptanceRadius,
            /*bStopOnOverlap*/ true,
            /*bUsePathfinding*/ true,
            /*bCanStrafe*/ true,
            /*FilterClass*/ nullptr,
            /*bAllowPartialPath*/ true
        );
    }
}
