// Copyright 2024 Locomotion System. All Rights Reserved.
// 输入组件 — EnhancedInput 绑定 Input component — EnhancedInput binding

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Core/CharacterInputData.h"
#include "CharacterInputComponent.generated.h"

class UInputAction;
class UInputMappingContext;
class UCharacterStateComponent;
class ACharacter;
struct FInputActionValue;

/**
 * 输入组件 Input component
 *
 * EnhancedInput → StateComp + MoverComp（直接调用，不经过 EventBus）。
 * 高频核心路径，直接调用更高效。
 */
UCLASS(ClassGroup=(Locomotion), meta=(BlueprintSpawnableComponent))
class UCharacterInputComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterInputComponent();

	// ── Config 蓝图配置 ────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Sprint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Walk;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Crouch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Strafe;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Traverse;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	// ── EnhancedInput 绑定 EnhancedInput binding ────────

	void BindActions();
	void UnbindActions();

	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnJumpStarted(const FInputActionValue& Value);
	void OnSprintStarted(const FInputActionValue& Value);
	void OnSprintCompleted(const FInputActionValue& Value);
	void OnWalkStarted(const FInputActionValue& Value);
	void OnWalkCompleted(const FInputActionValue& Value);
	void OnCrouchStarted(const FInputActionValue& Value);
	void OnStrafeStarted(const FInputActionValue& Value);
	void OnStrafeCompleted(const FInputActionValue& Value);
	void OnTraverseStarted(const FInputActionValue& Value);

	// ── Network 网络 RPC ───────────────────────────────

	/** 客户端→服务端 发送输入 Client → server send input */
	UFUNCTION(Server, Unreliable)
	void Server_SendInput(const FCharacterInputData& Input);

	// ── State 累积的输入 ────────────────────────────────

	FCharacterInputData PendingInput;

	UPROPERTY()
	TObjectPtr<UCharacterStateComponent> StateComponent;

	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;
};
