// Copyright 2024 Locomotion System. All Rights Reserved.
// 移动角色实现 Locomotion character implementation

#include "Character/LocomotionCharacter.h"
#include "Character/Core/CharacterEventBus.h"
#include "Character/State/CharacterStateComponent.h"
#include "Character/Input/CharacterInputComponent.h"
#include "Character/Movement/LocomotionMoverComponent.h"

ALocomotionCharacter::ALocomotionCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// EventBus — 零依赖，最先创建 Zero dependencies, created first
	EventBus = CreateDefaultSubobject<UCharacterEventBus>(TEXT("EventBus"));

	// StateComponent — 依赖 Core Depends on Core
	StateComponent = CreateDefaultSubobject<UCharacterStateComponent>(TEXT("StateComponent"));

	// CharInputComponent — 依赖 StateComponent Depends on StateComponent
	CharInputComponent = CreateDefaultSubobject<UCharacterInputComponent>(TEXT("CharInputComponent"));
	CharInputComponent->bAutoActivate = true;

	// MoverComponent — 依赖 StateComponent Depends on StateComponent
	MoverComponent = CreateDefaultSubobject<ULocomotionMoverComponent>(TEXT("MoverComponent"));
	MoverComponent->bAutoActivate = true;
}

void ALocomotionCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ALocomotionCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// EnhancedInput 绑定由 UCharacterInputComponent::BeginPlay 处理
	// EnhancedInput binding handled by UCharacterInputComponent::BeginPlay
}
