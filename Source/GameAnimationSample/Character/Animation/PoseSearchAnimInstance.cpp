// Copyright 2024 Locomotion System. All Rights Reserved.

#include "Character/Animation/PoseSearchAnimInstance.h"
#include "Character/State/CharacterStateComponent.h"
#include "Character/Movement/LocomotionMoverComponent.h"
#include "Character/Core/CharacterEventBus.h"
#include "Character/Core/CharacterEvents.h"
#include "GameFramework/Character.h"

UPoseSearchAnimInstance::UPoseSearchAnimInstance()
{
}

void UPoseSearchAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	CacheComponents();
}

void UPoseSearchAnimInstance::CacheComponents()
{
	AActor* Owner = GetOwningActor();
	if (!Owner) return;

	StateComponent = Owner->FindComponentByClass<UCharacterStateComponent>();
	MoverComponent  = Owner->FindComponentByClass<ULocomotionMoverComponent>();
	EventBus        = Owner->FindComponentByClass<UCharacterEventBus>();

	// ── 订阅持久状态变化 → 切换 DB（低频，≤5次/秒）─────
	if (StateComponent)
	{
		StateChangedHandle = StateComponent->OnStateChanged.AddWeakLambda(this,
			[this](const FCharacterStateSnapshot& Snapshot)
			{
				// AnimGraph 读取 CurrentState 在下次 NativeUpdateAnimation 时生效
				CurrentState = Snapshot;
			});
	}
}

// ─────────────────────────────────────────────────────
// 每帧更新 — 直接读 MoverComponent + StateComponent
// ─────────────────────────────────────────────────────
void UPoseSearchAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// 延迟初始化（可能在 BeginPlay 时 Owner 尚未就绪）
	if (!StateComponent || !MoverComponent)
	{
		CacheComponents();
		if (!StateComponent || !MoverComponent) return;
	}

	// ── 直接读 RuntimeData（高频，不通过事件）───────────
	CurrentSpeed = MoverComponent->GetCurrentSpeed();
	CurrentVelocity = MoverComponent->GetCurrentVelocity();
	bIsOnGround = MoverComponent->IsOnGround();
	CurrentDirection = StateComponent->GetCurrentDirection();

	// ── 直接读 Persistent State（也可能由 OnStateChanged 更新）─
	if (CurrentState.Speed != CurrentSpeed)
	{
		CurrentState.Speed = CurrentSpeed; // Speed 是 Runtime，每帧同步
	}

	// ── 轨迹预测 → PoseSearch 查询输入 ─────────────────
	UpdateTrajectory(DeltaSeconds);

	// ── Warping Alpha（根据 RotationMode 调整）───────────
	if (CurrentState.RotationMode == ERotationMode::LookingDirection)
	{
		// 横移/瞄准模式：全朝向 warping
		OrientationWarpingAlpha = FMath::FInterpTo(OrientationWarpingAlpha, 1.f, DeltaSeconds, 5.f);
	}
	else
	{
		// 移动方向模式：部分 warping（允许自然转身）
		OrientationWarpingAlpha = FMath::FInterpTo(OrientationWarpingAlpha, 0.5f, DeltaSeconds, 5.f);
	}

	// StrideWarping Alpha：低速时减弱（防止站立时步幅扭曲）
	const float TargetStrideAlpha = (CurrentSpeed > 100.f) ? 1.f : 0.f;
	StrideWarpingAlpha = FMath::FInterpTo(StrideWarpingAlpha, TargetStrideAlpha, DeltaSeconds, 10.f);
}

// ─────────────────────────────────────────────────────
// 轨迹预测
// ─────────────────────────────────────────────────────
void UPoseSearchAnimInstance::UpdateTrajectory(float DeltaSeconds)
{
	TArray<ULocomotionMoverComponent::FTrajectoryPoint> Trajectory;
	MoverComponent->GetPredictedTrajectory(Trajectory, 1.f, 20);

	FuturePositions.Reset(Trajectory.Num());
	FutureRotations.Reset(Trajectory.Num());

	for (const auto& Point : Trajectory)
	{
		// 转到骨骼网格组件本地空间
		const FVector LocalPos = GetSkelMeshComponent()
			? GetSkelMeshComponent()->GetComponentTransform().InverseTransformPosition(Point.Position)
			: Point.Position;

		FuturePositions.Add(LocalPos);
		FutureRotations.Add(Point.Rotation);
	}
}

// ─────────────────────────────────────────────────────
// 脚步事件 → EventBus
// ─────────────────────────────────────────────────────
void UPoseSearchAnimInstance::NotifyFootstep(EFootstepSide Side)
{
	if (EventBus)
	{
		EventBus->PublishFootstep(Side == EFootstepSide::Left
			? FFootstepEvent::ESide::Left
			: FFootstepEvent::ESide::Right);
	}
}
