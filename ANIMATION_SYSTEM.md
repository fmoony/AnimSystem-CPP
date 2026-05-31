# GameAnimationSample 动画系统架构文档

> UE 5.6 | PoseSearch 运动匹配 | 纯蓝图 → C++ 复刻参考

---

## 一、总体架构：四层数据管线

```
输入层 → 角色状态层 → 动画选择层 → 动画输出层
(EnhancedInput)  (CBP_SandboxCharacter)  (Chooser + PoseSearch)  (ABP_SandboxCharacter + Warping)
```

---

## 二、插件栈

| 插件 | 用途 |
|------|------|
| `PoseSearch` | 运动匹配引擎 — 在数据库中搜索最佳动画帧 |
| `Chooser` | 决策表系统 — 根据角色状态选择 PoseSearchDatabase |
| `AnimationWarping` | Root Motion 修正 — Orientation / Stride / Slope |
| `AnimationLocomotionLibrary` | 移动混合辅助曲线 |
| `MotionWarping` | 越障时精确同步角色到目标点 |
| `Mover` + `NetworkPrediction` | 网络化角色移动 |
| `RigLogic` + `LiveLink` + `LiveLinkControlRig` | MetaHuman 面部 |

---

## 三、输入层 (Enhanced Input)

| 输入动作 | 触发键 | 作用 |
|----------|--------|------|
| `IA_Move` | WASD / 左摇杆 | 移动输入（角色空间） |
| `IA_Look` | 鼠标 | 相机朝向 |
| `IA_Look_Gamepad` | 右摇杆 | 手柄相机 |
| `IA_Jump` | 空格 | 跳跃 |
| `IA_Sprint` | Shift | 冲刺 |
| `IA_Walk` | Ctrl | 强制行走 |
| `IA_Crouch` | C | 蹲伏 |
| `IA_Strafe` | — | 瞄准模式（朝向=视角） |
| `IA_Aim` | 右键 | 瞄准（收紧相机） |
| `IA_Interact` | E | 交互 |
| `IA_Traverse` | — | 越障触发 |
| `IA_NextCharacter` | — | 切换角色 |
| `IA_Move_WorldSpace` | — | 世界空间移动（备选） |

---

## 四、状态枚举

| 枚举 | 取值 | 含义 |
|------|------|------|
| `E_Gait` | Walk / Run / Sprint | 移动速度档位 |
| `E_Stance` | Stand / Crouch | 站立/蹲伏 |
| `E_MovementState` | Idle / Locomotion / InAir / Traversal | 宏观运动状态 |
| `E_MovementDirection` | F/B/L/R/FL/FR/BL/BR/LL/LR/RL/RR | 12 方向 |
| `E_RotationMode` | VelocityDir / LookingDir | 旋转参考 |
| `E_MovementMode` | OnGround / InAir | UE 原生移动模式 |
| `E_TraversalActionType` | Vault / Mantle / Hurdle / Climb / Catch | 越障类型 |

### 状态机流程

```
                       ┌───────────┐
            ┌─────────→│   Idle    │←─────────┐
            │          └─────┬─────┘          │
            │                │ 速度>阈值       │
            │          ┌─────▼─────┐          │
            │          │Locomotion │          │
            │          └──┬───┬───┘          │
            │    跳跃     │   │  越障触发     │
            │   ┌────────┘   └────────┐      │
            │   ▼                     ▼      │
            │ ┌──────┐           ┌──────────┐│
            │ │InAir │           │Traversal ├┘
            │ └──┬───┘           └────┬─────┘
            │    │  落地              │ 动作结束
            └────┘  ◄────────────────┘
```

### Gait 决策

```
IA_Sprint 按下 → Gait = Sprint
IA_Walk 按下   → Gait = Walk
默认           → 根据速度阈值自动在 Walk/Run 间切换
```

---

## 五、PoseSearch 运动匹配

### Chooser 表级联

