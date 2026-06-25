// Copyright 2024 Locomotion System. All Rights Reserved.
// 移动角色玩家控制器实现 Locomotion player controller implementation

#include "Game/LocomotionPlayerController.h"
#include "Character/LocomotionCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"

ALocomotionPlayerController::ALocomotionPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	// 输入模式：仅游戏，不显示鼠标 Game-only input, no mouse cursor
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableTouchEvents = true;
}

void ALocomotionPlayerController::BeginPlay()
{
	Super::BeginPlay();
	BindActions();
}

void ALocomotionPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void ALocomotionPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALocomotionPlayerController, CurrentCharacterIndex);
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

// ── 客户端按下切换 → RPC 到服务端 ────────────────────

void ALocomotionPlayerController::OnNextCharacter()
{
	ServerSwitchCharacter();
}

// ── Tick 每帧缓存控制旋转（仅本地）───────────────────

void ALocomotionPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 仅本地缓存控制旋转 Local-only rotation cache
	if (IsLocalController())
	{
		CachedControlRotation = GetControlRotation();
	}
}

// ── ServerSwitchCharacter 服务端权威实现 ──────────────

void ALocomotionPlayerController::ServerSwitchCharacter_Implementation()
{
	if (Characters.Num() == 0) return;

	// 保存当前 Pawn 的变换 Save current pawn transform
	APawn* PreviousPawn = GetPawn();
	const FTransform SpawnTransform = PreviousPawn
		? PreviousPawn->GetActorTransform()
		: FTransform::Identity;

	// 循环索引并复制到客户端 Wrap index and replicate to client
	CurrentCharacterIndex = (CurrentCharacterIndex + 1) % Characters.Num();

	// 销毁旧 Pawn Destroy old pawn（服务端权威）
	if (PreviousPawn)
	{
		PreviousPawn->Destroy();
	}

	// 在服务端生成新角色 Spawn new character on server
	ALocomotionCharacter* NewChar = GetWorld()->SpawnActor<ALocomotionCharacter>(
		Characters[CurrentCharacterIndex],
		SpawnTransform.GetLocation(),
		SpawnTransform.GetRotation().Rotator());

	if (NewChar)
	{
		// 服务端 Possess — 自动复制到客户端 Server possess — auto-replicates to client
		Possess(NewChar);
	}
}

// ── OnPossess 客户端收到新 Pawn → 恢复旋转 + 平滑视角 ─

void ALocomotionPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);

	// 客户端表现：恢复控制旋转 + 平滑视角过渡
	// Client-side: restore rotation + smooth view blend
	if (IsLocalController() && NewPawn)
	{
		SetControlRotation(CachedControlRotation);
		SetViewTargetWithBlend(NewPawn, 0.2f);
	}
}
