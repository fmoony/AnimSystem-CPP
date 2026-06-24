// Copyright 2024 Locomotion System. All Rights Reserved.

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
		StateComponent->OnStateChanged.AddWeakLambda(this, [this](const FCharacterStateSnapshot& Snapshot)
		{
			OnGaitChanged(Snapshot.Gait);
		});
	}

	if (CMC)
	{
		CMC->MaxWalkSpeed = RunSpeed;
	}
}

// ─────────────────────────────────────────────────────
// RuntimeData 直接读取
// ─────────────────────────────────────────────────────
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

// ─────────────────────────────────────────────────────
// 轨迹预测 — 用于 PoseSearch 查询输入
// ─────────────────────────────────────────────────────
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

	const float DeltaTime = PredictionTime / SampleCount;

	for (int32 i = 0; i <= SampleCount; ++i)
	{
		const float T = i * DeltaTime;
		FTrajectoryPoint Point;
		Point.Position = CurrentLocation + Velocity * T;
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

// ─────────────────────────────────────────────────────
// 输入响应
// ─────────────────────────────────────────────────────
void ULocomotionMoverComponent::ApplyMoveInput(const FVector2D& MoveInput)
{
	if (!OwnerCharacter) return;

	const FVector WorldMove = OwnerCharacter->GetActorForwardVector() * MoveInput.Y +
							  OwnerCharacter->GetActorRightVector()   * MoveInput.X;

	OwnerCharacter->AddMovementInput(WorldMove, 1.f);
}

// ─────────────────────────────────────────────────────
// OnStateChanged → 调整最大行走速度
// ─────────────────────────────────────────────────────
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
