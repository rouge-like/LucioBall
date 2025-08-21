// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AiWallRunning.generated.h"

class USphereComponent; 

UCLASS()
class LUCIOBALL_API AAiWallRunning : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAiWallRunning();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//Transform orientation
	// 기준이 될 SceneComponent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="References")
	USceneComponent* Orientation = nullptr;

	// Collision Channel (디폴트는 ECC_Visibility, 필요시 커스텀 채널 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallCheck")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	// cm 단위이므로 보통 100 = 1m
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallCheck")
	float WallCheckDistance = 1000.f;

	//wallRight 
	UPROPERTY(BlueprintReadOnly, Category="WallCheck")
	bool bWallRight = false;
	//wallLeft
	UPROPERTY(BlueprintReadOnly, Category="WallCheck")
	bool bWallLeft = false;

	//out RaycastHit rightWallhit
	UPROPERTY(BlueprintReadOnly, Category="WallCheck")
	FHitResult RightWallHit;
	//out RaycastHit rightWallhit
	UPROPERTY(BlueprintReadOnly, Category="WallCheck")
	FHitResult LeftWallHit;

	//private void CheckForWall()
	UFUNCTION(BlueprintCallable, Category="WallCheck")
	void CheckForWall();

	//보통 캐릭터와 캐릭터 이동 컴포넌트 캐싱(Caching: 처음에 한 번만 찾아서 변수에 저장해 둠)
	ACharacter* OwnerCharacter = nullptr;
	class UCharacterMovementComponent* MoveComp = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun")
	float WallRunForce = 500.f; // 원하는 힘 값 (N 단위)

	//이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float WalkSpeed = 600.f;   // Unity Walk Speed 7 → 언리얼 기본 단위 cm/s라 보통 ×100

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float SprintSpeed = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float SlideSpeed = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun")
	float WallRunSpeed = 850.f;  // Unity 8.5 ×100

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float SpeedIncreaseMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float SlopeIncreaseMultiplier = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float GroundDrag = 4.f;

	//점프
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jumping")
	float JumpForce = 1200.f;   // Unity 12 ×100

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jumping")
	float JumpCooldown = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jumping")
	float AirMultiplier = 0.4f;

	// 벽 달리기 메서드
	UFUNCTION(BlueprintCallable, Category="WallRun")
	void StartWallRun();   // StartWallRun()

	UFUNCTION(BlueprintCallable, Category="WallRun")
	void WallRunningMovement(); // WallRunningMovement()

	UFUNCTION(BlueprintCallable, Category="WallRun")
	void StopWallRun();    // StopWallRun()
	
	UPROPERTY(BlueprintReadOnly, Category="WallRun")
	bool bIsWallRunning = false;

	// 입력값 저장용 (horizontalInput, verticalInput)
	UPROPERTY(BlueprintReadOnly, Category="Input")
	float HorizontalInput = 0.f;   // A/D (또는 좌/우)
	UPROPERTY(BlueprintReadOnly, Category="Input")
	float VerticalInput = 0.f;     // W/S (전/후)

	// StateMachine() 대응 
	UFUNCTION(BlueprintCallable, Category="WallRun")
	void StateMachine();

	// AboveGround() 대응 (바닥과의 거리 체크)
	UFUNCTION(BlueprintCallable, Category="WallRun")
	bool AboveGround() const;

	UFUNCTION(BlueprintCallable, Category="WallRun")
	void FixedUpdate();
	bool TraceForWallWithTag(const FVector& Start, const FVector& Dir, float Distance, FHitResult& OutHit) const;

	// 벽에 붙도록 밀어주는 힘 (Unity: rb.AddForce(-wallNormal * 100))
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun|Tuning")
	float BasePushToWallForce = 120000.f;

	// 입력으로 벽에서 떨어지는 힘을 줄이는 계수 (0=무시, 1=완전취소)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun|Tuning")
	float PushCancelStrength = 1.0f;

	// (선택) 스피드에 따라 힘을 곱해줄 커브
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun|Tuning")
	UCurveFloat* PushToWallBySpeed = nullptr;

	// 벽으로 인정할 태그 이름 (에디터에서 바꿀 수 있게)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun|Detection")
	FName WallTag = TEXT("Wall");

	// 벽 법선이 '수직에 가까운지' 확인하는 문턱 (0~1, 0에 가까울수록 수직)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun|Detection", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MaxUpDotForWall = 0.5f; // 0.5≈60도 이상 기울기만 벽으로

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun|AI")
	bool bAutoWallRun = true;

	UPROPERTY(EditAnywhere, Category="WallRun|Rotation")
	float WallYawInterpSpeed = 12.f;

	bool bPrevUseControllerRotationYaw = false;
	bool bPrevOrientRotationToMovement = false;
	FRotator PrevRotationRate = FRotator::ZeroRotator;

	// 점프 후 N초 안에만 벽타기 허용
	UPROPERTY(EditAnywhere, Category="WallRun|Start")
	bool bRequireJumpToStart = true;

	UPROPERTY(EditAnywhere, Category="WallRun|Start", meta=(ClampMin="0.1", ClampMax="2.0"))
	float RecentJumpWindow = 0.7f;

	// JumpPoint 자동 점프 세기(LaunchCharacter용)
	UPROPERTY(EditAnywhere, Category="WallRun|Start")
	float AutoJumpZ = 650.f;

	// JumpPoint 인식 방법: Tag 사용
	UPROPERTY(EditAnywhere, Category="WallRun|Start")
	FName JumpPointTag = TEXT("JumpPoint");

	// 최근 점프 여부 타임윈도우
	bool bHasRecentJump = false;
	FTimerHandle RecentJumpResetHandle;

	// BeginPlay에서 JumpPoint 감지되면 자동 점프
	void TryAutoJumpFromJumpPoint();
	void MarkRecentJump();
	void ClearRecentJump();

	virtual void OnJumped_Implementation() override; // 점프키/Jump()일 때 콜백
	virtual void Landed(const FHitResult& Hit) override;

	bool IsOnJumpPoint() const;

	/** 점프 트리거용 큰 센서(메시보다 큼) */
	UPROPERTY(VisibleAnywhere, Category="WallRun|JumpPoint")
	USphereComponent* JumpSensor = nullptr;

	/** 센서 반지름(메시보다 크게) */
	UPROPERTY(EditAnywhere, Category="WallRun|JumpPoint", meta=(ClampMin="30.0"))
	float JumpSensorRadius = 150.f;

	/** 센서로 '태그'로 판정할지 */
	UPROPERTY(EditAnywhere, Category="WallRun|JumpPoint")
	bool bUseJumpPointTag = true;

	/** 센서로 '클래스'로도 판정하고 싶을 때 (BP_JumpPoint 할당) */
	UPROPERTY(EditAnywhere, Category="WallRun|JumpPoint")
	TSubclassOf<AActor> JumpPointClass;

	/** 센서가 겹치면 자동 점프할지 */
	UPROPERTY(EditAnywhere, Category="WallRun|JumpPoint")
	bool bAutoJumpFromSensor = true;

	UFUNCTION()
	void OnJumpSensorBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
								  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
								  bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnJumpSensorEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
								UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);



};