```
CHT_PoseSearchDatabases (顶级)
  ├── Idle       → PSD_Dense_Stand_Idles
  ├── Locomotion → CHT_PoseSearchDatabases_Dense
  │   ├── Stand Walk   → PSD_Dense_Stand_Walk_{Loops/Starts/Stops/Pivots}
  │   ├── Stand Run    → PSD_Dense_Stand_Run_{Loops/Starts/Stops/Pivots}
  │   ├── Stand Sprint → PSD_Dense_Stand_Sprint_{Loops/Starts/Stops/Pivots}
  │   └── Crouch Walk  → PSD_Dense_Crouch_Walk_{Loops/Starts/Stops/Pivots}
  ├── InAir      → PSD_Dense_Jumps / _Far / _FromTraversal
  └── Traversal  → PSD_Traversal
```

### 数据库按状态细分策略

- **Loops** — 匀速移动循环
- **Starts** — Idle → 移动加速段
- **Stops** — 移动减速 → Idle
- **Pivots** — 移动中改变方向（急转）

### Pose Search Schema 特征

| Schema | 特征 |
|--------|------|
| `PSS_Default` | 轨迹位置/朝向/速度 + 双脚位置 + 骨盆速度 |
| `PSS_Idle` | 轻量特征（小幅度运动） |
| `PSS_Jump` | 垂直轨迹 + 起跳/落地检测 |
| `PSS_Stop` | 停止特异性（减速曲线） |
| `PSS_Traversal` | 越障特异性 |

### 每帧工作原理

```
1. ABP 计算未来轨迹 (Trajectory) — 预测未来 ~1s 位置/朝向
2. Chooser Table 根据 E_MovementState/E_Gait/E_Stance 选择 PoseSearchDatabase
3. PoseSearch 节点在数据库中搜索与当前姿态+轨迹最匹配的帧
4. 返回最佳匹配动画姿势 + 过渡时间
5. 通过 Inertialization 平滑过渡到新动画
```

---

## 六、动画命名体系

### 通用模板

```
M_Neutral_{Stance}_{Gait}_{Shape}_{FromDir}_{ToDir}_{Foot}
```

### 12 方向编码

```
  BL  FL
   \  /
B — + — F      每 30° 一个方向
   /  \
  BR  FR
       + LL / LR / RL / RR (纯横向)
```

### Shape 分级（按转向角度）

| Shape | 角度 | 示例 | 用途 |
|-------|------|------|------|
| Box | ≤45° | F→FL | 轻转向 |
| Diamond | 45°–90° | F→L | 中转向 |
| Hourglass | 90°–135° | F→BL | 大转向 |
| Prism | ≤22.5° | F→LL | 微调 |
| Arc | 弧形 | Arc_Small_L | 曲线移动 |
| Circle_Strafe | 全圆 | Circle_Strafe_L | 圆形横移 |

### Foot 后缀

- `_Lfoot` / `_Rfoot` — 领先脚标记，用于脚步 IK 匹配和相位对齐

---

## 七、动画过渡体系

### Gait 间过渡

| 过渡 | 动画前缀 | 位置 |
|------|---------|------|
| Walk → Run | `M_Neutral_Transition_Walk_to_Run_` | `Run/` |
| Run → Walk | `M_Neutral_Transition_Run_to_Walk_` | `Walk/` |
| Run → Sprint | `M_Neutral_Transition_Run_to_Sprint_` | `Sprint/` |
| Sprint → Run | `M_Neutral_Transition_Sprint_to_Run_` | `Run/` |
| Stand ↔ Crouch | `M_Neutral_Transition_Stand_to_Crouch_` | `Crouch/` |

### 跳跃落地衔接

按着陆时的 Gait + 冲击力分类：

```
M_Neutral_Jump_Land_{Idle/Walk/Run/Sprint}_{Heavy/Light}
```

落地后无缝衔接对应 Gait 的 Locomotion 数据库。

---

## 八、越障系统 (Traversal)

### 触发流程

