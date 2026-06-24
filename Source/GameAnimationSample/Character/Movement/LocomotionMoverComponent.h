// Copyright 2024 Locomotion System. All Rights Reserved.
// 移动组件 — CMC 封装 Locomotion mover component — CMC wrapper

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Core/CharacterEnums.h"
#include "LocomotionMoverComponent.generated.h"

class UCharacterStateComponent;
class ACharacter;
class UCharacterMovementComponent;

/**
 * 移动组件 Locomotion mover component
 *
 * 封装 UCharacterMovementComponent，暴露 RuntimeData 供 AnimInstance 直接读取。
 * 后续替换为 UMoverComponent + NetworkPrediction。
 */
UCLASS(ClassGroup=(Locomotion), meta=(BlueprintSpawnableComponent))
class ULocomotionMoverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULocomotionMoverComponent();

	// ── RuntimeData 运行时数据（AnimInstance 每帧 Poll）──

	/** 获取当前速度向量 Get current velocity vector */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	FVector GetCurrentVelocity() const;

	/** 获取当前速率 Get current speed (scalar) */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	float GetCurrentSpeed() const;

	/** 是否在地面上 Is on ground */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsOnGround() const;

	// ── Trajectory 轨迹预测（PoseSearch 查询输入）───────

	/** 轨迹点 Trajectory point */
	struct FTrajectoryPoint
	{
		FVector Position = FVector::ZeroVector;
		FQuat   Rotation = FQuat::Identity;
		float   TimeOffset = 0.f;
	};

	/** 获取预测轨迹 Get predicted trajectory */
	void GetPredictedTrajectory(TArray<FTrajectoryPoint>& OutTrajectory, float PredictionTime = 1.f, int32 SampleCount = 20) const;

	// ── Input 输入响应 ──────────────────────────────────

	/** 应用移动输入 Apply move input（由 InputComponent 直接调用 called by InputComponent） */
	void ApplyMoveInput(const FVector2D& MoveInput);

	// ── Gait Response 步态响应 ──────────────────────────

	/** 步态变化时调整速度 Adjust speed on gait change */
	void OnGaitChanged(EGait NewGait);

protected:
	virtual void BeginPlay() override;

private:
	// ── Speed Config 速度配置 ───────────────────────────

	UPROPERTY(EditAnywhere, Category = "Movement|Speed")
	float WalkSpeed = 200.f;

	UPROPERTY(EditAnywhere, Category = "Movement|Speed")
	float RunSpeed = 500.f;

	UPROPERTY(EditAnywhere, Category = "Movement|Speed")
	float SprintSpeed = 750.f;

	UPROPERTY(EditAnywhere, Category = "Movement|Speed")
	float CrouchSpeed = 150.f;

	// ── Cached References 缓存的引用 ────────────────────

	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> CMC;

	UPROPERTY()
	TObjectPtr<UCharacterStateComponent> StateComponent;
};
