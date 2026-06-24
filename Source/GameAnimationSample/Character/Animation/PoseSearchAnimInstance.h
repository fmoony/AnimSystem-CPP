// Copyright 2024 Locomotion System. All Rights Reserved.
// PoseSearch 动画实例 PoseSearch animation instance — C++ ABP base class

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Character/Core/CharacterEnums.h"
#include "Character/Core/CharacterStateSnapshot.h"
#include "PoseSearchAnimInstance.generated.h"

class UCharacterStateComponent;
class ULocomotionMoverComponent;
class UCharacterEventBus;

/**
 * PoseSearch 动画实例基类 PoseSearch animation instance base
 *
 * 每帧 NativeUpdateAnimation 直接读取 MoverComp（Speed/Velocity/Trajectory）
 * 和 StateComp（Gait/Stance/Direction）。订阅 OnStateChanged 用于切换 DB。
 */
UCLASS()
class UPoseSearchAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPoseSearchAnimInstance();

	// ── AnimGraph Inputs 动画图输入（每帧填充）──────────

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|State")
	FCharacterStateSnapshot CurrentState;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Runtime")
	float CurrentSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Runtime")
	FVector CurrentVelocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Runtime")
	EMovementDirection CurrentDirection = EMovementDirection::F;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Runtime")
	bool bIsOnGround = true;

	// ── Trajectory 轨迹（PoseSearch 查询核心输入）───────

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Trajectory")
	TArray<FVector> FuturePositions;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Trajectory")
	TArray<FQuat> FutureRotations;

	// ── Warping 扭曲修正 ────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Locomotion|Warping")
	float OrientationWarpingAlpha = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Locomotion|Warping")
	float StrideWarpingAlpha = 1.f;

	// ── Chooser 选择器表 ────────────────────────────────

	/** PoseSearch 数据库选择器表 Database chooser table */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion|PoseSearch")
	TObjectPtr<UObject> DatabaseChooser;

	// ── Foley 音效 ──────────────────────────────────────

	/** 通知脚步事件 Notify footstep event（由 AnimNotify 调用 called by anim notify） */
	UFUNCTION(BlueprintCallable, Category = "Locomotion|Foley")
	void NotifyFootstep(EFootstepSide Side);

	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	void CacheComponents();
	void UpdateTrajectory(float DeltaSeconds);

	UPROPERTY()
	TObjectPtr<UCharacterStateComponent> StateComponent;

	UPROPERTY()
	TObjectPtr<ULocomotionMoverComponent> MoverComponent;

	UPROPERTY()
	TObjectPtr<UCharacterEventBus> EventBus;

	FDelegateHandle StateChangedHandle;
};
