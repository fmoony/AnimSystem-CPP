# C++ 动画系统架构设计 v3-final

> 状态（State）+ 事件（Event）+ 直接读取（RuntimeData）三者并存，不走"万物皆事件"

---

## 一、组件树

```
ACharacter (ALocomotionCharacter)
│
├── InputComponent          // EnhancedInput → 直接调 StateComp + MoverComp
│
├── StateComponent          // 状态判定（服务端权威）
│     ├── StateSnapshot     // 持久状态包（网络复制）
│     └── Publish           // 仅持久状态变化时发事件
│           GaitChanged
│           StanceChanged
│           MovementStateChanged
│           RotationModeChanged
│
├── EventBus               // 瞬时事件（一次性触发）
│     ├── Jump
│     ├── Land
│     ├── Footstep
│     ├── TraversalStart
│     ├── TraversalEnd
│     ├── WeaponFire
│     └── HitReact
│
├── MoverComponent          // 移动（Velocity/Speed/Trajectory 权威来源）
│
├── AnimInstance            // PoseSearch + Warping
│     ├── Read StateSnapshot
│     ├── Read Velocity / Speed / Trajectory（直接读 MoverComp）
│     └── Subscribe OnStateChanged（切换 PoseSearch DB）
│
├── TraversalComponent      // 越障检测 + MotionWarping
│
├── FoleyComponent          // 音效
│
└── CameraComponent         // 相机
```

---

## 二、三种数据，三种处理方式

### State（持久状态）— StateComponent 持有 + 发布变化事件

| 数据 | 变化频率 | 处理方式 |
|------|---------|---------|
| Gait | 每秒 0~5 次 | StateComponent 持有 → 变化时 `OnStateChanged.Broadcast(Snapshot)` |
| Stance | 同上 | 同上 |
| MovementState | 同上 | 同上 |
| RotationMode | 同上 | 同上 |

```cpp
// 订阅方：AnimInstance
StateComponent->OnStateChanged.AddWeakLambda(this, [this](const FCharacterStateSnapshot& S)
{
    // 切换 PoseSearch 数据库
    CurrentDatabase = SelectDatabase(S.Gait, S.Stance, S.MovementState);
});
```

### Event（瞬时事件）— EventBus 路由

| 事件 | 触发时机 | 发布者 |
|------|---------|--------|
| Jump | 起跳帧 | StateComponent 检测到进入 InAir |
| Land | 落地帧 | StateComponent 检测到离开 InAir |
| Footstep | 动画通知帧 | AnimNotify（通过 AnimInstance） |
| TraversalStart | 越障开始 | TraversalComponent |
| TraversalEnd | 越障结束 | TraversalComponent |
| WeaponFire | 开火帧 | 武器组件 |
| HitReact | 受击帧 | 伤害系统 |

```cpp
// 发布
EventBus->Publish(FJumpEvent{ LaunchVelocity });
EventBus->Publish(FLandEvent{ ImpactForce, LandingGait });

// 订阅
EventBus->Subscribe<FLandEvent>(this, [this](const FLandEvent& E) {
    PlayLandSound(E.ImpactForce);  // FoleyComponent
});
```

### RuntimeData（连续变化）— 直接读取

| 数据 | 来源 | 读取方式 |
|------|------|---------|
| Velocity | MoverComponent | `MoverComp->GetVelocity()` |
| Speed | MoverComponent | `MoverComp->GetCurrentSpeed()` |
| Trajectory | MoverComponent | `MoverComp->GetPredictedTrajectory()` |
| Direction | StateComponent | `StateComp->GetCurrentDirection()` |
| IsOnGround | MoverComponent | `MoverComp->IsOnGround()` |

```cpp
// AnimInstance::NativeUpdateAnimation — 每帧
void UPoseSearchAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    // 直接读，不订阅
    CurrentSpeed = MoverComp->GetCurrentSpeed();
    CurrentVelocity = MoverComp->GetVelocity();
    CurrentDirection = StateComp->GetCurrentDirection();
    PredictedTrajectory = MoverComp->GetPredictedTrajectory();
    
    // PoseSearch 节点用这些数据作为查询输入
}
```

---

## 三、EventBus 实现（修正版）

