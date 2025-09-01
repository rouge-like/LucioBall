#pragma once

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AiLucio.generated.h"

class ULucioSkillComponent;
class ULucioTeamComponent;
class ULucioBallSensorComponent;
class ULucioMoveAssistComponent;
class ULucioCombatComponent;
class ABouncyBall;
class UCharacterMovementComponent;
class UTextRenderComponent;
class AAIController;

UENUM(BlueprintType)
enum class ELucioState : uint8
{
    Idle,
    SeekBall,
    AttackBall,
    ClearBall,
    DefendGoal
};

UCLASS()
class LUCIOBALL_API AAiLucio : public ACharacter
{
    GENERATED_BODY()

public:
    AAiLucio();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    // ===== Components =====
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lucio|Comp")
    ULucioSkillComponent*     SkillComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lucio|Comp")
    ULucioTeamComponent*      TeamComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lucio|Comp")
    ULucioBallSensorComponent* SenseComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lucio|Comp")
    ULucioMoveAssistComponent* MoveComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lucio|Comp")
    ULucioCombatComponent*    CombatComp;

    // ===== Config =====
    UPROPERTY(EditAnywhere, Category="Lucio|Config")
    float PossessionRadius = 100.f;

    UPROPERTY(EditAnywhere, Category="Lucio|UI")
    bool bShowRoleText = true;

    UPROPERTY(EditAnywhere, Category="Lucio|UI")
    float TextHeightOffset = 120.f;

    // ===== Runtime =====
    UPROPERTY(VisibleAnywhere, Category="Lucio|State")
    ELucioState State = ELucioState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimSequence* MoveAnim;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* JumpAnim;
    
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsPlayingMoveAnim = false;

protected:
    void UpdateStateMachine(float DeltaSeconds);
    void DoAttackLogic(float DeltaSeconds);
    void DoDefenseLogic(float DeltaSeconds);
    void UpdateRoleBillboard();

private:
    TWeakObjectPtr<AAIController> CachedAI;
    UPROPERTY() UTextRenderComponent* RoleText = nullptr;

   
};
