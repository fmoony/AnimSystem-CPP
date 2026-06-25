# GM_Sandbox + PC_Sandbox 蓝图分析与 C++ 转换方案

> MCP 工具实时检查结果 | 2024-07-03

---

## 一、GM_Sandbox（GameMode）

### 1.1 蓝图内容

```
父类: GameModeBase
```

| 属性 | 值 | 说明 |
|------|-----|------|
| `DefaultPawnClass` | `CBP_SandboxCharacter` | 默认 Pawn |
| `PlayerControllerClass` | `PC_Sandbox` | 玩家控制器 |
| `GameStateClass` | `GameStateBase` | 默认 |
| `PlayerStateClass` | `PlayerState` | 默认 |
| `HUDClass` | `HUD` | 默认 |
| `bPauseable` | `true` | 允许暂停 |

**无自定义事件/函数**。纯粹的 Class 配置。

### 1.2 EventGraph 分析

35 个节点全部在 `EventGraph` 中，分为三个逻辑块：

**Block 1: 角色切换（IA_NextCharacter 触发）**
```
IA_NextCharacter 按下
  → Get Current Character Index
  → Index + 1, % Characters.Length  (循环)
  → Set Current Character Index
  → Get Controlled Pawn → Get Actor Transform → 保存位置/旋转
  → Destroy Actor (旧 Pawn)
  → Get Characters[Index] → SpawnActor (新角色, 保存的位置)
  → Possess (新角色)
  → Set View Target with Blend (平滑过渡)
```

**Block 2: 控制旋转缓存（Event Tick）**
```
Every Tick:
  → Get Control Rotation
  → Set CachedControlRotation  (角色切换时恢复旋转用)
```

**Block 3: 平台检测（Event Tick）**
```
Every Tick:
  → Get Platform Name
  → 如果是 Android/IOS:
      → 检测是否连接 Gamepad
      → Set Virtual Joystick Visibility (有手柄则隐藏)
```

### 1.3 蓝图变量

| 变量 | 类型 | 默认值 | 用途 |
|------|------|--------|------|
| `Characters` | `TArray<TSubclassOf<ACharacter>>` | 7 个角色蓝图 | 可切换的角色列表 |
| `Current Character Index` | `int32` | 0 | 当前选中的角色索引 |
| `CachedControlRotation` | `FRotator` | (0,0,0) | 缓存的控制器旋转 |

---

## 二、C++ 转换方案

### 2.1 ALocomotionGameMode

| 属性 | C++ 设置 |
|------|---------|
| `DefaultPawnClass` | `ALocomotionCharacter::StaticClass()` |
| `PlayerControllerClass` | `ALocomotionPlayerController::StaticClass()` |

```cpp
// Game/LocomotionGameMode.h
UCLASS()
class ALocomotionGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ALocomotionGameMode();
};

// Game/LocomotionGameMode.cpp
ALocomotionGameMode::ALocomotionGameMode()
{
    DefaultPawnClass = ALocomotionCharacter::StaticClass();
    PlayerControllerClass = ALocomotionPlayerController::StaticClass();
}
```

### 2.2 ALocomotionPlayerController

核心逻辑分三块：

#### A) 构造函数 — 输入/鼠标配置

```cpp
ALocomotionPlayerController::ALocomotionPlayerController()
{
    bShowMouseCursor = false;
    bEnableClickEvents = false;
    bEnableTouchEvents = true;
    DefaultMouseCursor = EMouseCursor::Default;
    CurrentMouseCursor = EMouseCursor::None;
}
```

#### B) 角色切换系统

```cpp
// 可切换的角色列表（在蓝图/子类中配置）
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
TArray<TSubclassOf<ALocomotionCharacter>> Characters;

UPROPERTY()
int32 CurrentCharacterIndex = 0;

// 切换角色
void SwitchToNextCharacter()
{
    if (Characters.Num() == 0) return;

    // 保存当前位置
    APawn* OldPawn = GetPawn();
    FTransform SpawnTransform = OldPawn 
        ? OldPawn->GetActorTransform() 
        : FTransform::Identity;

    // 循环索引
    CurrentCharacterIndex = (CurrentCharacterIndex + 1) % Characters.Num();

    // 销毁旧 Pawn
    if (OldPawn) OldPawn->Destroy();

    // 生成新角色
    ALocomotionCharacter* NewChar = GetWorld()->SpawnActor<ALocomotionCharacter>(
        Characters[CurrentCharacterIndex], SpawnTransform);

    // 控制新角色
    if (NewChar) Possess(NewChar);

    // 恢复控制旋转
    SetControlRotation(CachedControlRotation);
}
```

#### C) 控制旋转缓存 + 平台检测

```cpp
FRotator CachedControlRotation;

virtual void Tick(float DeltaTime) override
{
    Super::Tick(DeltaTime);
    CachedControlRotation = GetControlRotation();
}
```

---

## 三、文件清单

| 文件 | 操作 |
|------|------|
| `Game/LocomotionGameMode.h` | 新建 — 3 行核心代码 |
| `Game/LocomotionGameMode.cpp` | 新建 — 设置 DefaultPawnClass + PlayerControllerClass |
| `Game/LocomotionPlayerController.h` | 新建 — 角色切换 + 旋转缓存 |
| `Game/LocomotionPlayerController.cpp` | 新建 — ~80 行实现 |
| `Game/Game.Build.cs` | 不需要 — 使用项目模块的 `GameAnimationSample.Build.cs` |

---

## 四、架构影响

GM + PC 加入后的完整框架：

```
ALocomotionGameMode
  ├── DefaultPawnClass = ALocomotionCharacter
  └── PlayerControllerClass = ALocomotionPlayerController

ALocomotionPlayerController  (Possess)
  ├── 角色切换 (Characters 数组 + IA_NextCharacter)
  ├── 控制旋转缓存 (CachedControlRotation)
  ├── 输入模式 (GameOnly, 无鼠标)
  └── Possess → ALocomotionCharacter

ALocomotionCharacter
  ├── EventBus + StateComponent + InputComponent + MoverComponent
  ├── TraversalComponent + FoleyComponent
  └── CameraBoom + FollowCamera
```

**注意：** 角色切换逻辑（`IA_NextCharacter`）目前在 `PC_Sandbox` 的 EventGraph 中，但它使用了 EnhancedInput。在我们的 C++ 架构中，`UCharacterInputComponent` 已经绑定了 `IA_NextCharacter` — 但**角色切换应该在 Controller 层处理**（因为 Pawn 被销毁时 InputComponent 也销毁了）。

**修正方案：** 把 `IA_NextCharacter` 从 `UCharacterInputComponent` 移到 `ALocomotionPlayerController` 中绑定。Controller 比 Pawn 生命周期更稳定，切换 Pawn 时 Controller 不销毁。
