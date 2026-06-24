// Copyright 2024 Locomotion System. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Core/CharacterEvents.h"
#include "CharacterEventBus.generated.h"

// ─────────────────────────────────────────────────────
// 瞬时事件总线 — 纯路由，不持有任何游戏状态
//
// 硬约束：
//   ✅ Subscribe / Unsubscribe / Publish
//   ❌ 不持有状态、不缓存事件、不 Tick、不复制、不依赖其他组件
//   ❌ 不知道事件的具体语义（纯模板转发）
//
// 生命周期安全：
//   Subscribe 要求传入 UObject* Owner，
//   内部使用 AddWeakLambda，Owner 销毁后自动解绑。
// ─────────────────────────────────────────────────────

UCLASS()
class UCharacterEventBus : public UActorComponent
{
	GENERATED_BODY()

public:
	// ── 订阅 ──────────────────────────────────────────
	// Owner 用于生命周期绑定，Owner 销毁后自动解绑
	template<typename TEvent>
	FDelegateHandle Subscribe(UObject* Owner, TFunction<void(const TEvent&)> Callback)
	{
		auto& Dispatcher = GetDispatcher<TEvent>();
		return Dispatcher.AddWeakLambda(Owner, MoveTemp(Callback));
	}

	// ── 取消订阅 ──────────────────────────────────────
	template<typename TEvent>
	void Unsubscribe(FDelegateHandle Handle)
	{
		GetDispatcher<TEvent>().Remove(Handle);
	}

	// ── 发布（纯转发，不存储事件）─────────────────────
	template<typename TEvent>
	void Publish(const TEvent& Event)
	{
		GetDispatcher<TEvent>().Broadcast(Event);
	}

	// ── 便捷方法（常用事件提供显式重载，方便蓝图/调试）──
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
	// ── 每个事件类型独立一个委托，实例级（非 static）──
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
	TArray<TSharedPtr<IDelegateWrapperBase>> DispatcherPool;
};
