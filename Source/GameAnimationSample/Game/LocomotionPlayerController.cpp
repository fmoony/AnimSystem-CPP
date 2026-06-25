// Copyright 2024 Locomotion System. All Rights Reserved.
// 移动角色玩家控制器实现 Locomotion player controller implementation

#include "Game/LocomotionPlayerController.h"
#include "Character/LocomotionCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"

ALocomotionPlayerController::ALocomotionPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	// 输入模式：仅游戏，不显示鼠标 Game-only input, no mouse cursor
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableTouchEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CurrentMouseCursor = EMouseCursor::None;
}

void ALocomotionPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 绑定 IA_NextCharacter（Controller 层绑定，不随 Pawn 销毁）
	// Bind IA_NextCharacter at Controller level — survives Pawn destruction
	BindActions();
}

void ALocomotionPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

// ── BindActions 绑定切换角色输入 ──────────────────────

void ALocomotionPlayerController::BindActions()
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	if (Subsystem && InputMappingContext)
	{
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInput && IA_NextCharacter)
	{
		EnhancedInput->BindAction(IA_NextCharacter, ETriggerEvent::Started,
			this, &ALocomotionPlayerController::OnNextCharacter);
	}
}

void ALocomotionPlayerController::OnNextCharacter()
{
	SwitchToNextCharacter();
}

// ── Tick 每帧缓存控制旋转 ────────────────────────────

void ALocomotionPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 缓存控制旋转，角色切换时恢复 Cache control rotation for restore on switch
	CachedControlRotation = GetControlRotation();
}

// ── SwitchToNextCharacter 角色切换核心逻辑 ────────────

void ALocomotionPlayerController::SwitchToNextCharacter()
{
	if (Characters.Num() == 0) return;

	// 保存当前 Pawn 的变换 Save current pawn transform
	APawn* PreviousPawn = GetPawn();
	const FTransform SpawnTransform = PreviousPawn
		? PreviousPawn->GetActorTransform()
		: FTransform::Identity;

	// 循环索引 Wrap index
	CurrentCharacterIndex = (CurrentCharacterIndex + 1) % Characters.Num();

	// 销毁旧 Pawn Destroy old pawn
	if (PreviousPawn)
	{
		PreviousPawn->Destroy();
	}

	// 生成新角色 Spawn new character
	ALocomotionCharacter* NewChar = GetWorld()->SpawnActor<ALocomotionCharacter>(
		Characters[CurrentCharacterIndex],
		SpawnTransform.GetLocation(),
		SpawnTransform.GetRotation().Rotator());

	if (NewChar)
	{
		// 控制新角色 Possess new character
		Possess(NewChar);

		// 恢复控制旋转 Restore control rotation
		SetControlRotation(CachedControlRotation);

		// 平滑切换视角 Set view target with blend for smooth transition
		SetViewTargetWithBlend(NewChar, 0.2f);
	}
}
