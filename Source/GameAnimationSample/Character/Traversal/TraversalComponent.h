// Copyright 2024 Locomotion System. All Rights Reserved.
// 越障组件 — Trace 检测 + MotionWarping Traversal component — trace detection + motion warping

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Traversal/TraversalTypes.h"
#include "TraversalComponent.generated.h"

class UCharacterEventBus;
class UCharacterStateComponent;
class ACharacter;

/**
 * 越障组件 Traversal component
 *
 * 职责：收到 Traverse 输入 → SphereTrace 检测障碍物 → 判定动作类型 →
 *       MotionWarping 目标设置 → 发布 FTraversalEvent。
 * 依赖：EventBus（发布事件）、StateComponent（读取速度/朝向）。
 */
UCLASS(ClassGroup=(Locomotion), meta=(BlueprintSpawnableComponent))
class UTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTraversalComponent();

	// ── Attempt 尝试越障 ───────────────────────────────

	/** 尝试发起越障 Attempt to start traversal（由 InputComponent 调用 called by InputComponent） */
	void TryTraversal();

	/** 越障完成回调 Traversal completed callback（由 AnimNotify 调用 called by AnimNotify） */
	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void CompleteTraversal(bool bSuccess);

	// ── Query 查询 ─────────────────────────────────────

	/** 是否正在越障中 Is currently traversing */
	bool IsTraversing() const { return bIsTraversing; }

	/** 获取最后一次检测结果 Get last check result */
	const FTraversalCheckResult& GetLastResult() const { return LastResult; }

protected:
	virtual void BeginPlay() override;

private:
	// ── Detection 检测 ─────────────────────────────────

	/** 球形扫描检测障碍物 Sphere trace to detect obstacle */
	bool DetectObstacle(FTraversalCheckResult& OutResult) const;

	/** 从障碍物几何判定动作类型 Classify action type from obstacle geometry */
	static ETraversalActionType ClassifyAction(float ObstacleHeight, float ObstacleDepth);

	// ── Config 配置 ────────────────────────────────────

	/** 检测距离 Detection distance */
	UPROPERTY(EditAnywhere, Category = "Traversal|Detection")
	float DetectionDistance = 100.f;

	/** 检测半径 Detection radius */
	UPROPERTY(EditAnywhere, Category = "Traversal|Detection")
	float DetectionRadius = 30.f;

	/** 检测高度偏移 Detection height offset（从角色脚底算 From character feet） */
	UPROPERTY(EditAnywhere, Category = "Traversal|Detection")
	float DetectionHeightOffset = 50.f;

	/** 最大可跨越高度 Max vaultable height */
	UPROPERTY(EditAnywhere, Category = "Traversal|Detection")
	float MaxVaultHeight = 120.f;

	/** 最大可攀爬高度 Max climbable height */
	UPROPERTY(EditAnywhere, Category = "Traversal|Detection")
	float MaxClimbHeight = 200.f;

	// ── State 状态 ─────────────────────────────────────

	bool bIsTraversing = false;
	FTraversalCheckResult LastResult;

	// ── Cached References 缓存引用 ──────────────────────

	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UCharacterStateComponent> StateComponent;

	UPROPERTY()
	TObjectPtr<UCharacterEventBus> EventBus;
};
