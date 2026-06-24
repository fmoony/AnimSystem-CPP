// Copyright 2024 Locomotion System. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Core/CharacterEnums.h"
#include "LocomotionMoverComponent.generated.h"

class UCharacterStateComponent;
class ACharacter;
class UCharacterMovementComponent;

// ─────────────────────────────────────────────────────
// 移动组件 — 封装 UCharacterMovementComponent
//
// 职责：
//   - 暴露 RuntimeData 供 AnimInstance 直接读取（GetVelocity/GetSpeed/GetTrajectory）
//   - 订阅 OnStateChanged → 根据 Gait 调整最大速度/加速度
//   - ApplyInput() 由 InputComponent 直接调用
//
// 后续替换为 UMoverComponent + NetworkPrediction。
// ─────────────────────────────────────────────────────

UCLASS(ClassGroup=(Locomotion), meta=(BlueprintSpawnableComponent))
class ULocomotionMoverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULocomotionMoverComponent();

	// ── RuntimeData 直接读取（供 AnimInstance 每帧 Poll）──
	UFUNCTION(BlueprintCallable, Category = "Movement")
	FVector GetCurrentVelocity() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	float GetCurrentSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsOnGround() const;

	// ── 轨迹预测（PoseSearch 查询输入）──────────────────
	// 返回未来 PredictionTime 秒内的位置+朝向序列
	struct FTrajectoryPoint
	{
		FVector Position = FVector::ZeroVector;
		FQuat   Rotation = FQuat::Identity;
		float   TimeOffset = 0.f;
	};

	void GetPredictedTrajectory(TArray<FTrajectoryPoint>& OutTrajectory, float PredictionTime = 1.f, int32 SampleCount = 20) const;

	// ── 输入响应（由 InputComponent 直接调用）───────────
	void ApplyMoveInput(const FVector2D& MoveInput);

	// ── 步态速度配置（OnStateChanged 触发）──────────────
	void OnGaitChanged(EGait NewGait);

protected:
	virtual void BeginPlay() override;

private:
	// ── 速度配置 ──────────────────────────────────────
	UPROPERTY(EditAnywhere, Category = "Movement|Speed")
	float WalkSpeed = 200.f;

	UPROPERTY(EditAnywhere, Category = "Movement|Speed")
	float RunSpeed = 500.f;

	UPROPERTY(EditAnywhere, Category = "Movement|Speed")
	float SprintSpeed = 750.f;

	UPROPERTY(EditAnywhere, Category = "Movement|Speed")
	float CrouchSpeed = 150.f;

	// ── 缓存的引用 ────────────────────────────────────
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> CMC;

	UPROPERTY()
	TObjectPtr<UCharacterStateComponent> StateComponent;
};
