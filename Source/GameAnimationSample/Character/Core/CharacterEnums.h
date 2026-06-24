// Copyright 2024 Locomotion System. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CharacterEnums.generated.h"

// ─────────────────────────────────────────────────────
// Persistent State Enums（持久状态，网络复制，变化时发事件）
// ─────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EGait : uint8
{
	Walk,
	Run,
	Sprint
};

UENUM(BlueprintType)
enum class EStance : uint8
{
	Stand,
	Crouch
};

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Idle,
	Locomotion,
	InAir,
	Traversal,
	Ragdoll
};

UENUM(BlueprintType)
enum class ERotationMode : uint8
{
	VelocityDirection,	// 角色朝向 = 移动方向
	LookingDirection	// 角色朝向 = 视角方向（横移/瞄准）
};

// ─────────────────────────────────────────────────────
// Runtime Quantization（本地计算，不复制，直接读取）
// ─────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EMovementDirection : uint8
{
	F,		// Forward
	FL,		// Forward-Left
	FR,		// Forward-Right
	B,		// Backward
	BL,		// Backward-Left
	BR,		// Backward-Right
	L,		// Left
	R,		// Right
	LL,		// Lateral-Left (pure strafe)
	LR,		// Lateral-Right-Forward
	RL,		// Right-Left-Forward
	RR		// Lateral-Right (pure strafe)
};

// ─────────────────────────────────────────────────────
// Traversal（越障）
// ─────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ETraversalActionType : uint8
{
	Vault,
	Mantle,
	Hurdle,
	Climb,
	Catch
};
