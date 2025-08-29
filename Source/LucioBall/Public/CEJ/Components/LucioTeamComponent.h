#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LucioTeamComponent.generated.h"

UENUM(BlueprintType)
enum class ETeamSide : uint8
{
	Player,
	Enemy,
	Unknown
};

UCLASS(ClassGroup=(Lucio), meta=(BlueprintSpawnableComponent))
class LUCIOBALL_API ULucioTeamComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	ULucioTeamComponent();

	UPROPERTY(EditAnywhere, Category="Team|Tags")
	FName FriendlyTag = TEXT("Player");

	UPROPERTY(EditAnywhere, Category="Team|Tags")
	FName EnemyTag = TEXT("Enemy");

	UFUNCTION(BlueprintCallable, Category="Team")
	void ResolveTeamFromTags();

	UPROPERTY(VisibleAnywhere, Category="Team")
	ETeamSide Team = ETeamSide::Unknown;

	// 아군은 y<0
	// 적군은 y>0
	int32 GoalYSign() const;
};
