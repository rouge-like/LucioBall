
#include "CEJ/Components/LucioSkillComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ULucioSkillComponent::ULucioSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool ULucioSkillComponent::CanUse() const
{
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	return !bActive && (Now - LastUseTime) >= Cooldown;
}

bool ULucioSkillComponent::TryUse()
{
	if (!CanUse()) return false;
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	bActive = true;
	LastUseTime = Now;
	EndTime = Now + Duration;
	return true;
}

void ULucioSkillComponent::TickSkill(float Now)
{
	if (bActive && Now >= EndTime)
	{
		bActive = false;
	}
}

float ULucioSkillComponent::ComputeSpeed() const
{
	return BaseSpeed * (bActive ? BoostMultiplier : 1.f);
}

void ULucioSkillComponent::ApplyTo(UCharacterMovementComponent* Move) const
{
	if (!Move) return;
	Move->MaxWalkSpeed = ComputeSpeed();
	Move->MaxAcceleration = 5000.f;
	Move->BrakingDecelerationWalking = 2000.f;
	Move->GroundFriction = 8.f;
}
