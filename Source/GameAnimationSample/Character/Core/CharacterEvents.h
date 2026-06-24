// Copyright 2024 Locomotion System. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/Core/CharacterEnums.h"

// ─────────────────────────────────────────────────────
// 瞬时事件 — 一次性触发，通过 EventBus 路由
//
// 设计约束：
//   - 纯数据 struct，不继承 UObject
//   - 不持有状态，不追踪历史
//   - 仅在事件发生的帧有效
// ─────────────────────────────────────────────────────

// ── 跳跃 ────────────────────────────────────────────
struct FJumpEvent
{
	FVector LaunchVelocity = FVector::ZeroVector;
};

// ── 落地 ────────────────────────────────────────────
struct FLandEvent
{
	float ImpactForce = 0.f;
	EGait  LandingGait = EGait::Run;
};

// ── 脚步（由 AnimNotify 在动画帧触发）───────────────
struct FFootstepEvent
{
	enum class ESide : uint8 { Left, Right };
	ESide Side = ESide::Left;
};

// ── 越障 ────────────────────────────────────────────
struct FTraversalEvent
{
	enum class EPhase : uint8 { Started, Completed, Interrupted };
	EPhase Phase = EPhase::Started;
	ETraversalActionType ActionType = ETraversalActionType::Vault;
	FVector TargetLocation = FVector::ZeroVector;
};

// ── 受击 ────────────────────────────────────────────
struct FHitEvent
{
	AActor* Instigator = nullptr;
	FVector ImpactPoint = FVector::ZeroVector;
	FVector ImpactNormal = FVector::UpVector;
	float   Damage = 0.f;
};

// ── 武器开火 ────────────────────────────────────────
struct FWeaponFireEvent
{
	FVector  MuzzleLocation = FVector::ZeroVector;
	FRotator MuzzleRotation = FRotator::ZeroRotator;
};
