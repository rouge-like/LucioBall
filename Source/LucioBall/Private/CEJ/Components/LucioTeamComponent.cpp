
#include "CEJ/Components/LucioTeamComponent.h"

ULucioTeamComponent::ULucioTeamComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULucioTeamComponent::ResolveTeamFromTags()
{
	AActor* Owner = GetOwner();
	Team = ETeamSide::Unknown;
	if (!Owner) return;

	const bool bIsFriendly =
		Owner->ActorHasTag(FriendlyTag) ||
		Owner->ActorHasTag(FName(TEXT("Player"))); // (옵션) 기존 맵 호환

	if (bIsFriendly) { Team = ETeamSide::Player; return; }
	if (Owner->ActorHasTag(EnemyTag)) { Team = ETeamSide::Enemy; return; }
}

int32 ULucioTeamComponent::GoalYSign() const
{
	switch (Team)
	{
	case ETeamSide::Player: return -1; // y<0
	case ETeamSide::Enemy:    return +1; // y>0
	default:                  return 0;
	}
}