### 3.0 硬约束

```
EventBus = 纯管道，零状态

✅ Subscribe / Unsubscribe / Publish
❌ 不持有游戏数据、不缓存事件、不 Tick、不复制、不依赖其他组件
```

### 3.1 修正实现

**修了三个问题：**
1. `static` 改实例 `TMap` — 每个角色独立
2. `Subscribe` 用 `this` + `AddWeakLambda` — UObject 销毁自动解绑
3. 纯模板，编译期确定，零运行时类型信息开销

```cpp
// CharacterEventBus.h
UCLASS()
class UCharacterEventBus : public UActorComponent
{
    GENERATED_BODY()

public:
    // 订阅（WeakLambda — 订阅者销毁后自动失效）
    template<typename TEvent, typename TUserObject>
    FDelegateHandle Subscribe(TUserObject* InUserObject, 
                              TFunction<void(const TEvent&)> Callback)
    {
        auto& Delegate = GetDispatcher<TEvent>();
        return Delegate.AddWeakLambda(InUserObject, MoveTemp(Callback));
    }

    // 取消订阅
    template<typename TEvent>
    void Unsubscribe(FDelegateHandle Handle)
    {
        GetDispatcher<TEvent>().Remove(Handle);
    }

    // 发布（纯转发，不存储事件）
    template<typename TEvent>
    void Publish(const TEvent& Event)
    {
        GetDispatcher<TEvent>().Broadcast(Event);
    }

private:
    template<typename TEvent>
    TMulticastDelegate<void(const TEvent&)>& GetDispatcher()
    {
        // 实例级 TMap，每个角色独立，非 static
        const int32 Key = IndexFor<TEvent>();
        if (!DispatcherMap.Contains(Key))
        {
            DispatcherMap.Add(Key, TMulticastDelegate<void(const TEvent&)>());
        }
        return *reinterpret_cast<TMulticastDelegate<void(const TEvent&)>*>(
            DispatcherMap.Find(Key));
    }

    template<typename TEvent>
    static int32 IndexFor()
    {
        static int32 Idx = NextIndex++;
        return Idx;
    }

    static int32 NextIndex;
    
    // 每个 Bus 实例独立的委托池（非 static）
    // 实际因 UE 委托无拷贝构造，可用 TSharedPtr 包装
    TMap<int32, TSharedPtr<void>> DispatcherMap;
};
```

---

## 四、StateComponent 设计

```cpp
// CharacterStateComponent.h
UCLASS()
class UCharacterStateComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // ── 持久状态快照（网络复制）───────────────────────
    UPROPERTY(ReplicatedUsing=OnRep_StateSnapshot)
    FCharacterStateSnapshot StateSnapshot;

    // ── 持久状态变化事件 ──────────────────────────────
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnStateChanged, const FCharacterStateSnapshot&);
    FOnStateChanged OnStateChanged;

    // ── RuntimeData 直接读取接口（高频，不复制）─────────
    EMovementDirection GetCurrentDirection() const { return CurrentDirection; }
    EGait              GetCurrentGait()      const { return StateSnapshot.Gait; }
    EStance            GetCurrentStance()    const { return StateSnapshot.Stance; }
    EMovementState     GetCurrentMovementState() const { return StateSnapshot.MovementState; }

    // ── 服务端权威更新 ────────────────────────────────
    void UpdateState(const FCharacterInputData& Input, 
                     const FVector& Velocity, 
                     bool bIsOnGround);

private:
    UFUNCTION()
    void OnRep_StateSnapshot();

    // RuntimeData — 本地缓存，不复制，直接读
    EMovementDirection CurrentDirection;

    // 纯函数判定
    static EGait              DetermineGait(const FCharacterInputData& Input, float Speed);
    static EStance            DetermineStance(const FCharacterInputData& Input, EStance Current);
    static EMovementState     DetermineMovementState(float Speed, bool bOnGround, bool bTraversing);
    static EMovementDirection QuantizeDirection(const FVector& Velocity, const FRotator& ActorRotation);
};
```

---

## 五、InputComponent — 不经过 EventBus

