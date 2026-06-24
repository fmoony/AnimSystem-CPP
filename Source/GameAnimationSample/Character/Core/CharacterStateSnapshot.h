// Copyright 2024 Locomotion System. All Rights Reserved.
// 持久状态快照 — 网络复制包 Persistent state snapshot — network replication packet

#pragma once

#include "CoreMinimal.h"
#include "Character/Core/CharacterEnums.h"
#include "CharacterStateSnapshot.generated.h"

/**
 * 持久状态快照 Persistent state snapshot
 *
 * 4 个 uint8 枚举 + 1 个 float = 8 bytes。
 * operator== 配合 COND_None 仅在变化时复制。
 * 服务端权威，客户端接收后广播 OnStateChanged。
 */
USTRUCT(BlueprintType)
struct FCharacterStateSnapshot
{
	GENERATED_BODY()

	UPROPERTY()
	EGait Gait = EGait::Run;

	UPROPERTY()
	EStance Stance = EStance::Stand;

	UPROPERTY()
	EMovementState MovementState = EMovementState::Idle;

	UPROPERTY()
	ERotationMode RotationMode = ERotationMode::VelocityDirection;

	UPROPERTY()
	float Speed = 0.f;

	bool operator==(const FCharacterStateSnapshot& Other) const
	{
		return Gait == Other.Gait
			&& Stance == Other.Stance
			&& MovementState == Other.MovementState
			&& RotationMode == Other.RotationMode
			&& FMath::IsNearlyEqual(Speed, Other.Speed);
	}

	bool operator!=(const FCharacterStateSnapshot& Other) const
	{
		return !(*this == Other);
	}
};
