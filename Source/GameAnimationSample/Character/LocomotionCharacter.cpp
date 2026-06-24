// Copyright 2024 Locomotion System. All Rights Reserved.

#include "Character/LocomotionCharacter.h"
#include "Character/Core/CharacterEventBus.h"
#include "Character/State/CharacterStateComponent.h"
#include "Character/Input/CharacterInputComponent.h"

ALocomotionCharacter::ALocomotionCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// ── 创建 EventBus（零依赖，最先创建）───────────────
	EventBus = CreateDefaultSubobject<UCharacterEventBus>(TEXT("EventBus"));

	// ── 创建 StateComponent（依赖 Core）────────────────
	StateComponent = CreateDefaultSubobject<UCharacterStateComponent>(TEXT("StateComponent"));

	// ── 创建 CharInputComponent（依赖 StateComponent）──────
	CharInputComponent = CreateDefaultSubobject<UCharacterInputComponent>(TEXT("CharInputComponent"));
	CharInputComponent->bAutoActivate = true;
}

void ALocomotionCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ALocomotionCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// EnhancedInput 绑定由 UCharacterInputComponent::BeginPlay 处理
	// （它需要访问 EnhancedInputSubsystem，在 SetupPlayerInputComponent 之后）
}
