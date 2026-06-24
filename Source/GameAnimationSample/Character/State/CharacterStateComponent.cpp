// Copyright 2024 Locomotion System. All Rights Reserved.
// 角色状态组件实现 Character state component implementation

#include "Character/State/CharacterStateComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// ── Speed Thresholds 速度阈值 ──────────────────────────

// 超过此值为 Locomotion Above = Locomotion
static constexpr float WalkSpeedThreshold  = 10.f;
// 超过此值为 Run Above = Run
static constexpr float RunSpeedThreshold   = 200.f;
// 超过此值为 Sprint Above = Sprint
static constexpr float SprintSpeedThreshold = 500.f;

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

	// 仅在状态变化时复制 Replicate only when state changes
	DOREPLIFETIME_CONDITION(UCharacterStateComponent, StateSnapshot, COND_None);
}

// ── UpdateState 服务端权威更新 ─────────────────────────

void UCharacterStateComponent::UpdateState(const FCharacterInputData& Input,
										   const FVector& Velocity,
										   bool bIsOnGround)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	const float Speed = Velocity.Size2D();
	const bool bHasInput = Input.HasMovementInput();

	// 判定当前状态 Determine current state
	const EGait NewGait = DetermineGait(Input, Speed, bIsOnGround);
	const EStance NewStance = DetermineStance(Input.bCrouchToggled, StateSnapshot.Stance);
	const EMovementState NewMovementState = DetermineMovementState(Speed, bIsOnGround, false);
	const ERotationMode NewRotationMode = Input.bStrafeHeld
		? ERotationMode::LookingDirection
		: ERotationMode::VelocityDirection;

	// 方向量化 Direction quantization（本地 RuntimeData）
	if (bHasInput && Speed > WalkSpeedThreshold)
	{
		AActor* Owner = GetOwner();
		const FRotator ActorRot = Owner ? Owner->GetActorRotation() : FRotator::ZeroRotator;
		CurrentDirection = QuantizeDirection(Velocity.GetSafeNormal2D(), ActorRot);
	}

	// 构建快照，比较后决定是否复制 Build snapshot, compare, replicate if changed
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

// ── LocalPredict 客户端本地预测 ────────────────────────

void UCharacterStateComponent::LocalPredict(const FCharacterInputData& Input,
											const FVector& Velocity,
											bool bIsOnGround)
{
	const float Speed = Velocity.Size2D();
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

	// 仅预测值变化时广播 Broadcast only when prediction differs
	if (Predicted != StateSnapshot)
	{
		StateSnapshot = Predicted;
		CurrentDirection = Input.HasMovementInput()
			? QuantizeDirection(Velocity.GetSafeNormal2D(), ActorRot)
			: CurrentDirection;
		OnStateChanged.Broadcast(StateSnapshot);
	}

	PreviousStance = NewStance;
}

// ── OnRep_StateSnapshot 客户端收到复制 ─────────────────

void UCharacterStateComponent::OnRep_StateSnapshot()
{
	OnStateChanged.Broadcast(StateSnapshot);
}

// ── DetermineGait 步态判定 静态纯函数 ──────────────────

EGait UCharacterStateComponent::DetermineGait(const FCharacterInputData& Input,
											  float Speed,
											  bool bIsOnGround)
{
	// 空中保持 Run Keep Run while airborne
	if (!bIsOnGround) return EGait::Run;

	// 按住 Sprint 且速度足够 Sprint held and fast enough
	if (Input.bSprintHeld && Speed > SprintSpeedThreshold)
	{
		return EGait::Sprint;
	}

	// 按住 Walk 强制步行 Walk held forces walk
	if (Input.bWalkHeld)
	{
		return EGait::Walk;
	}

	// 默认按速度判定 Default: speed-based
	if (Speed < WalkSpeedThreshold) return EGait::Walk;
	if (Speed < SprintSpeedThreshold) return EGait::Run;
	return EGait::Sprint;
}

// ── DetermineStance 站姿判定 静态纯函数 ────────────────

EStance UCharacterStateComponent::DetermineStance(bool bCrouchToggled, EStance Current)
{
	if (!bCrouchToggled) return Current;
	return (Current == EStance::Stand) ? EStance::Crouch : EStance::Stand;
}

// ── DetermineMovementState 运动状态判定 静态纯函数 ─────

EMovementState UCharacterStateComponent::DetermineMovementState(float Speed,
																bool bIsOnGround,
																bool bIsTraversing)
{
	if (bIsTraversing) return EMovementState::Traversal;
	if (!bIsOnGround)  return EMovementState::InAir;
	return (Speed > WalkSpeedThreshold) ? EMovementState::Locomotion : EMovementState::Idle;
}

// ── QuantizeDirection 速度向量→12方向 静态纯函数 ───────

EMovementDirection UCharacterStateComponent::QuantizeDirection(const FVector& Velocity2D,
																const FRotator& ActorRotation)
{
	if (Velocity2D.IsNearlyZero())
	{
		return EMovementDirection::F;
	}

	// 转到角色本地空间 Transform to local space
	const FVector LocalVel = ActorRotation.UnrotateVector(Velocity2D);
	const float AngleRad = FMath::Atan2(LocalVel.Y, LocalVel.X);
	const float AngleDeg = FMath::RadiansToDegrees(AngleRad);

	// 12 扇区，每扇 30° 12 sectors, 30° each
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
