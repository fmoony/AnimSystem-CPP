// Copyright 2024 Locomotion System. All Rights Reserved.
// 越障类型定义 Traversal type definitions

#pragma once

#include "CoreMinimal.h"
#include "Character/Core/CharacterEnums.h"

/**
 * 越障检测输入 Traversal check input
 *
 * 由 TraversalComponent 在收到 Traverse 输入时构建。
 */
struct FTraversalCheckInput
{
	/** 角色当前位置 Character current location */
	FVector CharacterLocation = FVector::ZeroVector;

	/** 角色当前朝向 Character current forward direction */
	FVector CharacterForward = FVector::ForwardVector;

	/** 角色胶囊体半径 Character capsule radius */
	float CapsuleRadius = 34.f;

	/** 角色胶囊体半高 Character capsule half-height */
	float CapsuleHalfHeight = 88.f;

	/** 当前速度向量 Current velocity */
	FVector Velocity = FVector::ZeroVector;
};

/**
 * 越障检测结果 Traversal check result
 *
 * 从 Trace 检测中提取的障碍物几何信息。
 */
struct FTraversalCheckResult
{
	/** 是否检测到可越障物 Found a traversable obstacle */
	bool bFoundObstacle = false;

	/** 越障动作类型 Traversal action type */
	ETraversalActionType ActionType = ETraversalActionType::Vault;

	/** 障碍物顶部位置 Obstacle top edge location */
	FVector ObstacleTop = FVector::ZeroVector;

	/** 障碍物深度 Obstacle depth */
	float ObstacleDepth = 0.f;

	/** 障碍物高度 Obstacle height */
	float ObstacleHeight = 0.f;

	/** 着陆点位置 Landing location after traversal */
	FVector LandingLocation = FVector::ZeroVector;
};
