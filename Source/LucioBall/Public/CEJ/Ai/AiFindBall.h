#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AiFindBall.generated.h"

UCLASS()
class AAiFindBall : public ACharacter
{
	GENERATED_BODY()

public:
	AAiFindBall();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// 탐색 / 이동 보조
	void FindTargetByTagOnce();   // Ball
	void FindGoalByTagOnce();     // Goal
	void MoveLogicTowardGoal(float DeltaSeconds);

	// 볼 위치에서 골대 방향으로 트레이스
	bool DoGoalDirectionTrace(const FVector& Start, const FVector& Dir, FHitResult& OutHit);

	UPROPERTY(EditAnywhere, Category="FindBall|Target")
	FName TargetTag = TEXT("BP_BallTest");   // 볼 액터에 이 태그 부여

	UPROPERTY(EditAnywhere, Category="FindBall|Target")
	FName GoalTag = TEXT("BP_GoalPost");     // 골대 액터에 이 태그 부여

	UPROPERTY(EditAnywhere, Category="FindBall|Move")
	float AcceptanceRadius = 120.f;          // MoveTo 허용 반경

	UPROPERTY(EditAnywhere, Category="FindBall|Soccer")
	float BehindBallDistance = 220.f;        // 볼 뒤 어프로치 오프셋

	UPROPERTY(EditAnywhere, Category="FindBall|Soccer")
	float KickThroughDistance = 300.f;       // 볼을 관통해 골대 방향으로 나아갈 거리

	UPROPERTY(EditAnywhere, Category="FindBall|Soccer")
	float ApproachTolerance = 120.f;         // 어프로치 지점 도착 판정 반경

	// === 트레이스/디버그 ===
	UPROPERTY(EditAnywhere, Category="FindBall|Trace")
	float TraceLength = 3000.f;              // 볼에서 골대 방향으로 쏠 트레이스 길이

	UPROPERTY(EditAnywhere, Category="FindBall|Trace")
	bool bUseSphereTrace = false;            // 스피어 트레이스 사용 여부

	UPROPERTY(EditAnywhere, Category="FindBall|Trace", meta=(EditCondition="bUseSphereTrace"))
	float TraceRadius = 40.f;                // 스피어 반지름

	UPROPERTY(EditAnywhere, Category="FindBall|Debug")
	bool bDebugDraw = true;

private:
	// 찾은 타깃들
	TWeakObjectPtr<AActor> TargetActor; // Ball
	TWeakObjectPtr<AActor> GoalActor;   // Goal

	bool bLoggedNotFound = false;       // 볼 미발견 로그 중복 방지
};
