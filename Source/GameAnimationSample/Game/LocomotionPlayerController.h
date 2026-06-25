// Copyright 2024 Locomotion System. All Rights Reserved.
// 移动角色玩家控制器 Locomotion player controller — character switching + camera rotation

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LocomotionPlayerController.generated.h"

class ALocomotionCharacter;
class UInputAction;
class UInputMappingContext;

/**
 * 移动角色玩家控制器 Locomotion player controller
 *
 * 职责：角色切换（Characters 数组 + IA_NextCharacter）、控制旋转缓存、输入模式配置。
 * 不处理移动输入（由 ALocomotionCharacter::UCharacterInputComponent 处理）。
 */
UCLASS()
class ALocomotionPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALocomotionPlayerController();

	// ── Character Switching 角色切换 ────────────────────

	/** 可切换的角色列表 Switchable character classes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	TArray<TSubclassOf<ALocomotionCharacter>> Characters;

	/** 切换到下一个角色 Switch to next character */
	UFUNCTION(BlueprintCallable, Category = "Character")
	void SwitchToNextCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

private:
	// ── Input 输入绑定 ─────────────────────────────────

	void BindActions();
	void OnNextCharacter();

	// ── Config 输入配置 ────────────────────────────────

	/** 输入映射上下文 Input mapping context */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> InputMappingContext;

	/** 切换角色动作 Next character action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_NextCharacter;

	// ── State 状态 ─────────────────────────────────────

	/** 当前角色索引 Current character index */
	UPROPERTY()
	int32 CurrentCharacterIndex = 0;

	/** 缓存的控制旋转 Cached control rotation（角色切换时恢复） */
	FRotator CachedControlRotation;
};
