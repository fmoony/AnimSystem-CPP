// Copyright 2024 Locomotion System. All Rights Reserved.
// PoseSearch 动画实例实现 PoseSearch animation instance implementation

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

	// 订阅持久状态变化 → 切换 DB（低频，≤5次/秒）Subscribe to persistent state changes
	if (StateComponent)
	{
		StateChangedHandle = StateComponent->OnStateChanged.AddWeakLambda(this,
			[this](const FCharacterStateSnapshot& Snapshot)
			{
				CurrentState = Snapshot;
			});
	}
}

// ── NativeUpdateAnimation 每帧更新 ────────────────────

void UPoseSearchAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// 延迟初始化 Deferred init（Owner 可能尚未就绪）
	if (!StateComponent || !MoverComponent)
	{
		CacheComponents();
		if (!StateComponent || !MoverComponent) return;
	}

	// 直接读 RuntimeData 高频数据 Direct poll high-frequency data
	CurrentSpeed = MoverComponent->GetCurrentSpeed();
	CurrentVelocity = MoverComponent->GetCurrentVelocity();
	bIsOnGround = MoverComponent->IsOnGround();
	CurrentDirection = StateComponent->GetCurrentDirection();

	// Speed 是 Runtime，每帧同步 Speed is runtime, sync every frame
	if (CurrentState.Speed != CurrentSpeed)
	{
		CurrentState.Speed = CurrentSpeed;
	}

	// 轨迹预测 Trajectory prediction
	UpdateTrajectory(DeltaSeconds);

	// 朝向 Warping Alpha Orientation warping alpha
	if (CurrentState.RotationMode == ERotationMode::LookingDirection)
	{
		// 横移/瞄准模式：全朝向 warping Strafe/aim mode: full orientation warp
		OrientationWarpingAlpha = FMath::FInterpTo(OrientationWarpingAlpha, 1.f, DeltaSeconds, 5.f);
	}
	else
	{
		// 移动方向模式：部分 warping Velocity mode: partial warp
		OrientationWarpingAlpha = FMath::FInterpTo(OrientationWarpingAlpha, 0.5f, DeltaSeconds, 5.f);
	}

	// 步幅 Warping Alpha Stride warping alpha
	const float TargetStrideAlpha = (CurrentSpeed > 100.f) ? 1.f : 0.f;
	StrideWarpingAlpha = FMath::FInterpTo(StrideWarpingAlpha, TargetStrideAlpha, DeltaSeconds, 10.f);
}

// ── UpdateTrajectory 轨迹预测 ─────────────────────────

void UPoseSearchAnimInstance::UpdateTrajectory(float DeltaSeconds)
{
	TArray<ULocomotionMoverComponent::FTrajectoryPoint> Trajectory;
	MoverComponent->GetPredictedTrajectory(Trajectory, 1.f, 20);

	FuturePositions.Reset(Trajectory.Num());
	FutureRotations.Reset(Trajectory.Num());

	for (const auto& Point : Trajectory)
	{
		// 转到骨骼网格本地空间 Transform to skeletal mesh local space
		const FVector LocalPos = GetSkelMeshComponent()
			? GetSkelMeshComponent()->GetComponentTransform().InverseTransformPosition(Point.Position)
			: Point.Position;

		FuturePositions.Add(LocalPos);
		FutureRotations.Add(Point.Rotation);
	}
}

// ── NotifyFootstep 脚步事件 → EventBus ────────────────

void UPoseSearchAnimInstance::NotifyFootstep(EFootstepSide Side)
{
	if (EventBus)
	{
		EventBus->PublishFootstep(Side == EFootstepSide::Left
			? FFootstepEvent::ESide::Left
			: FFootstepEvent::ESide::Right);
	}
}
