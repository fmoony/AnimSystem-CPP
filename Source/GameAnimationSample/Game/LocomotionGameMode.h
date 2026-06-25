// Copyright 2024 Locomotion System. All Rights Reserved.
// 移动游戏模式 Locomotion game mode

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LocomotionGameMode.generated.h"

/**
 * 移动游戏模式 Locomotion game mode
 *
 * 设置 DefaultPawnClass 和 PlayerControllerClass。
 * 其余使用引擎默认（GameState / PlayerState / HUD）。
 */
UCLASS()
class ALocomotionGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALocomotionGameMode();
};
