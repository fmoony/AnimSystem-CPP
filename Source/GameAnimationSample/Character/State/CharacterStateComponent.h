// Copyright 2024 Locomotion System. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Core/CharacterEnums.h"
#include "Character/Core/CharacterStateSnapshot.h"
#include "Character/Core/CharacterInputData.h"
#include "CharacterStateComponent.generated.h"

// ─────────────────────────────────────────────────────
// 角色状态组件 — 服务端权威状态判定
//
// 职责：
//   - 从 InputData + Velocity 判定持久状态（Gait/Stance/MovementState/RotationMode）
//   - 网络复制 StateSnapshot（仅在变化时发送）
//   - 变化时发布 OnStateChanged
//   - 暴露 RuntimeData 直接读取（CurrentDirection）
//
// 不负责：
//   - 移动计算（MoverComponent）
//   - 动画播放（AnimInstance）
//   - 瞬时事件（EventBus）
// ─────────────────────────────────────────────────────

DECLARE_MULTICAST_DELEGATE_OneParam(FOnStateChanged, const FCharacterStateSnapshot&);

UCLASS(ClassGroup=(Locomotion), meta=(BlueprintSpawnableComponent))
class UCharacterStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterStateComponent();

	// ── 状态变化事件 ──────────────────────────────────
	FOnStateChanged OnStateChanged;

	// ── Persistent State 直接读取 ─────────────────────
	EGait			  GetGait()			  const { return StateSnapshot.Gait; }
	EStance			  GetStance()		  const { return StateSnapshot.Stance; }
	EMovementState	  GetMovementState()  const { return StateSnapshot.MovementState; }
	ERotationMode	  GetRotationMode()	  const { return StateSnapshot.RotationMode; }
	float			  GetCurrentSpeed()	  const { return StateSnapshot.Speed; }

	// ── RuntimeData 直接读取（不复制，本地计算）────────
	EMovementDirection GetCurrentDirection() const { return CurrentDirection; }

	// ── 服务端权威更新（每帧调用）─────────────────────
	void UpdateState(const FCharacterInputData& Input,
					 const FVector& Velocity,
					 bool bIsOnGround);

	// ── 客户端本地预测（不等服务端复制）───────────────
	void LocalPredict(const FCharacterInputData& Input,
					  const FVector& Velocity,
					  bool bIsOnGround);

	// ── 静态判定函数（纯函数，方便单元测试）───────────
	static EGait			  DetermineGait(const FCharacterInputData& Input, float Speed, bool bIsOnGround);
	static EStance			  DetermineStance(bool bCrouchToggled, EStance Current);
	static EMovementState	  DetermineMovementState(float Speed, bool bIsOnGround, bool bIsTraversing);
	static EMovementDirection QuantizeDirection(const FVector& Velocity2D, const FRotator& ActorRotation);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// ── 持久状态快照（网络复制）───────────────────────
	UPROPERTY(ReplicatedUsing = OnRep_StateSnapshot)
	FCharacterStateSnapshot StateSnapshot;

	UFUNCTION()
	void OnRep_StateSnapshot();

	// ── RuntimeData（本地缓存）────────────────────────
	EMovementDirection CurrentDirection = EMovementDirection::F;
	EStance			   PreviousStance = EStance::Stand;
};
