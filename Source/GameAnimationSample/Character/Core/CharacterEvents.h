// Copyright 2024 Locomotion System. All Rights Reserved.
// 瞬时事件结构体 Instantaneous event structs — routed via EventBus

#pragma once

#include "CoreMinimal.h"
#include "Character/Core/CharacterEnums.h"

// ── Jump 跳跃 ─────────────────────────────────────────

/** 跳跃事件 Jump event */
struct FJumpEvent
{
	FVector LaunchVelocity = FVector::ZeroVector;
};

// ── Land 落地 ─────────────────────────────────────────

/** 落地事件 Land event */
struct FLandEvent
{
	float ImpactForce = 0.f;
	EGait  LandingGait = EGait::Run;
};

// ── Footstep 脚步 ─────────────────────────────────────

/** 脚步事件 Footstep event（由 AnimNotify 触发） */
struct FFootstepEvent
{
	enum class ESide : uint8 { Left, Right };
	ESide Side = ESide::Left;
};

// ── Traversal 越障 ────────────────────────────────────

/** 越障事件 Traversal event */
struct FTraversalEvent
{
	enum class EPhase : uint8 { Started, Completed, Interrupted };
	EPhase Phase = EPhase::Started;
	ETraversalActionType ActionType = ETraversalActionType::Vault;
	FVector TargetLocation = FVector::ZeroVector;
};

// ── Hit 受击 ──────────────────────────────────────────

/** 受击事件 Hit event */
struct FHitEvent
{
	AActor* Instigator = nullptr;
	FVector ImpactPoint = FVector::ZeroVector;
	FVector ImpactNormal = FVector::UpVector;
	float   Damage = 0.f;
};

// ── WeaponFire 开火 ───────────────────────────────────

/** 武器开火事件 Weapon fire event */
struct FWeaponFireEvent
{
	FVector  MuzzleLocation = FVector::ZeroVector;
	FRotator MuzzleRotation = FRotator::ZeroRotator;
};
