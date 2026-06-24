// Copyright 2024 Locomotion System. All Rights Reserved.
// 输入组件实现 Input component implementation

#include "Character/Input/CharacterInputComponent.h"
#include "Character/State/CharacterStateComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/LocalPlayer.h"

UCharacterInputComponent::UCharacterInputComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UCharacterInputComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	StateComponent = GetOwner()->FindComponentByClass<UCharacterStateComponent>();

	BindActions();
}

void UCharacterInputComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindActions();
	Super::EndPlay(EndPlayReason);
}

// ── BindActions 绑定 Enhanced Input ───────────────────

void UCharacterInputComponent::BindActions()
{
	APlayerController* PC = nullptr;
	if (OwnerCharacter)
	{
		PC = Cast<APlayerController>(OwnerCharacter->GetController());
	}
	if (!PC) return;

	// 添加 Mapping Context Add mapping context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		PC->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		if (InputMappingContext)
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}

	// 绑定 Actions Bind actions
	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PC->InputComponent);
	if (!EnhancedInput) return;

	if (IA_Move)    EnhancedInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &UCharacterInputComponent::OnMove);
	if (IA_Look)    EnhancedInput->BindAction(IA_Look, ETriggerEvent::Triggered, this, &UCharacterInputComponent::OnLook);
	if (IA_Jump)    EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Started, this, &UCharacterInputComponent::OnJumpStarted);
	if (IA_Sprint)  EnhancedInput->BindAction(IA_Sprint, ETriggerEvent::Started, this, &UCharacterInputComponent::OnSprintStarted);
	if (IA_Sprint)  EnhancedInput->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &UCharacterInputComponent::OnSprintCompleted);
	if (IA_Walk)    EnhancedInput->BindAction(IA_Walk, ETriggerEvent::Started, this, &UCharacterInputComponent::OnWalkStarted);
	if (IA_Walk)    EnhancedInput->BindAction(IA_Walk, ETriggerEvent::Completed, this, &UCharacterInputComponent::OnWalkCompleted);
	if (IA_Crouch)  EnhancedInput->BindAction(IA_Crouch, ETriggerEvent::Started, this, &UCharacterInputComponent::OnCrouchStarted);
	if (IA_Strafe)  EnhancedInput->BindAction(IA_Strafe, ETriggerEvent::Started, this, &UCharacterInputComponent::OnStrafeStarted);
	if (IA_Strafe)  EnhancedInput->BindAction(IA_Strafe, ETriggerEvent::Completed, this, &UCharacterInputComponent::OnStrafeCompleted);
	if (IA_Traverse) EnhancedInput->BindAction(IA_Traverse, ETriggerEvent::Started, this, &UCharacterInputComponent::OnTraverseStarted);
}

void UCharacterInputComponent::UnbindActions()
{
	if (!OwnerCharacter) return;

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC) return;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		PC->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		if (InputMappingContext)
		{
			Subsystem->RemoveMappingContext(InputMappingContext);
		}
	}

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PC->InputComponent);
	if (EnhancedInput)
	{
		EnhancedInput->ClearBindingsForObject(this);
	}
}

// ── Tick 每帧读取输入 → 直接调 StateComp ──────────────

void UCharacterInputComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerCharacter || !StateComponent) return;

	// 读取移动速度 Read movement velocity
	UCharacterMovementComponent* CMC = OwnerCharacter->GetCharacterMovement();
	const FVector Velocity = CMC ? CMC->GetLastUpdateVelocity() : FVector::ZeroVector;
	const bool bOnGround = CMC ? CMC->IsMovingOnGround() : true;

	if (OwnerCharacter->GetLocalRole() == ROLE_AutonomousProxy)
	{
		// 客户端 → 发送 RPC + 本地预测 Client → send RPC + local prediction
		Server_SendInput(PendingInput);
		StateComponent->LocalPredict(PendingInput, Velocity, bOnGround);
	}
	else if (OwnerCharacter->GetLocalRole() == ROLE_Authority)
	{
		// 服务端（或单机）→ 直接权威判定 Server (or standalone) → direct authoritative
		StateComponent->UpdateState(PendingInput, Velocity, bOnGround);
	}

	// 清除瞬时输入 Reset instant inputs
	PendingInput.ResetInstants();
}

// ── Action Callbacks 输入回调 ─────────────────────────

void UCharacterInputComponent::OnMove(const FInputActionValue& Value)
{
	PendingInput.MoveInput = Value.Get<FVector2D>();
}

void UCharacterInputComponent::OnLook(const FInputActionValue& Value)
{
	PendingInput.LookInput = Value.Get<FVector2D>();
}

void UCharacterInputComponent::OnJumpStarted(const FInputActionValue& Value)
{
	PendingInput.bJumpPressed = true;
	if (OwnerCharacter)
	{
		OwnerCharacter->Jump();
	}
}

void UCharacterInputComponent::OnSprintStarted(const FInputActionValue& Value)  { PendingInput.bSprintHeld = true; }
void UCharacterInputComponent::OnSprintCompleted(const FInputActionValue& Value){ PendingInput.bSprintHeld = false; }
void UCharacterInputComponent::OnWalkStarted(const FInputActionValue& Value)    { PendingInput.bWalkHeld = true; }
void UCharacterInputComponent::OnWalkCompleted(const FInputActionValue& Value)  { PendingInput.bWalkHeld = false; }

void UCharacterInputComponent::OnCrouchStarted(const FInputActionValue& Value)
{
	PendingInput.bCrouchToggled = true;
	if (OwnerCharacter)
	{
		OwnerCharacter->Crouch();
	}
}

void UCharacterInputComponent::OnStrafeStarted(const FInputActionValue& Value)   { PendingInput.bStrafeHeld = true; }
void UCharacterInputComponent::OnStrafeCompleted(const FInputActionValue& Value) { PendingInput.bStrafeHeld = false; }

void UCharacterInputComponent::OnTraverseStarted(const FInputActionValue& Value)
{
	PendingInput.bTraversePressed = true;
}

// ── Server_SendInput RPC 客户端→服务端 ────────────────

void UCharacterInputComponent::Server_SendInput_Implementation(const FCharacterInputData& Input)
{
	if (!StateComponent) return;

	UCharacterMovementComponent* CMC = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr;
	const FVector Velocity = CMC ? CMC->GetLastUpdateVelocity() : FVector::ZeroVector;
	const bool bOnGround = CMC ? CMC->IsMovingOnGround() : true;

	StateComponent->UpdateState(Input, Velocity, bOnGround);
}
