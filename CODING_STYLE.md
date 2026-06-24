# 代码编写规范 Coding Style Guide

> UE 5.6 C++ | Locomotion System | Tab 缩进 | Allman 花括号 | 中英双语注释

---

## 1. 花括号换行 Allman Brace Style

函数、if/else、for、while、switch 的花括号**独占一行**。

```cpp
// 正确 Correct
void Foo()
{
    if (Condition)
    {
        // ...
    }
    else
    {
        // ...
    }
}

// 错误 Wrong
void Foo() {
    if (Condition) {
    }
}
```

**例外**：内联 getter/setter 可单行。

```cpp
FORCEINLINE int32 GetCount() const { return Count; }
```

## 2. 缩进与空白 Indentation & Whitespace

- **缩进**：Tab（UE 引擎标准），宽度等于 4 空格
- **指针/引用**：`Type* Ptr`，`const Type& Ref`

## 3. 文件头 File Header

首行版权，第二行文件职责简述（中英双语），然后 `#pragma once`。

```cpp
// Copyright 2024 Locomotion System. All Rights Reserved.
// 角色状态组件 — 服务端权威状态判定 Character state component — server-authoritative state determination

#pragma once
```

## 4. Include 顺序 Include Order

```cpp
// 1. 本文件对应头文件 Corresponding header
#include "Character/State/CharacterStateComponent.h"

// 2. 引擎头文件 Engine headers
#include "GameFramework/Character.h"

// 3. 项目内其他模块 Project modules
#include "Character/Core/CharacterEventBus.h"
```

## 5. 函数注释 Function Comments

公有函数用 `/** */`，保护/私有用 `//`。注释说**做什么 What**。

```cpp
/** 从输入数据判定当前步态 Determine gait from input data */
void UpdateState(const FCharacterInputData& Input, const FVector& Velocity, bool bIsOnGround);

// 本地预测路径 Local prediction path
void LocalPredict(const FCharacterInputData& Input);
```

## 6. UE 反射宏 UHT Macros

宏紧贴声明，中间**无空行**。注释写在宏上方。

```cpp
/** 处理角色死亡逻辑 Handle character death */
UFUNCTION(BlueprintCallable, Category = "Character")
void Die();

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speed")
float MaxWalkSpeed;
```

## 7. 函数内注释 In-Function Comments

注释解释**意图 Why**，不翻译代码 What。逻辑块前保留空行。

```cpp
// 验证控制器与输入有效性 Validate controller and input
if (GetController() == nullptr)
{
    return;
}

// 处理 90 度十字路口转弯 Handle 90-degree turn
if (bTurn)
{
    // ...
}
```

标记词全大写：

```cpp
// TODO: 需要实现可配置冷却时间 Need configurable cooldown
// FIXME: 低帧率下状态判定延迟一帧 State lag at low FPS
// HACK: 临时用 CMC 替代 Mover Temporary CMC instead of Mover
// NOTE: 此函数在 GameThread 调用 Called on GameThread only
```

## 8. 中英双语注释 Bilingual Comments

**中文在前，英文在后，空格分隔**。内容对等。

```cpp
/** 触发角色死亡逻辑 Trigger character death logic */
void Die();

// 验证控制器有效性 Validate controller
if (!Controller)
{
    return;
}

// 行尾注释 End-of-line
EMovementDirection::F,  // Forward 前
```
