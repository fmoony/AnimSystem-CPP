// Copyright 2024 Locomotion System. All Rights Reserved.
// 移动组件实现 Mover component implementation

#include "Character/Movement/LocomotionMoverComponent.h"
#include "Character/State/CharacterStateComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

ULocomotionMoverComponent::ULocomotionMoverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULocomotionMoverComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		CMC = OwnerCharacter->GetCharacterMovement();
	}

	StateComponent = GetOwner()->FindComponentByClass<UCharacterStateComponent>();
	if (StateComponent)
	{
		// 订阅状态变化 → 调整移动速度 Subscribe to state → adjust speed
		StateComponent->OnStateChanged.AddWeakLambda(this, [this](const FCharacterStateSnapshot& Snapshot)
		{
			OnGaitChanged(Snapshot.Gait);
		});
	}

	// 初始化默认速度 Init default speed
	if (CMC)
	{
		CMC->MaxWalkSpeed = RunSpeed;
	}
}

// ── RuntimeData 直接读取 Direct read ──────────────────

FVector ULocomotionMoverComponent::GetCurrentVelocity() const
{
	if (CMC) return CMC->Velocity;
	if (OwnerCharacter) return OwnerCharacter->GetVelocity();
	return FVector::ZeroVector;
}

float ULocomotionMoverComponent::GetCurrentSpeed() const
{
	if (CMC) return CMC->Velocity.Size2D();
	if (OwnerCharacter) return OwnerCharacter->GetVelocity().Size2D();
	return 0.f;
}

bool ULocomotionMoverComponent::IsOnGround() const
{
	if (CMC) return CMC->IsMovingOnGround();
	return true;
}

// ── 轨迹预测 Trajectory prediction ────────────────────

void ULocomotionMoverComponent::GetPredictedTrajectory(
	TArray<FTrajectoryPoint>& OutTrajectory,
	float PredictionTime,
	int32 SampleCount) const
{
	OutTrajectory.Reset(SampleCount);
	if (!OwnerCharacter || SampleCount < 1) return;

	const FVector CurrentLocation = OwnerCharacter->GetActorLocation();
	const FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
	const FVector Velocity = GetCurrentVelocity();

	// 简单匀速预测 Simple dead-reckoning prediction
	const float DeltaTime = PredictionTime / SampleCount;

	for (int32 i = 0; i <= SampleCount; ++i)
	{
		const float T = i * DeltaTime;
		FTrajectoryPoint Point;
		Point.Position = CurrentLocation + Velocity * T;

		// 速度够大时沿速度方向，否则保持当前朝向 Orient to velocity if fast enough
		if (Velocity.SizeSquared2D() > 100.f)
		{
			Point.Rotation = Velocity.ToOrientationQuat();
		}
		else
		{
			Point.Rotation = CurrentRotation.Quaternion();
		}
		Point.TimeOffset = T;
		OutTrajectory.Add(Point);
	}
}

// ── ApplyMoveInput 输入响应 ────────────────────────────

void ULocomotionMoverComponent::ApplyMoveInput(const FVector2D& MoveInput)
{
	if (!OwnerCharacter) return;

	// 世界空间移动 World-space movement
	const FVector WorldMove = OwnerCharacter->GetActorForwardVector() * MoveInput.Y +
							  OwnerCharacter->GetActorRightVector()   * MoveInput.X;

	OwnerCharacter->AddMovementInput(WorldMove, 1.f);
}

// ── OnGaitChanged 步态变化 → 调整最大速度 ─────────────

void ULocomotionMoverComponent::OnGaitChanged(EGait NewGait)
{
	if (!CMC) return;

	switch (NewGait)
	{
	case EGait::Walk:   CMC->MaxWalkSpeed = WalkSpeed;   break;
	case EGait::Run:    CMC->MaxWalkSpeed = RunSpeed;    break;
	case EGait::Sprint: CMC->MaxWalkSpeed = SprintSpeed; break;
	default: break;
	}
}
