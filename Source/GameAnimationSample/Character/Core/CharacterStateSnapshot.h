// Copyright 2024 Locomotion System. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/Core/CharacterEnums.h"
#include "CharacterStateSnapshot.generated.h"

// ─────────────────────────────────────────────────────
// 持久状态快照 — 网络复制包
//
// 设计要点：
//   - 4 个 uint8 枚举 + 1 个 float = 8 bytes
//   - operator== 用于仅在变化时复制（COND_None + 比较）
//   - 服务端权威，客户端接收后广播 OnStateChanged
// ─────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCharacterStateSnapshot
{
	GENERATED_BODY()

	// ── 持久状态枚举（4 bytes 总计）───────────────────
	UPROPERTY()
	EGait Gait = EGait::Run;

	UPROPERTY()
	EStance Stance = EStance::Stand;

	UPROPERTY()
	EMovementState MovementState = EMovementState::Idle;

	UPROPERTY()
	ERotationMode RotationMode = ERotationMode::VelocityDirection;

	// ── 标量 ─────────────────────────────────────────
	UPROPERTY()
	float Speed = 0.f;

	// ── 比较 ─────────────────────────────────────────
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
