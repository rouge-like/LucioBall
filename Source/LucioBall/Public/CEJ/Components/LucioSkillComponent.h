#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LucioSkillComponent.generated.h"

UCLASS(ClassGroup=(Lucio), meta=(BlueprintSpawnableComponent))
class LUCIOBALL_API ULucioSkillComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	ULucioSkillComponent();

	// 설정값
	UPROPERTY(EditAnywhere, Category="Skill")
	float BaseSpeed = 800.f; // 기본 이동속도

	UPROPERTY(EditAnywhere, Category="Skill")
	float BoostMultiplier = 1.6f; // 스킬 사용시 속도 배수

	UPROPERTY(EditAnywhere, Category="Skill")
	float Cooldown = 6.f; // 스킬 쿨타임(sec)

	UPROPERTY(EditAnywhere, Category="Skill")
	float Duration = 3.f; // 스킬 지속시간(sec)

	// 상태값
	UPROPERTY(VisibleAnywhere, Category="Skill")
	bool bActive = false;

	UFUNCTION(BlueprintCallable, Category="Skill")
	bool CanUse() const;

	UFUNCTION(BlueprintCallable, Category="Skill")
	bool TryUse(); // 가능하면 즉시 발동(true 반환)

	UFUNCTION(BlueprintCallable, Category="Skill")
	void TickSkill(float NowSeconds);

	UFUNCTION(BlueprintCallable, Category="Skill")
	float ComputeSpeed() const; // 현재상태에서 목표 속도

	UFUNCTION(BlueprintCallable, Category="Skill")
	void ApplyTo(class UCharacterMovementComponent* Move) const;

private:
	float LastUseTime = -FLT_MAX;
	float EndTime = 0.f;
};
