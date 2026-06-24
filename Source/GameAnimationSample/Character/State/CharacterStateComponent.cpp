// Copyright 2024 Locomotion System. All Rights Reserved.

#include "Character/State/CharacterStateComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// ── Speed thresholds ─────────────────────────────────
static constexpr float WalkSpeedThreshold  = 10.f;   // > 此值为 Locomotion
static constexpr float RunSpeedThreshold   = 200.f;  // > 此值为 Run
static constexpr float SprintSpeedThreshold = 500.f; // > 此值为 Sprint

UCharacterStateComponent::UCharacterStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UCharacterStateComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCharacterStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 仅在状态变化时复制（COND_None + FCharacterStateSnapshot::operator!=）
	DOREPLIFETIME_CONDITION(UCharacterStateComponent, StateSnapshot, COND_None);
}

// ─────────────────────────────────────────────────────
// 服务端权威更新
// ─────────────────────────────────────────────────────
void UCharacterStateComponent::UpdateState(const FCharacterInputData& Input,
										   const FVector& Velocity,
										   bool bIsOnGround)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	const float Speed = Velocity.Size2D();
	const bool bHasInput = Input.HasMovementInput();

	// ── 判定 ──────────────────────────────────────────
	const EGait NewGait = DetermineGait(Input, Speed, bIsOnGround);
	const EStance NewStance = DetermineStance(Input.bCrouchToggled, StateSnapshot.Stance);
	const EMovementState NewMovementState = DetermineMovementState(Speed, bIsOnGround, false);
	const ERotationMode NewRotationMode = Input.bStrafeHeld
		? ERotationMode::LookingDirection
		: ERotationMode::VelocityDirection;

	// ── 方向量化（本地 RuntimeData）───────────────────
	if (bHasInput && Speed > WalkSpeedThreshold)
	{
		AActor* Owner = GetOwner();
		const FRotator ActorRot = Owner ? Owner->GetActorRotation() : FRotator::ZeroRotator;
		CurrentDirection = QuantizeDirection(Velocity.GetSafeNormal2D(), ActorRot);
	}

	// ── 构建快照，比较后决定是否复制 ──────────────────
	FCharacterStateSnapshot NewSnapshot;
	NewSnapshot.Gait = NewGait;
	NewSnapshot.Stance = NewStance;
	NewSnapshot.MovementState = NewMovementState;
	NewSnapshot.RotationMode = NewRotationMode;
	NewSnapshot.Speed = Speed;

	if (NewSnapshot != StateSnapshot)
	{
		StateSnapshot = NewSnapshot;
		OnStateChanged.Broadcast(StateSnapshot);
	}
}

// ─────────────────────────────────────────────────────
// 客户端本地预测
// ─────────────────────────────────────────────────────
void UCharacterStateComponent::LocalPredict(const FCharacterInputData& Input,
											const FVector& Velocity,
											bool bIsOnGround)
{
	const float Speed = Velocity.Size2D();
	const bool bHasInput = Input.HasMovementInput();
	const AActor* Owner = GetOwner();
	const FRotator ActorRot = Owner ? Owner->GetActorRotation() : FRotator::ZeroRotator;

	const EGait NewGait = DetermineGait(Input, Speed, bIsOnGround);
	const EStance NewStance = DetermineStance(Input.bCrouchToggled, PreviousStance);
	const EMovementState NewMovementState = DetermineMovementState(Speed, bIsOnGround, false);

	FCharacterStateSnapshot Predicted;
	Predicted.Gait = NewGait;
	Predicted.Stance = NewStance;
	Predicted.MovementState = NewMovementState;
	Predicted.RotationMode = Input.bStrafeHeld
		? ERotationMode::LookingDirection
		: ERotationMode::VelocityDirection;
	Predicted.Speed = Speed;

	if (Predicted != StateSnapshot)
	{
		StateSnapshot = Predicted;
		CurrentDirection = bHasInput ? QuantizeDirection(Velocity.GetSafeNormal2D(), ActorRot) : CurrentDirection;
		OnStateChanged.Broadcast(StateSnapshot);
	}

	PreviousStance = NewStance;
}

// ─────────────────────────────────────────────────────
// 客户端收到复制
// ─────────────────────────────────────────────────────
void UCharacterStateComponent::OnRep_StateSnapshot()
{
	OnStateChanged.Broadcast(StateSnapshot);
}

// ─────────────────────────────────────────────────────
// 静态判定 — Gait
// ─────────────────────────────────────────────────────
EGait UCharacterStateComponent::DetermineGait(const FCharacterInputData& Input,
											  float Speed,
											  bool bIsOnGround)
{
	if (!bIsOnGround) return EGait::Run; // 空中保持 Run

	if (Input.bSprintHeld && Speed > SprintSpeedThreshold)
	{
		return EGait::Sprint;
	}

	if (Input.bWalkHeld)
	{
		return EGait::Walk;
	}

	// 默认按速度判定
	if (Speed < WalkSpeedThreshold)
	{
		return EGait::Walk;
	}
	if (Speed < SprintSpeedThreshold)
	{
		return EGait::Run;
	}
	return EGait::Sprint;
}

// ─────────────────────────────────────────────────────
// 静态判定 — Stance
// ─────────────────────────────────────────────────────
EStance UCharacterStateComponent::DetermineStance(bool bCrouchToggled, EStance Current)
{
	if (!bCrouchToggled) return Current;
	return (Current == EStance::Stand) ? EStance::Crouch : EStance::Stand;
}

// ─────────────────────────────────────────────────────
// 静态判定 — MovementState
// ─────────────────────────────────────────────────────
EMovementState UCharacterStateComponent::DetermineMovementState(float Speed,
																bool bIsOnGround,
																bool bIsTraversing)
{
	if (bIsTraversing) return EMovementState::Traversal;
	if (!bIsOnGround)  return EMovementState::InAir;
	return (Speed > WalkSpeedThreshold) ? EMovementState::Locomotion : EMovementState::Idle;
}

// ─────────────────────────────────────────────────────
// 静态判定 — 速度向量 → 12方向
// ─────────────────────────────────────────────────────
EMovementDirection UCharacterStateComponent::QuantizeDirection(const FVector& Velocity2D,
																const FRotator& ActorRotation)
{
	if (Velocity2D.IsNearlyZero())
	{
		return EMovementDirection::F;
	}

	// 转到角色本地空间
	const FVector LocalVel = ActorRotation.UnrotateVector(Velocity2D);
	const float AngleRad = FMath::Atan2(LocalVel.Y, LocalVel.X); // -PI..PI
	const float AngleDeg = FMath::RadiansToDegrees(AngleRad);

	// 12 扇区，每扇 30°
	// F=0°, FL=30°, L=60°, BL=90° → 150°, B=180°, BR=-150°...RR=-30°
	if (AngleDeg > 165.f || AngleDeg <= -165.f) return EMovementDirection::B;
	if (AngleDeg > 135.f) return EMovementDirection::BL;
	if (AngleDeg > 105.f) return EMovementDirection::L;
	if (AngleDeg >  75.f) return EMovementDirection::FL;
	if (AngleDeg >  15.f) return EMovementDirection::F;
	if (AngleDeg > -15.f) return EMovementDirection::F;
	if (AngleDeg > -45.f) return EMovementDirection::FR;
	if (AngleDeg > -75.f) return EMovementDirection::R;
	if (AngleDeg > -105.f) return EMovementDirection::BR;
	if (AngleDeg > -135.f) return EMovementDirection::B;
	if (AngleDeg > -165.f) return EMovementDirection::BL;

	return EMovementDirection::B;
}
