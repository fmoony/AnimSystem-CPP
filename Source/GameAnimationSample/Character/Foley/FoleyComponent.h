// Copyright 2024 Locomotion System. All Rights Reserved.
// 音效组件 Foley component — EventBus-driven sound effects

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Core/CharacterEnums.h"
#include "FoleyComponent.generated.h"

class UCharacterEventBus;
class UCharacterStateComponent;

/**
 * 音效组件 Foley component
 *
 * 订阅 EventBus（FFootstepEvent/FLandEvent/FJumpEvent）和
 * StateComponent::OnStateChanged（Gait 切换音效预设）。
 * 不主动轮询，完全事件驱动。
 */
UCLASS(ClassGroup=(Locomotion), meta=(BlueprintSpawnableComponent))
class UFoleyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFoleyComponent();

protected:
	virtual void BeginPlay() override;

private:
	// ── Event Handlers 事件处理 ────────────────────────

	void OnFootstep(EFootstepSide Side);
	void OnLand(float ImpactForce, EGait Gait);
	void OnJump(const FVector& LaunchVelocity);
	void OnGaitChanged(EGait NewGait);

	// ── Config 配置 ────────────────────────────────────

	/** 脚步音量 Footstep volume */
	UPROPERTY(EditAnywhere, Category = "Foley")
	float FootstepVolume = 1.f;

	/** 落地音量倍数 Land volume multiplier */
	UPROPERTY(EditAnywhere, Category = "Foley")
	float LandVolumeMultiplier = 1.5f;

	// ── Cached State 缓存状态 ──────────────────────────

	EGait CurrentGait = EGait::Run;
	bool bIsOnGround = true;

	// ── Cached References 缓存引用 ──────────────────────

	UPROPERTY()
	TObjectPtr<UCharacterEventBus> EventBus;

	UPROPERTY()
	TObjectPtr<UCharacterStateComponent> StateComponent;

	FDelegateHandle FootstepHandle;
	FDelegateHandle LandHandle;
	FDelegateHandle JumpHandle;
	FDelegateHandle StateChangedHandle;
};
