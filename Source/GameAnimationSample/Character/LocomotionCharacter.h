// Copyright 2024 Locomotion System. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LocomotionCharacter.generated.h"

class UCharacterEventBus;
class UCharacterStateComponent;
class UCharacterInputComponent;

// ─────────────────────────────────────────────────────
// LocomotionCharacter — 组件组装根
//
// 在构造函数中创建 EventBus + StateComponent + InputComponent。
// MoverComponent 使用标准 UCharacterMovementComponent（后续替换为 UMoverComponent）。
// ─────────────────────────────────────────────────────

UCLASS()
class ALocomotionCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ALocomotionCharacter();

	// ── 组件访问器 ────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	UCharacterEventBus* GetEventBus() const { return EventBus; }

	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	UCharacterStateComponent* GetStateComponent() const { return StateComponent; }

	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	UCharacterInputComponent* GetCharInputComponent() const { return CharInputComponent; }

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
};
