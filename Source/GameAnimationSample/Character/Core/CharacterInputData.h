// Copyright 2024 Locomotion System. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/Core/CharacterEnums.h"
#include "CharacterInputData.generated.h"

// ─────────────────────────────────────────────────────
// 标准化输入 — EnhancedInput → InputComponent → StateComponent
//
// 所有布尔输入使用 "Pressed/Released/Toggled" 语义，
// 由 InputComponent 每帧消费后清零瞬时值。
// ─────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCharacterInputData
{
	GENERATED_BODY()

	// ── 模拟量（持续）─────────────────────────────────
	UPROPERTY()
	FVector2D MoveInput = FVector2D::ZeroVector;	// WASD / 左摇杆

	UPROPERTY()
	FVector2D LookInput = FVector2D::ZeroVector;	// 鼠标 / 右摇杆

	// ── Toggle 状态（保持）───────────────────────────
	UPROPERTY()
	bool bSprintHeld = false;

	UPROPERTY()
	bool bWalkHeld = false;

	UPROPERTY()
	bool bStrafeHeld = false;

	// ── 瞬时动作（当前帧消费，下帧清零）───────────────
	UPROPERTY()
	bool bJumpPressed = false;

	UPROPERTY()
	bool bCrouchToggled = false;

	UPROPERTY()
	bool bTraversePressed = false;

	// ── 工具方法 ─────────────────────────────────────
	void ResetInstants()
	{
		bJumpPressed = false;
		bCrouchToggled = false;
		bTraversePressed = false;
	}

	bool HasMovementInput() const
	{
		return !MoveInput.IsNearlyZero();
	}

	// ── 从速度向量量化 12 方向（纯函数）───────────────
	static EMovementDirection QuantizeDirection(const FVector& Velocity2D, const FRotator& ActorRotation);
};
