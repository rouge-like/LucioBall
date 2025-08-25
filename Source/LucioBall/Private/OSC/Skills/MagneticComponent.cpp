// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Skills/MagneticComponent.h"
#include "OSC/BouncyBall.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

// 이 컴포넌트는 주변의 공(BouncyBall)을 끌어당기는 자기장 효과를 구현합니다.
// 특정 범위 내에 있는 공을 소유자(Owner) 방향으로 끌어당기는 힘을 주기적으로 적용합니다.

// Sets default values for this component's properties
UMagneticComponent::UMagneticComponent()
{
	// 이 컴포넌트가 매 프레임 업데이트(Tick)되도록 설정합니다.
	// 자기장 효과를 지속적으로 적용하기 위해 필요합니다.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMagneticComponent::BeginPlay()
{
	Super::BeginPlay();

	// 게임이 시작되면 월드에 존재하는 공을 찾아서 BallActor 변수에 할당합니다.
	FindBall();
}


// Called every frame
void UMagneticComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 경과 시간을 누적합니다. 이는 쿨타임을 계산하는 데 사용됩니다.
	ElapsedTime += DeltaTime;
	
	// BallActor가 유효하지 않은 경우 (예: 아직 찾지 못했거나, 파괴된 경우) 다시 공을 찾습니다.
	if (!BallActor) FindBall();


	// 경과 시간이 쿨타임보다 크거나 같을 때 자기장 효과를 발동합니다.
	if (ElapsedTime >= CoolTime)
	{
		// 소유자(이 컴포넌트를 가진 액터)와 공의 위치를 가져옵니다.
		FVector OwnerLocation = GetOwner()->GetActorLocation();
		FVector BallLocation = BallActor->GetActorLocation();
		// 소유자와 공 사이의 거리를 계산합니다.
		float Distance = FVector::Dist(OwnerLocation, BallLocation);

		// 공이 설정된 최소(MinRange) 및 최대(MaxRange) 범위 내에 있을 때 bIsPulling을 true로 설정합니다.
		if (Distance > MinRange && Distance < MaxRange)
		{
			if (!bIsPulling)
			{
				bIsPulling = true;
				BallActor->BouncyBallSetVelocity(FVector::ZeroVector, OwnerActor);
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, "UseMagnetic");
			}
		}
	}

	// bIsPulling이 true일 때 공을 끌어당기는 로직을 실행합니다.
	if(bIsPulling)
	{
		FVector OwnerLocation = GetOwner()->GetActorLocation();
		FVector BallLocation = BallActor->GetActorLocation();
		
		// 목표 위치는 소유자 위치에서 공 방향으로 120.0f 떨어진 지점입니다.
		FVector DirectionToBall = (BallLocation - OwnerLocation).GetSafeNormal();
		FVector TargetLocation = OwnerLocation + DirectionToBall * 120.0f;

		// 현재 위치에서 목표 위치로 부드럽게 이동시킵니다.
		FVector NewBallLocation = FMath::VInterpTo(BallLocation, TargetLocation, DeltaTime, PullInterpSpeed);
		BallActor->SetActorLocation(NewBallLocation);

		// 목표 위치에 거의 도달하면 끌어당기는 것을 멈춥니다.
		if (FVector::Dist(NewBallLocation, TargetLocation) < 1.0f)
		{
			bIsPulling = false;
			ElapsedTime = 0; // 쿨타임 리셋
		}
	}
}

// 월드 내의 ABouncyBall 액터를 찾아 BallActor 멤버 변수에 할당하는 함수입니다.
void UMagneticComponent::FindBall()
{
	// 월드에 있는 모든 액터를 담을 배열을 선언합니다.
	TArray<AActor*> FoundBalls;
	// UGameplayStatics 함수를 사용하여 월드에서 ABouncyBall 클래스의 모든 액터를 찾아 FoundBalls 배열에 넣습니다.
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABouncyBall::StaticClass(), FoundBalls);

	// 만약 한 개 이상의 공을 찾았다면
	if (FoundBalls.Num() > 0)
	{
		// 첫 번째로 찾은 공을 ABouncyBall 타입으로 캐스팅하여 BallActor 변수에 저장합니다.
		BallActor = Cast<ABouncyBall>(FoundBalls[0]);
	}
}