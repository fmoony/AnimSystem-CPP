// Copyright 2024 Locomotion System. All Rights Reserved.
// 角色状态组件 — 服务端权威状态判定 Character state component — server-authoritative state determination

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Core/CharacterEnums.h"
#include "Character/Core/CharacterStateSnapshot.h"
#include "Character/Core/CharacterInputData.h"
#include "CharacterStateComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnStateChanged, const FCharacterStateSnapshot&);

/**
 * 角色状态组件 Character state component
 *
 * 职责：从 InputData + Velocity 判定持久状态；网络复制 StateSnapshot；
 *       变化时广播 OnStateChanged；暴露 RuntimeData 直接读取。
 * 不负责：移动计算（MoverComponent）、动画播放（AnimInstance）、瞬时事件（EventBus）。
 */
UCLASS(ClassGroup=(Locomotion), meta=(BlueprintSpawnableComponent))
class UCharacterStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterStateComponent();

	// ── Events 事件 ────────────────────────────────────

	/** 状态变化事件 State changed event */
	FOnStateChanged OnStateChanged;

	// ── Persistent State Getters 持久状态读取 ───────────

	EGait			  GetGait()			  const { return StateSnapshot.Gait; }
	EStance			  GetStance()		  const { return StateSnapshot.Stance; }
	EMovementState	  GetMovementState()  const { return StateSnapshot.MovementState; }
	ERotationMode	  GetRotationMode()	  const { return StateSnapshot.RotationMode; }
	float			  GetCurrentSpeed()	  const { return StateSnapshot.Speed; }

	// ── RuntimeData Getter 运行时数据读取（不复制）──────

	EMovementDirection GetCurrentDirection() const { return CurrentDirection; }

	// ── Authority 服务端权威 ────────────────────────────

	/** 服务端权威状态更新 Server-authoritative state update（每帧调用 called per frame） */
	void UpdateState(const FCharacterInputData& Input,
					 const FVector& Velocity,
					 bool bIsOnGround);

	/** 客户端本地预测 Client-side local prediction（不等服务端复制 before replication arrives） */
	void LocalPredict(const FCharacterInputData& Input,
					  const FVector& Velocity,
					  bool bIsOnGround);

	// ── Static Pure Functions 静态判定纯函数 ────────────

	static EGait			  DetermineGait(const FCharacterInputData& Input, float Speed, bool bIsOnGround);
	static EStance			  DetermineStance(bool bCrouchToggled, EStance Current);
	static EMovementState	  DetermineMovementState(float Speed, bool bIsOnGround, bool bIsTraversing);
	static EMovementDirection QuantizeDirection(const FVector& Velocity2D, const FRotator& ActorRotation);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(ReplicatedUsing = OnRep_StateSnapshot)
	FCharacterStateSnapshot StateSnapshot;

	UFUNCTION()
	void OnRep_StateSnapshot();

	// RuntimeData — 本地缓存不复制 Locally cached, not replicated
	EMovementDirection CurrentDirection = EMovementDirection::F;
	EStance			   PreviousStance = EStance::Stand;
};
