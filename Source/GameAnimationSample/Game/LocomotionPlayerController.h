// Copyright 2024 Locomotion System. All Rights Reserved.
// 移动角色玩家控制器 — 角色切换 + 网络同步 Locomotion player controller — character switching + network

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
 * 网络设计：
 *   客户端：缓存 ControlRotation + 发起 ServerSwitchCharacter RPC
 *   服务端：Destroy → Spawn → Possess（权威）
 *   客户端 OnPossess：恢复旋转 + 平滑视角过渡
 * 移动输入由 ALocomotionCharacter::UCharacterInputComponent 处理。
 */
UCLASS()
class ALocomotionPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALocomotionPlayerController();

	/** 可切换的角色列表 Switchable character classes（Client+Server 配置一致） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	TArray<TSubclassOf<ALocomotionCharacter>> Characters;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

	// ── Network 网络 ───────────────────────────────────

	/** 服务端执行角色切换 Server-authoritative character switch */
	UFUNCTION(Server, Reliable)
	void ServerSwitchCharacter();

	/** Possess 时客户端恢复旋转 + 视角平滑 Client-side restore rotation + view blend on possess */
	virtual void OnPossess(APawn* NewPawn) override;

private:
	// ── Input 输入 ─────────────────────────────────────

	void BindActions();
	void OnNextCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_NextCharacter;

	// ── State 状态 ─────────────────────────────────────

	/** 当前角色索引 Current character index（服务端复制到客户端） */
	UPROPERTY(Replicated)
	int32 CurrentCharacterIndex = 0;

	/** 缓存的控制器旋转 Cached control rotation（本地，不复制） */
	FRotator CachedControlRotation;
};
