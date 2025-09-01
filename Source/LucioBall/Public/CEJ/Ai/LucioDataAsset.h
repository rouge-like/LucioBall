// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EditorFramework/AssetImportData.h"
#include "LucioDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class LUCIOBALL_API ULucioDataAsset : public UAssetImportData
{
	GENERATED_BODY()
public:	
	 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(ClampMin="0", UIMin="0"))
    float BaseMovementSpeed = 960.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(ClampMin="0", UIMin="0"))
    float BaseJumpPower = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(ClampMin="0", UIMin="0"))
    float WallRunSpeedMultiplier = 1.3f;

    // Skills
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skills", meta=(ClampMin="0", UIMin="0"))
    float ESkillSpeedMultiplier = 1.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skills", meta=(ClampMin="0", UIMin="0"))
    float UltimateSpeedMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skills", meta=(ClampMin="0", UIMin="0"))
    float UltimateJumpMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skills", meta=(ClampMin="0"))
    float ESkillCooldown = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skills", meta=(ClampMin="0"))
    float ESkillDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skills", meta=(ClampMin="0"))
    float UltimateCooldown = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skills", meta=(ClampMin="0"))
    float UltimateDuration = 8.0f;

    // Attack
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attack", meta=(ClampMin="0"))
    float BallKickImpulse = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attack", meta=(ClampMin="0"))
    float AttackDistance = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attack", meta=(ClampMin="0"))
    float PossessionRadius = 200.0f;

    // AI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI", meta=(ClampMin="0"))
    float AcceptanceRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI", meta=(ClampMin="0"))
    float EmergencyDistanceThreshold = 300.0f;

    // Tags
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Tags")
    FName AttackerTag = FName("Attacker");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Tags")
    FName DefenderTag = FName("Defender");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Tags")
    FName BallTag = FName("BouncyBall");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lucio|Tags")
    FName GoalTag = FName("SoccerGoal");
};
