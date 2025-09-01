

#include "CEJ/Ai/AiLucio.h"
#include "AIController.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CEJ/Components/LucioSkillComponent.h"
#include "CEJ/Components/LucioTeamComponent.h"
#include "CEJ/Components/LucioBallSensorComponent.h"
#include "CEJ/Components/LucioMoveAssistComponent.h"
#include "CEJ/Components/LucioCombatComponent.h"
#include "OSC/BouncyBall.h"

AAiLucio::AAiLucio()
{
	PrimaryActorTick.bCanEverTick = true;

	SkillComp = CreateDefaultSubobject<ULucioSkillComponent>(TEXT("SkillComp"));
	TeamComp  = CreateDefaultSubobject<ULucioTeamComponent>(TEXT("TeamComp"));
	SenseComp = CreateDefaultSubobject<ULucioBallSensorComponent>(TEXT("SenseComp"));
	MoveComp  = CreateDefaultSubobject<ULucioMoveAssistComponent>(TEXT("MoveComp"));
	CombatComp= CreateDefaultSubobject<ULucioCombatComponent>(TEXT("CombatComp"));

	// 캐릭터 이동 기본치(스킬에서 최종 반영)
	if (UCharacterMovementComponent* M = GetCharacterMovement())
	{
		M->MaxWalkSpeed = SkillComp->BaseSpeed; // 800
		M->JumpZVelocity = 700.f;
	}
	// 간단한 역할 텍스트
	RoleText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("RoleText"));
	if (RoleText)
	{
		RoleText->SetupAttachment(RootComponent);
		RoleText->SetHorizontalAlignment(EHTA_Center);
		RoleText->SetVerticalAlignment(EVRTA_TextCenter);
		RoleText->SetWorldSize(30.f);
	}

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp)
	{
		MeshComp->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
		MeshComp->SetRelativeLocation(FVector(0.f, -10.f, 60.f));
		MeshComp->SetRelativeScale3D(FVector(47.f));

		static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshRef(
			TEXT("SkeletalMesh'/Game/CEJ/Animations/Stay.Stay'")
		);

		if (MeshRef.Succeeded())
		{
			MeshComp->SetSkeletalMesh(MeshRef.Object);
		}
	}

	// 이동 애니메이션 로드
	static ConstructorHelpers::FObjectFinder<UAnimSequence> MoveAnimRef(
		TEXT("AnimSequence'/Game/CEJ/Animations/Skateboarding_Anim.Skateboarding_Anim'")
	);
	if (MoveAnimRef.Succeeded())
	{
		MoveAnim = MoveAnimRef.Object;
	}
 
	// 점프 애니메이션 로드
	static ConstructorHelpers::FObjectFinder<UAnimSequence> JumpAnimRef(
		TEXT("AnimSequence'/Game/CEJ/Animations/Jumping_Anim.Jumping_Anim'")
	);
	if (JumpAnimRef.Succeeded())
	{
		JumpAnim = JumpAnimRef.Object;
	}
		
}

void AAiLucio::BeginPlay()
{
	Super::BeginPlay();
	
	CachedAI = Cast<AAIController>(GetController());

	TeamComp->ResolveTeamFromTags();                                  // 맵 태그로 팀 결정
	SenseComp->EnsureTargets(TeamComp->GoalYSign());                  // 공/골대 캐시
	if (RoleText) RoleText->SetText(FText::FromString(TEXT("DYNAMIC")));
}

void AAiLucio::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!SenseComp || !SkillComp || !MoveComp) return;

	// 타임기반 스킬 상태 갱신
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	SkillComp->TickSkill(Now);

	// 타깃/골대 보정 
	SenseComp->EnsureTargets(TeamComp->GoalYSign());

	const FVector MyLoc   = GetActorLocation();
	const FVector LandLoc = SenseComp->GetBallLandLocation();

	// 추격 최적화 + 속도 적용
	MoveComp->OptimizeChaseAndSpeed(this, GetCharacterMovement(), SkillComp, MyLoc, LandLoc, TeamComp->GoalYSign());

	// 간단 FSM
	UpdateStateMachine(DeltaSeconds);

	// 역할 텍스트 업데이트
	UpdateRoleBillboard();
}

void AAiLucio::UpdateStateMachine(float DeltaSeconds)
{
	if (!SenseComp) { State = ELucioState::Idle; return; }

	const float LandY = SenseComp->GetBallLandLocation().Y;
	const int32 GoalSign = TeamComp->GoalYSign();

	// 같은 쪽이면 공격, 반대쪽이면 수비
	const bool bAttackSide = (GoalSign < 0) ? (LandY < 0.f) : (LandY > 0.f);

	switch (State)
	{
	case ELucioState::Idle:
		State = ELucioState::SeekBall;
		break;

	case ELucioState::SeekBall:
	{
		// 공 착지 지점으로 이동
		if (CachedAI.IsValid())
			MoveComp->MoveToWithMaxSpeed(CachedAI.Get(), this, SenseComp->GetBallLandLocation());

		// 가까워지면 모드 전환
		if (SenseComp->IsBallNearby(PossessionRadius))
			State = bAttackSide ? ELucioState::AttackBall : ELucioState::ClearBall;
		break;
	}

	case ELucioState::AttackBall:
		DoAttackLogic(DeltaSeconds);
		break;

	case ELucioState::ClearBall:
		DoDefenseLogic(DeltaSeconds);
		break;

	case ELucioState::DefendGoal:
	default:
		// 필요 시 골대 앞 대기 등으로 확장 가능
		State = ELucioState::SeekBall;
		break;
	}
}

void AAiLucio::DoAttackLogic(float DeltaSeconds)
{
	if (!SenseComp || !CombatComp) { State = ELucioState::Idle; return; }

	const bool bClose = SenseComp->IsBallNearby(PossessionRadius);
	if (!bClose) { State = ELucioState::SeekBall; return; }

	CombatComp->KickTowardsGoal(SenseComp->Ball.Get(), SenseComp->Goal.Get(), this);
	State = ELucioState::SeekBall;
}

void AAiLucio::DoDefenseLogic(float DeltaSeconds)
{
	if (!SenseComp || !CombatComp) { State = ELucioState::Idle; return; }

	const bool bClose = SenseComp->IsBallNearby(PossessionRadius);
	if (bClose)
	{
		CombatComp->ClearUpAndOut(SenseComp->Ball.Get(), SenseComp->Goal.Get(), this);
		State = ELucioState::SeekBall;
	}
	else
	{
		// 수비일 때도 착지 지점으로 이동해 선점
		if (CachedAI.IsValid())
			MoveComp->MoveToWithMaxSpeed(CachedAI.Get(), this, SenseComp->GetBallLandLocation());
	}
}

void AAiLucio::UpdateRoleBillboard()
{
	if (!bShowRoleText || !RoleText) return;

	FString Text;
	switch (TeamComp->Team)
	{
	case ETeamSide::Player:	  Text = TEXT("PLAYER (y<0)"); break;
	case ETeamSide::Enemy:    Text = TEXT("ENEMY (y>0)");    break;
	default:                  Text = TEXT("UNKNOWN");        break;
	}
	RoleText->SetText(FText::FromString(Text));
	RoleText->SetRelativeLocation(FVector(0,0,TextHeightOffset));
}