```
IA_Traverse 按下
  → 角色前方球形扫描 (Traversable 通道)
  → 检测障碍物几何 (高度/深度/宽度)
  → 填充 S_TraversalCheckInputs
  → 判定 E_TraversalActionType (Vault/Mantle/Hurdle/Climb)
  → CHT_TraversalAnims 选择具体动画
  → MotionWarping 同步 Root Motion 到目标点
  → 播完 Blend 回 Locomotion Loop
```

### 动画组织

| 类型 | 普通动画 | Additive 动画 | 数量 |
|------|---------|---------------|------|
| Vault | `M_Neutral_Traversal_Vault_*` | `AM_M_Neutral_Traversal_Vault_*` | 3+3 |
| Mantle | `M_Neutral_Traversal_Mantle_*` | `AM_M_Neutral_Traversal_Mantle_*` | 6+6 |
| Hurdle | `M_Neutral_Traversal_Hurdle_*` | `AM_M_Neutral_Traversal_Hurdle_*` | 10+10 |
| Climb | `M_Neutral_Traversal_Climb_*` | `AM_M_Neutral_Traversal_Climb_*` | 6+6 |

---

## 九、Animation Warping 修正层

在 PoseSearch 输出后叠加：

| Warp 类型 | 功能 |
|-----------|------|
| **Orientation Warping** | 扭转脊柱修正朝向 — 动画直线前进，实际移动方向可能不同 |
| **Stride Warping** | 拉长/缩短步幅 — 匹配实际速度与动画速度的差异 |
| **Slope Warping** | 坡度适配 — 上下坡 IK 修正防穿地/浮空 |

Warp Alpha 由 Animation Modifiers 动态计算：
- `AM_OrientationWarpingAlpha`
- `AM_RateWarpingAlpha`

---

## 十、Foley 音效系统

### 架构：GameplayTags 驱动

```
动画帧 BP_AnimNotify_FoleyEvent
  → 按 Gait + Direction 发送 GameplayTag
  → CBP_SandboxCharacter 监听 GameplayTag
  → 播放对应音效
```

### GameplayTags 体系

```
Foley.Event.Walk / WalkBackwds
Foley.Event.Run / RunBackwds / RunStrafe
Foley.Event.Jump / Land
Foley.Event.Scuff / ScuffPivot / ScuffWall
Foley.Event.Handplant / Tumble
```

### Notify 蓝图资产

| Notify | 用途 |
|--------|------|
| `BP_AnimNotify_FoleyEvent` | 基类 |
| `BP_AnimNotify_FoleyEvent_Walk_L/R` | 走路脚步声 |
| `BP_AnimNotify_FoleyEvent_Run_L/R` | 跑步脚步声 |
| `BP_AnimNotify_FoleyEvent_Jump` | 跳跃音效 |
| `BP_AnimNotify_FoleyEvent_Land` | 落地音效 |
| `BP_AnimNotify_FoleyEvent_Scuff_L/R` | 擦地音效 |
| `BP_AnimNotify_FoleyEvent_Handplant_L/R` | 手支撑音效 |
| `BP_NotifyState_EarlyTransition` | 提前过渡状态 |
| `BP_NotifyState_MontageBlendOut` | 蒙太奇混合退出 |

---

## 十一、角色重定向管线

所有角色共享 **UEFN Mannequin** 源动画，通过 IK Rig 重定向：

| 目标角色 | IK Rig | Retarget Pose |
|----------|--------|---------------|
| UE5 Manny/Quinn | `IK_UE5_Mannequin_Retarget` | `RTG_UEFN_to_UE5_Mannequin` |
| UE4 Mannequin | `IK_UE4_Mannequin_Retarget` | `RTG_UEFN_to_UE4_Mannequin` |
| Echo | `IK_Echo_Retarget` | `RTG_UEFN_to_Echo` |
| TwinBlast | `IK_TwinBlast_Retarget` | `RTG_UEFN_to_TwinBlast` |
| MetaHuman | `IK_Metahuman_Retarget` | `RTG_UEFN_to_Metahuman_nrw/ovw` |

