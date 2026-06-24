// Copyright 2024 Locomotion System. All Rights Reserved.
// 音效组件实现 Foley component implementation

#include "Character/Foley/FoleyComponent.h"
#include "Character/Core/CharacterEventBus.h"
#include "Character/State/CharacterStateComponent.h"

UFoleyComponent::UFoleyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFoleyComponent::BeginPlay()
{
	Super::BeginPlay();

	EventBus = GetOwner()->FindComponentByClass<UCharacterEventBus>();
	StateComponent = GetOwner()->FindComponentByClass<UCharacterStateComponent>();

	// 订阅瞬时脚步事件 Subscribe to footstep events
	if (EventBus)
	{
		FootstepHandle = EventBus->Subscribe<FFootstepEvent>(this,
			[this](const FFootstepEvent& E) { OnFootstep(E.Side); });


		LandHandle = EventBus->Subscribe<FLandEvent>(this,
			[this](const FLandEvent& E) { OnLand(E.ImpactForce, E.LandingGait); });

		JumpHandle = EventBus->Subscribe<FJumpEvent>(this,
			[this](const FJumpEvent& E) { OnJump(E.LaunchVelocity); });
	}

	// 订阅步态变化 Subscribe to gait changes
	if (StateComponent)
	{
		StateChangedHandle = StateComponent->OnStateChanged.AddWeakLambda(this,
			[this](const FCharacterStateSnapshot& Snapshot)
			{
				OnGaitChanged(Snapshot.Gait);
				// 检测落地 Detect landing
				if (!bIsOnGround && Snapshot.MovementState == EMovementState::Locomotion)
				{
					OnLand(Snapshot.Speed, Snapshot.Gait);
				}
				bIsOnGround = (Snapshot.MovementState != EMovementState::InAir);
			});
	}
}

// ── OnFootstep 脚步事件 ───────────────────────────────

void UFoleyComponent::OnFootstep(EFootstepSide Side)
{
	// TODO: 根据 CurrentGait + Side 选择并播放音效 Select and play footstep sound
	// 音效选择逻辑：
	//   Walk → Foley_fs_1p_sneaker_concrete_walk_*
	//   Run  → Foley_fs_1p_sneaker_concrete_run_*
	//   Sprint → 使用 Run 音效（加速播放）
	//   Crouch → 使用 Walk 音效（降低音量）

	UE_LOG(LogTemp, Verbose, TEXT("[Foley] Footstep: Side=%d Gait=%d"), (int32)Side, (int32)CurrentGait);
}

// ── OnLand 落地事件 ───────────────────────────────────

void UFoleyComponent::OnLand(float ImpactForce, EGait Gait)
{
	// TODO: 根据 ImpactForce + Gait 选择落地音效
	// Heavy (>500) → Foley_fs_1p_sneaker_concrete_land_heavy
	// Light (<500) → Foley_fs_1p_sneaker_concrete_land_light

	UE_LOG(LogTemp, Verbose, TEXT("[Foley] Land: Force=%.0f Gait=%d"), ImpactForce, (int32)Gait);
}

// ── OnJump 跳跃事件 ───────────────────────────────────

void UFoleyComponent::OnJump(const FVector& LaunchVelocity)
{
	// TODO: 播放跳跃音效 Play jump sound

	UE_LOG(LogTemp, Verbose, TEXT("[Foley] Jump: Vel=(%.0f, %.0f, %.0f)"),
		LaunchVelocity.X, LaunchVelocity.Y, LaunchVelocity.Z);
}

// ── OnGaitChanged 步态变化 ────────────────────────────

void UFoleyComponent::OnGaitChanged(EGait NewGait)
{
	CurrentGait = NewGait;
	// TODO: 切换音效预设（音量/音色/SoundBank）
}
