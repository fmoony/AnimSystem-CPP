// Copyright 2024 Locomotion System. All Rights Reserved.
// 移动角色 — 组件组装根 Locomotion character — component assembly root

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LocomotionCharacter.generated.h"

class UCharacterEventBus;
class UCharacterStateComponent;
class UCharacterInputComponent;
class ULocomotionMoverComponent;
class UTraversalComponent;
class UFoleyComponent;
class USpringArmComponent;
class UCharacterCameraComponent;

/**
 * 移动角色 Locomotion character
 *
 * 在构造函数中创建 EventBus + StateComponent + InputComponent + MoverComponent。
 * 各组件通过 EventBus（瞬时事件）和 OnStateChanged（状态变化）通信。
 */
UCLASS()
class ALocomotionCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ALocomotionCharacter();

	// ── Component Accessors 组件访问器 ──────────────────

	/** 获取事件总线 Get the event bus */
	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	UCharacterEventBus* GetEventBus() const { return EventBus; }

	/** 获取状态组件 Get the state component */
	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	UCharacterStateComponent* GetStateComponent() const { return StateComponent; }

	/** 获取输入组件 Get the input component */
	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	UCharacterInputComponent* GetCharInputComponent() const { return CharInputComponent; }

	/** 获取移动组件 Get the mover component */
	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	ULocomotionMoverComponent* GetMoverComponent() const { return MoverComponent; }

	/** 获取越障组件 Get the traversal component */
	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	UTraversalComponent* GetTraversalComponent() const { return TraversalComponent; }

	/** 获取音效组件 Get the foley component */
	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	UFoleyComponent* GetFoleyComponent() const { return FoleyComponent; }

	/** 获取弹簧臂 Get the camera boom */
	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** 获取跟随相机 Get the follow camera */
	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	UCharacterCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Locomotion")
	TObjectPtr<UCharacterEventBus> EventBus;

	UPROPERTY(VisibleAnywhere, Category = "Locomotion")
	TObjectPtr<UCharacterStateComponent> StateComponent;

	UPROPERTY(VisibleAnywhere, Category = "Locomotion")
	TObjectPtr<UCharacterInputComponent> CharInputComponent;

	UPROPERTY(VisibleAnywhere, Category = "Locomotion")
	TObjectPtr<ULocomotionMoverComponent> MoverComponent;

	UPROPERTY(VisibleAnywhere, Category = "Locomotion")
	TObjectPtr<UTraversalComponent> TraversalComponent;

	UPROPERTY(VisibleAnywhere, Category = "Locomotion")
	TObjectPtr<UFoleyComponent> FoleyComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterCameraComponent> FollowCamera;
};