后处理 ABP 层：`ABP_*_PostProcess` — 每个角色独立的后处理动画蓝图。

---

## 十二、Control Rig 资产

| Control Rig | 用途 |
|-------------|------|
| `CR_Mannequin_Body` | 主骨架身体控制 |
| `CR_Mannequin_Procedural` | 程序化次级运动 |
| `CR_Echo_Helpers` / `CR_Echo_Twist` | Echo 姿态修正 |
| `MetaHuman_ControlRig` | MetaHuman 全身 CR |
| `Face_ControlBoard_CtrlRig` | 面部控制板 |
| `HeadMovementIK_Proc_CtrlRig` | 头部 IK 程序化 |
| `Neck_CtrlRig` | 颈部专用 |

---

## 十三、Animation Modifiers

| Modifier | 功能 |
|----------|------|
| `AM_BakePhaseCurveFromFootstepNotifies` | 从脚步通知烘焙相位曲线 |
| `AM_DistanceFromLedge` | 距离边缘检测 |
| `AM_FootSpeed_L/R` | 左右脚速度计算 |
| `AM_MoveData_Speed` | 移动速度数据 |
| `AM_OrientationWarpingAlpha` | 朝向 Warp 权重 |
| `AM_RateWarpingAlpha` | 步幅 Warp 权重 |

---

## 十四、关键 BlendSpace & AimOffset

| 资产 | 类型 | 路径 |
|------|------|------|
| `BS1D_Additive_Lean_Run` | 1D BlendSpace | `Animations/Run/` |
| `BS_Neutral_AO_Stand` | AimOffset | `Animations/AimOffset/` |
| `AO_Blend_Curve` | 曲线 | `Animations/AimOffset/` |
| `Curve_StrafeSpeedMap` | Float Curve | `Blueprints/Data/` |

---

## 十五、完整帧循环

```
1. [输入] EnhancedInput → IA_Move / Sprint / Crouch / Jump / Traverse
2. [状态] CBP_SandboxCharacter 更新 CharacterInputState
   (E_Gait, E_Stance, E_MovementState, E_MovementDirection, E_RotationMode)
3. [轨迹] ABP_SandboxCharacter 计算 Trajectory（预测未来路径）
4. [选择] CHT_PoseSearchDatabases Chooser 根据状态选择 PoseSearchDatabase
5. [搜索] PoseSearch 节点在选定数据库中匹配最佳动画帧
6. [过渡] Inertialization 混合节点平滑衔接新旧动画
7. [修正] Orientation + Stride + Slope Warping 修正 Root Motion
8. [输出] 最终动画姿势 → 骨骼网格渲染
9. [音效] Foley Notify → GameplayTags → 音效播放
```

---

## C++ 复刻路线

| 优先级 | 模块 | 内容 |
|--------|------|------|
| 1 | 数据结构层 | 枚举 + 结构体 (E_Gait / E_Stance / CharacterInputState 等) |
| 2 | 角色移动层 | Mover 组件 + 移动状态管理 |
| 3 | PoseSearch 集成 | Schema + Database 配置 C++ 层 |
| 4 | 动画蓝图 C++ 基类 | UAnimInstance 子类，PoseSearch 节点 + Warping + AimOffset |
| 5 | 越障系统 | 检测 Trace + Chooser Table 逻辑 |
| 6 | Foley 系统 | UAnimNotify C++ 子类 |
| 7 | 重定向管线 | IKRig + RetargetPose 代码配置 |
| 8 | Control Rig | 动态生成 CR |

---

> 项目路径：`Content/Blueprints/` `Content/Characters/UEFN_Mannequin/Animations/`
> 数据资产：`.uasset` (二进制)，通过 UE Editor 查看
> MCP Bridge：`Plugins/McpAutomationBridge/` (v0.1.4 by ChiR24)
