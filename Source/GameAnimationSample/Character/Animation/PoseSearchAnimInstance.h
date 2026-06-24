// Copyright 2024 Locomotion System. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Character/Core/CharacterEnums.h"
#include "Character/Core/CharacterStateSnapshot.h"
#include "PoseSearchAnimInstance.generated.h"

class UCharacterStateComponent;
class ULocomotionMoverComponent;
class UCharacterEventBus;

UENUM()
enum class EFootstepSide : uint8
{
	Left,
	Right
};

// ─────────────────────────────────────────────────────
// PoseSearch AnimInstance C++ 基类
//
// 数据流（每帧）：
//   NativeUpdateAnimation()
//     ├─ 直接读 MoverComponent  → Speed, Velocity, Direction, Trajectory
//     ├─ 直接读 StateComponent  → Gait, Stance, MovementState, RotationMode
//     └─ 写入 BlueprintReadOnly 属性 → AnimGraph PoseSearch/Warping 节点消费
//
// 事件响应（低频）：
//   OnStateChanged → 切换 PoseSearch Database
//
// 不负责：
//   - PoseSearch 节点逻辑（在 ABP 编辑器中配置）
//   - 最终动画输出（AnimGraph）
// ─────────────────────────────────────────────────────

UCLASS()
class UPoseSearchAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPoseSearchAnimInstance();

	// ── AnimGraph 输入（每帧由 NativeUpdateAnimation 填充）─
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

	// ── Trajectory（PoseSearch 查询核心输入）────────────
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Trajectory")
	TArray<FVector> FuturePositions;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Trajectory")
	TArray<FQuat> FutureRotations;

	// ── Warping ─────────────────────────────────────────
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Locomotion|Warping")
	float OrientationWarpingAlpha = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Locomotion|Warping")
	float StrideWarpingAlpha = 1.f;

	// ── Chooser 表引用（在 ABP 蓝图中配置）──────────────
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion|PoseSearch")
	TObjectPtr<UObject> DatabaseChooser; // UChooserTable* — forward declared as UObject for module decoupling

	// ── 脚步事件 ────────────────────────────────────────
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
