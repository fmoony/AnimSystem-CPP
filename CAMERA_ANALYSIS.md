# 相机系统蓝图分析与 C++ 转换方案

> MCP 实时检查 | CameraDirector + BPI_GameplayCamera + CBP_SandboxCharacter

---

## 一、蓝图相机架构

### 1.1 组件清单

| 蓝图 | 父类 | 职责 |
|------|------|------|
| `CameraDirector_SandboxCharacter` | `BlueprintCameraDirectorEvaluator` | 相机模式调度中心 |
| `BPI_GameplayCamera` | `Interface` | 相机-角色通信接口 |
| `CBP_SandboxCharacter` | `Character` | 持有 `CameraStyle` / `MovementStickMode` / `GameplayCamera` |

### 1.2 关键枚举（推断）

| 枚举 | 取值 |
|------|------|
| `E_CameraStyle` | ≥7 个值（从 Switch on E_CameraStyle 的 7 个 Activate Camera Rig 分支推断） |
| `E_CameraMode` | ≥3 个值（从 3 个 Switch on E_CameraMode 推断） |
| `E_Stance` | 影响相机模式切换（Stand / Crouch → 不同相机 rig） |

### 1.3 CameraDirector EventGraph 逻辑流（34 节点）

```
Event Run Camera Director
  → Find Evaluation Context Owner Actor（获取角色引用）
  → Message: Get_CharacterPropertiesForCamera（BPI 调用，从角色拉取状态）
  → Break S_CharacterPropertiesForCamera（解包）
  → Set CharacterPropertiesForCamera（缓存）
  → │
  └─→ Switch on E_CameraStyle（主切换）
       ├─ Style A → Activate Camera Rig X
       ├─ Style B → Activate Camera Rig Y
       ├─ Style C → Sequence
       │             ├─ Switch on E_Stance
       │             │   ├─ Stand → Switch on E_CameraMode → Activate Camera Rig
       │             │   └─ Crouch → Switch on E_CameraMode → Activate Camera Rig
       │             └─ Switch on E_CameraMode → Activate Camera Rig
       ├─ Style D → Activate Camera Rig
       ├─ Style E → Activate Camera Rig
       ├─ Style F → Activate Camera Rig
       └─ Style G → Activate Camera Rig
```

**核心逻辑：** 根据 `E_CameraStyle × E_Stance × E_CameraMode` 三维矩阵选择 `Activate Camera Rig`。

### 1.4 角色侧相机数据

从 CBP 的 CDO 提取的相机相关属性：

| 属性 | 值 | 说明 |
|------|-----|------|
| `Camera(NotUsedByDefault)` | null | 被禁用的默认相机 |
| `GameplayCamera` | null | UE5 GameplayCamera 组件 |
| `SpringArm` | null | 弹簧臂组件 |
| `CameraStyle` | NewEnumerator1 | 相机风格 |
| `MovementStickMode` | NewEnumerator0 | 摇杆模式 |
| `StrafeSpeedMapCurve` | `/Game/Blueprints/Data/Curve_StrafeSpeedMap` | 横移速度映射曲线 |
| `bUseControllerRotationYaw` | false | 不使用控制器 Yaw |
| `bOrientRotationToMovement` | false | 不朝向移动方向 |

---

## 二、C++ 转换方案

### 2.1 设计原则

相机系统使用 UE5 的 **Gameplay Camera System**（非传统 SpringArm + Camera）。

该系统的核心概念：
- **CameraComponent** — UCameraComponent 的子类，定义相机的 FOV/后处理等
- **CameraRig** — 定义相机在角色空间的位置/旋转规则
- **CameraDirectorEvaluator** — 根据角色状态选择激活哪个 Rig

### 2.2 需要新增的文件

| 文件 | 操作 | 说明 |
|------|------|------|
| `Camera/CameraEnums.h` | 新建 | `E_CameraStyle` / `E_CameraMode` 枚举 |
| `Camera/CameraProperties.h` | 新建 | `FCharacterPropertiesForCamera` 结构体 |
| `Camera/LocomotionCameraDirector.h/.cpp` | 新建 | 替换 `CameraDirector_SandboxCharacter` |
| `Camera/BPI_LocomotionCamera.h/.cpp` | 新建 | 替换 `BPI_GameplayCamera` |
| `Character/LocomotionCharacter.h/.cpp` | 修改 | 添加 `GameplayCamera` 组件 + BPI 实现 |

