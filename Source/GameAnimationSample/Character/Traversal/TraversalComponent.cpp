// Copyright 2024 Locomotion System. All Rights Reserved.
// 越障组件实现 Traversal component implementation

#include "Character/Traversal/TraversalComponent.h"
#include "Character/Core/CharacterEventBus.h"
#include "Character/State/CharacterStateComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UTraversalComponent::UTraversalComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTraversalComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	StateComponent = GetOwner()->FindComponentByClass<UCharacterStateComponent>();
	EventBus = GetOwner()->FindComponentByClass<UCharacterEventBus>();
}

// ── TryTraversal 尝试越障 ─────────────────────────────

void UTraversalComponent::TryTraversal()
{
	// 已在越障中则忽略 Ignore if already traversing
	if (bIsTraversing) return;

	// 检测障碍物 Detect obstacle
	FTraversalCheckResult Result;
	if (!DetectObstacle(Result)) return;

	LastResult = Result;
	bIsTraversing = true;

	// 通过 EventBus 发布越障开始事件 Publish traversal start via EventBus
	if (EventBus)
	{
		EventBus->PublishTraversal(
			FTraversalEvent::EPhase::Started,
			Result.ActionType,
			Result.LandingLocation);
	}

	// TODO: 设置 MotionWarping 目标 Setup MotionWarping target
	// UMotionWarpingComponent::AddOrUpdateWarpTarget(...)
}

// ── CompleteTraversal 越障完成 ────────────────────────

void UTraversalComponent::CompleteTraversal(bool bSuccess)
{
	if (!bIsTraversing) return;

	bIsTraversing = false;

	// 发布越障完成事件 Publish traversal completed via EventBus
	if (EventBus)
	{
		EventBus->PublishTraversal(
			bSuccess ? FTraversalEvent::EPhase::Completed : FTraversalEvent::EPhase::Interrupted,
			LastResult.ActionType,
			LastResult.LandingLocation);
	}
}

// ── DetectObstacle 障碍物检测 ─────────────────────────

bool UTraversalComponent::DetectObstacle(FTraversalCheckResult& OutResult) const
{
	if (!OwnerCharacter) return false;

	UWorld* World = GetWorld();
	if (!World) return false;

	const FVector Location = OwnerCharacter->GetActorLocation();
	const FVector Forward = OwnerCharacter->GetActorForwardVector();
	const FVector Up = FVector::UpVector;

	// 构建检测输入 Build check input
	FTraversalCheckInput Input;
	Input.CharacterLocation = Location;
	Input.CharacterForward = Forward;
	Input.CapsuleRadius = OwnerCharacter->GetCapsuleComponent()
		? OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius() : 34.f;
	Input.CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()
		? OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 88.f;

	// 起始点：角色前方地面高度 Start point: in front of character at floor level
	const FVector TraceStart = Location + Forward * Input.CapsuleRadius +
							   Up * DetectionHeightOffset;
	const FVector TraceEnd = TraceStart + Forward * DetectionDistance;

	// 球形扫描 Sphere trace
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	QueryParams.bTraceComplex = false;

	FHitResult Hit;
	const bool bHit = World->SweepSingleByChannel(
		Hit, TraceStart, TraceEnd, FQuat::Identity,
		ECC_GameTraceChannel1, // Traversable channel
		FCollisionShape::MakeSphere(DetectionRadius),
		QueryParams);

	if (!bHit) return false;

	// 从碰撞点向上扫描，检测障碍物顶部 Scan upward from hit point for top edge
	const FVector WallTopStart = Hit.Location + Up * MaxClimbHeight;
	const FVector WallTopEnd = Hit.Location;
	FHitResult TopHit;
	const bool bTopHit = World->LineTraceSingleByChannel(
		TopHit, WallTopStart, WallTopEnd,
		ECC_GameTraceChannel1, QueryParams);

	const float ObstacleHeight = bTopHit
		? (WallTopStart.Z - TopHit.Location.Z)
		: (Hit.Location.Z - Location.Z);

	// 从顶部向前扫描，检测深度和着陆点 Scan forward from top for depth and landing
	const FVector OverTopStart = bTopHit ? TopHit.Location + Up * 10.f : WallTopStart;
	const FVector OverTopEnd = OverTopStart + Forward * (DetectionDistance + 50.f);
	FHitResult DepthHit;
	const bool bDepthHit = World->LineTraceSingleByChannel(
		DepthHit, OverTopStart, OverTopEnd,
		ECC_GameTraceChannel1, QueryParams);

	const float ObstacleDepth = bDepthHit
		? FVector::Dist(OverTopStart, DepthHit.Location)
		: 30.f;

	// 检查是否可越障 Check if traversable
	if (ObstacleHeight < 10.f || ObstacleHeight > MaxClimbHeight)
	{
		return false;
	}

	// 填充结果 Fill result
	OutResult.bFoundObstacle = true;
	OutResult.ActionType = ClassifyAction(ObstacleHeight, ObstacleDepth);
	OutResult.ObstacleTop = bTopHit ? TopHit.Location : Hit.Location + Up * ObstacleHeight;
	OutResult.ObstacleHeight = ObstacleHeight;
	OutResult.ObstacleDepth = ObstacleDepth;
	OutResult.LandingLocation = bDepthHit
		? DepthHit.Location + Forward * DetectionDistance * 0.5f
		: OverTopEnd;

	return true;
}

// ── ClassifyAction 动作分类 静态纯函数 ────────────────

ETraversalActionType UTraversalComponent::ClassifyAction(float ObstacleHeight, float ObstacleDepth)
{
	// 低矮宽障碍 → Vault Low + wide → Vault
	if (ObstacleHeight < 80.f && ObstacleDepth > 40.f)
	{
		return ETraversalActionType::Vault;
	}

	// 低矮窄障碍 → Hurdle Low + narrow → Hurdle
	if (ObstacleHeight < 100.f && ObstacleDepth < 40.f)
	{
		return ETraversalActionType::Hurdle;
	}

	// 中等高度 → Mantle Medium height → Mantle
	if (ObstacleHeight < 150.f)
	{
		return ETraversalActionType::Mantle;
	}

	// 较高 → Climb Tall → Climb
	return ETraversalActionType::Climb;
}
