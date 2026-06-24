// Copyright 2024 Locomotion System. All Rights Reserved.
// 瞬时事件总线 — 纯路由不持有状态 Instantaneous event bus — pure routing, zero state

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Core/CharacterEvents.h"
#include "CharacterEventBus.generated.h"

/**
 * 瞬时事件总线 Instantaneous event bus
 *
 * 职责：Subscribe / Unsubscribe / Publish。
 * 硬约束：不持有游戏状态、不缓存事件、不 Tick、不复制、不依赖其他组件。
 * 生命周期安全：Subscribe 传入 UObject* Owner，内部 AddWeakLambda 自动解绑。
 */
UCLASS()
class UCharacterEventBus : public UActorComponent
{
	GENERATED_BODY()

public:
	// ── Subscribe 订阅 ──────────────────────────────────

	/** 订阅事件 Subscribe to event（Owner 销毁后自动解绑 auto-detach on owner destroy） */
	template<typename TEvent>
	FDelegateHandle Subscribe(UObject* Owner, TFunction<void(const TEvent&)> Callback)
	{
		auto& Dispatcher = GetDispatcher<TEvent>();
		return Dispatcher.AddWeakLambda(Owner, MoveTemp(Callback));
	}

	// ── Unsubscribe 取消订阅 ────────────────────────────

	template<typename TEvent>
	void Unsubscribe(FDelegateHandle Handle)
	{
		GetDispatcher<TEvent>().Remove(Handle);
	}

	// ── Publish 发布 ────────────────────────────────────

	/** 发布事件 Publish event（纯转发，不存储） */
	template<typename TEvent>
	void Publish(const TEvent& Event)
	{
		GetDispatcher<TEvent>().Broadcast(Event);
	}

	// ── Convenience 便捷方法 ────────────────────────────

	void PublishJump(const FVector& LaunchVelocity)
	{
		Publish(FJumpEvent{ LaunchVelocity });
	}

	void PublishLand(float ImpactForce, EGait LandingGait)
	{
		Publish(FLandEvent{ ImpactForce, LandingGait });
	}

	void PublishFootstep(FFootstepEvent::ESide Side)
	{
		Publish(FFootstepEvent{ Side });
	}

	void PublishTraversal(FTraversalEvent::EPhase Phase, ETraversalActionType Type, const FVector& Target)
	{
		Publish(FTraversalEvent{ Phase, Type, Target });
	}

	void PublishHit(AActor* Instigator, const FVector& Point, const FVector& Normal, float Damage)
	{
		Publish(FHitEvent{ Instigator, Point, Normal, Damage });
	}

	void PublishWeaponFire(const FVector& MuzzleLoc, const FRotator& MuzzleRot)
	{
		Publish(FWeaponFireEvent{ MuzzleLoc, MuzzleRot });
	}

private:
	// ── Internal 内部实现 ───────────────────────────────

	template<typename TEvent>
	TMulticastDelegate<void(const TEvent&)>& GetDispatcher()
	{
		static const int32 TypeIndex = NextTypeIndex++;
		if (!DispatcherPool.IsValidIndex(TypeIndex))
		{
			DispatcherPool.SetNum(TypeIndex + 1);
		}
		if (!DispatcherPool[TypeIndex].IsValid())
		{
			DispatcherPool[TypeIndex] = MakeShared<TDelegateWrapper<TEvent>>();
		}
		return StaticCastSharedPtr<TDelegateWrapper<TEvent>>(DispatcherPool[TypeIndex])->Delegate;
	}

	struct IDelegateWrapperBase
	{
		virtual ~IDelegateWrapperBase() = default;
	};

	template<typename TEvent>
	struct TDelegateWrapper : IDelegateWrapperBase
	{
		TMulticastDelegate<void(const TEvent&)> Delegate;
	};

	static int32 NextTypeIndex;

	// 每个 Bus 实例独立的委托池 Per-instance delegate pool（非 static）
	TArray<TSharedPtr<IDelegateWrapperBase>> DispatcherPool;
};
