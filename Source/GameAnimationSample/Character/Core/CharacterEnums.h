// Copyright 2024 Locomotion System. All Rights Reserved.
// 角色动画枚举定义 Character animation enum definitions

#pragma once

#include "CoreMinimal.h"
#include "CharacterEnums.generated.h"

// ── Persistent State 持久状态（网络复制，变化时发事件）──

/** 步态 Walk / Run / Sprint */
UENUM(BlueprintType)
enum class EGait : uint8
{
	Walk,
	Run,
	Sprint
};

/** 站姿 Stand / Crouch */
UENUM(BlueprintType)
enum class EStance : uint8
{
	Stand,
	Crouch
};

/** 宏观运动状态 Idle / Locomotion / InAir / Traversal / Ragdoll */
UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Idle,
	Locomotion,
	InAir,
	Traversal,
	Ragdoll
};

/** 旋转参考 移动方向 VelocityDirection / 视角方向 LookingDirection */
UENUM(BlueprintType)
enum class ERotationMode : uint8
{
	VelocityDirection,
	LookingDirection
};

// ── Runtime Quantization 运行时量化（本地计算，直接读取）──

/** 12 方向速度量化 12-direction velocity quantization */
UENUM(BlueprintType)
enum class EMovementDirection : uint8
{
	F,  // Forward 前
	FL, // Forward-Left 前左
	FR, // Forward-Right 前右
	B,  // Backward 后
	BL, // Backward-Left 后左
	BR, // Backward-Right 后右
	L,  // Left 左
	R,  // Right 右
	LL, // Lateral-Left 纯横移左
	LR, // Lateral-Right-Forward 横移右前
	RL, // Right-Left-Forward 右左前
	RR  // Lateral-Right 纯横移右
};

// ── Traversal 越障 ────────────────────────────────────

/** 越障动作类型 Vault / Mantle / Hurdle / Climb / Catch */
UENUM(BlueprintType)
enum class ETraversalActionType : uint8
{
	Vault,
	Mantle,
	Hurdle,
	Climb,
	Catch
};
