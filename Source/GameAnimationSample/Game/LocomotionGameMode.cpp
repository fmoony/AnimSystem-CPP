// Copyright 2024 Locomotion System. All Rights Reserved.
// 移动游戏模式实现 Locomotion game mode implementation

#include "Game/LocomotionGameMode.h"
#include "Character/LocomotionCharacter.h"
#include "Game/LocomotionPlayerController.h"

ALocomotionGameMode::ALocomotionGameMode()
{
	// 设置默认 Pawn 和 Controller 类 Set default Pawn and Controller classes
	DefaultPawnClass = ALocomotionCharacter::StaticClass();
	PlayerControllerClass = ALocomotionPlayerController::StaticClass();
}