### 2.3 枚举设计

```cpp
// Camera/CameraEnums.h

/** 相机风格 Camera style — 整体相机行为模式 */
UENUM()
enum class ECameraStyle : uint8
{
    ThirdPerson,    // 第三人称跟随 Third-person follow
    Aim,            // 瞄准 Aim down sights
    Strafe,         // 横移 Strafe
    Traversal,      // 越障 Traversal
    Idle,           // 待机 Idle
    Sprint,         // 冲刺 Sprint
    Crouch          // 蹲伏 Crouch
};

/** 相机模式 Camera mode — 子行为 */
UENUM()
enum class ECameraMode : uint8
{
    Default,        // 默认 Default
    Tight,          // 收紧 Tight
    Far             // 拉远 Far
};
```

### 2.4 CharacterPropertiesForCamera 结构体

```cpp
// Camera/CameraProperties.h

/** 角色属性（通过 BPI 从 Character 拉取，供 CameraDirector 使用） */
USTRUCT(BlueprintType)
struct FCharacterPropertiesForCamera
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    ECameraStyle CameraStyle = ECameraStyle::ThirdPerson;

    UPROPERTY(BlueprintReadOnly)
    ECameraMode CameraMode = ECameraMode::Default;

    UPROPERTY(BlueprintReadOnly)
    EStance Stance = EStance::Stand;

    UPROPERTY(BlueprintReadOnly)
    EGait Gait = EGait::Run;

    UPROPERTY(BlueprintReadOnly)
    bool bIsTraversing = false;

    UPROPERTY(BlueprintReadOnly)
    float Speed = 0.f;
};
```

### 2.5 BPI Interface

```cpp
// Camera/BPI_LocomotionCamera.h

UINTERFACE(BlueprintType)
class UBPI_LocomotionCamera : public UInterface
{
    GENERATED_BODY()
};

class IBPI_LocomotionCamera
{
    GENERATED_BODY()

public:
    /** 获取相机所需的角色属性 Get character properties for camera */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera")
    FCharacterPropertiesForCamera GetCharacterPropertiesForCamera() const;
};
```

### 2.6 CameraDirector

```cpp
// Camera/LocomotionCameraDirector.h

UCLASS()
class ULocomotionCameraDirector : public UBlueprintCameraDirectorEvaluator
{
    GENERATED_BODY()

protected:
    /** 角色属性缓存 Cached character properties */
    UPROPERTY()
    FCharacterPropertiesForCamera CharacterProperties;

    virtual void RunCameraDirector(UCameraComponent* CameraComponent, float DeltaTime) override;

private:
    /** 根据 CameraStyle + Stance + CameraMode 激活对应的 Camera Rig */
    void EvaluateCameraRig(UCameraComponent* CameraComponent);

    /** 横移速度映射曲线 Strafe speed mapping curve */
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    TObjectPtr<UCurveFloat> StrafeSpeedMapCurve;
};
```

### 2.7 LocomotionCharacter 相机改造

当前 `ALocomotionCharacter` 有 `CameraBoom` + `FollowCamera`（传统 SpringArm + Camera）。
需要改为 UE5 Gameplay Camera System：

```cpp
// 替换：
//   CameraBoom (USpringArmComponent) + FollowCamera (UCharacterCameraComponent)
// 为：
//   GameplayCamera (UCameraComponent 子类)

UPROPERTY(VisibleAnywhere, Category = "Camera")
TObjectPtr<UCameraComponent> GameplayCamera;
```

---

## 三、实施优先级

| 优先级 | 内容 | 复杂度 |
|--------|------|--------|
| 1 | `CameraEnums.h` + `CameraProperties.h` | 低 |
| 2 | `BPI_LocomotionCamera` Interface | 低 |
| 3 | `LocomotionCharacter` 实现 BPI + 添加 `GameplayCamera` | 中 |
| 4 | `LocomotionCameraDirector` — 核心调度逻辑 | 高 |
| 5 | 配置 `StrafeSpeedMapCurve` | 低 |
| 6 | 替换当前 SpringArm+FollowCamera | 中 |

---

## 四、网络考虑

- CameraDirector 运行在**客户端**（纯表现层，不复制）
- `CharacterPropertiesForCamera` 从 StateComponent 读取（已复制）
- `CameraStyle` / `CameraMode` 可本地计算，不需网络传输
- `GameplayCamera` 的激活/切换是纯客户端操作
