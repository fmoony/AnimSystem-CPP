// Copyright 2024 Locomotion System. All Rights Reserved.
// 标准化输入数据 Standardized input data struct

#pragma once

#include "CoreMinimal.h"
#include "Character/Core/CharacterEnums.h"
#include "CharacterInputData.generated.h"

/**
 * 标准化输入 Standardized input
 *
 * EnhancedInput → InputComponent 每帧累积到此结构体。
 * 瞬时动作（Jump/Crouch/Traverse）在当前帧消费后由 ResetInstants() 清零。
 */
USTRUCT(BlueprintType)
struct FCharacterInputData
{
	GENERATED_BODY()

	// ── 模拟量持续输入 Analog continuous ────────────────

	UPROPERTY()
	FVector2D MoveInput = FVector2D::ZeroVector;

	UPROPERTY()
	FVector2D LookInput = FVector2D::ZeroVector;

	// ── 保持状态 Toggle holds ───────────────────────────

	UPROPERTY()
	bool bSprintHeld = false;

	UPROPERTY()
	bool bWalkHeld = false;

	UPROPERTY()
	bool bStrafeHeld = false;

	// ── 瞬时动作 Instant actions（消费后清零）───────────

	UPROPERTY()
	bool bJumpPressed = false;

	UPROPERTY()
	bool bCrouchToggled = false;

	UPROPERTY()
	bool bTraversePressed = false;

	/** 清除瞬时输入 Clear instant inputs（每帧末调用） */
	void ResetInstants()
	{
		bJumpPressed = false;
		bCrouchToggled = false;
		bTraversePressed = false;
	}

	/** 是否有移动输入 Has movement input */
	bool HasMovementInput() const
	{
		return !MoveInput.IsNearlyZero();
	}
};