```cpp
// CharacterInputComponent.h
UCLASS()
class UCharacterInputComponent : public UActorComponent
{
    // 绑定 Enhanced Input Actions
    // 每帧直接调用 StateComponent + MoverComponent，不经过事件总线
    
    void TickComponent(float DeltaTime, ELevelTick TickType,
                       FActorComponentTickFunction* ThisTickFunction) override
    {
        FCharacterInputData Input = BuildInputData();
        
        // 直接调用（高频核心路径，不需要事件中转）
        StateComp->UpdateState(Input, MoverComp->GetVelocity(), MoverComp->IsOnGround());
        MoverComp->ApplyInput(Input);
        
        // 瞬时按键检测 → EventBus
        if (Input.bJumpPressed && StateComp->CanJump())
        {
            MoverComp->TriggerJump();
            // Jump 事件由 StateComponent 在检测到 InAir 时发布
        }
    }
};
```

---

## 六、组件职责总表

| 组件 | 持有状态 | 发布事件 | 订阅事件 | 直接读取 |
|------|---------|---------|---------|---------|
| **InputComponent** | — | — | — | — |
| **StateComponent** | StateSnapshot | `OnStateChanged` | — | — |
| **EventBus** | ❌ 零状态 | — | — | — |
| **MoverComponent** | Velocity/Speed/Trajectory | — | — | — |
| **AnimInstance** | — | `FFootstepEvent` → EventBus | `OnStateChanged` (切换DB) | Speed/Velocity/Direction/Trajectory |
| **TraversalComponent** | CurrentAction | `FTraversalStart/End` → EventBus | `OnStateChanged` (检查许可) | Velocity |
| **FoleyComponent** | — | — | `FFootstepEvent`, `FLandEvent`, `FJumpEvent` | Speed/Direction (选音效变体) |
| **CameraComponent** | — | — | `OnStateChanged` | Speed (FOV) |

---

## 七、网络同步

```
服务端权威:
  StateComponent  → Snapshot 复制（5字节压缩包，仅在变化时发送）
  MoverComponent  → 位置/速度 复制（Mover 内置 + NetworkPrediction）

客户端:
  OnRep_StateSnapshot → OnStateChanged.Broadcast → AnimInstance 切换 DB
  Mover 回滚/插值 → AnimInstance 直接读修正后的 Velocity/Speed/Trajectory
```

---

## 八、目录结构

```
Source/GameAnimationSample/
├── Core/
│   ├── CharacterEnums.h                // EGait, EStance, EMovementState...
│   ├── CharacterStateSnapshot.h        // FCharacterStateSnapshot (5字节)
│   ├── CharacterEvents.h               // 瞬时事件: FJumpEvent, FLandEvent...
│   ├── CharacterEventBus.h/.cpp        // 瞬时事件总线
│   └── CharacterInputData.h            // FCharacterInputData
│
├── Input/
│   └── CharacterInputComponent.h/.cpp
│
├── State/
│   └── CharacterStateComponent.h/.cpp
│
├── Movement/
│   └── CharacterMoverComponent.h/.cpp
│
├── Animation/
│   ├── PoseSearchAnimInstance.h/.cpp
│   └── Notifies/
│       └── AnimNotify_FoleyEvent.h/.cpp
│
├── Traversal/
│   ├── TraversalTypes.h
│   └── TraversalComponent.h/.cpp
│
├── Foley/
│   └── FoleyComponent.h/.cpp
│
├── Camera/
│   └── CharacterCameraComponent.h/.cpp
│
└── LocomotionCharacter.h/.cpp          // 组装所有组件
```

---

## 九、实施顺序

| # | 模块 | 内容 |
|---|------|------|
| 1 | `Core/` | 枚举 + Snapshot + 事件 struct + EventBus |
| 2 | `State/` | StateComponent（判定 + 复制 + OnStateChanged） |
| 3 | `Input/` | InputComponent（直接调 State + Mover） |
| 4 | `LocomotionCharacter` | 组装，本地跑通 |
| 5 | `Movement/` | MoverComponent |
| 6 | `Animation/` | AnimInstance（直接读数 + 订阅 OnStateChanged） |
| 7 | 网络验证 | 客户端-服务端复制 |
| 8+ | Traversal / Foley / Camera | 增量接入 |
