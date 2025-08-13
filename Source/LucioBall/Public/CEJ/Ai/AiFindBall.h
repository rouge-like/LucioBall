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

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	// 매 프레임 따라갈 타깃 (BP_BallTest)
	TWeakObjectPtr<AActor> TargetActor;

public:
	// BP_BallTest(Actor)의 Tags 에 넣은 값과 정확히 일치해야 함
	UPROPERTY(EditAnywhere, Category="AI")
	FName TargetTag = TEXT("BouncyBall");

	UPROPERTY(EditAnywhere, Category="AI")
	float AcceptanceRadius = 120.f;

	bool bLoggedNotFound = false;

private:
	void FindTargetByTagOnce();
};
