// Copyright 2024 Locomotion System. All Rights Reserved.
// 角色相机组件 Character camera component

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Character/Core/CharacterEnums.h"
#include "CharacterCameraComponent.generated.h"

class USpringArmComponent;
class UCharacterStateComponent;

/**
 * 角色相机组件 Character camera component
 *
 * 订阅 StateComponent::OnStateChanged 调整 FOV（冲刺时增大）。
 * 读取 Speed 动态调整 SpringArm 长度。
 */
UCLASS(ClassGroup=(Locomotion), meta=(BlueprintSpawnableComponent))
class UCharacterCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	UCharacterCameraComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	// ── FOV Config 视野配置 ────────────────────────────

	UPROPERTY(EditAnywhere, Category = "Camera|FOV")
	float NormalFOV = 90.f;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV")
	float SprintFOV = 100.f;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV")
	float AimFOV = 60.f;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV")
	float FOVInterpSpeed = 5.f;

	// ── SpringArm Config 弹簧臂配置 ────────────────────

	UPROPERTY(EditAnywhere, Category = "Camera|SpringArm")
	float NormalArmLength = 300.f;

	UPROPERTY(EditAnywhere, Category = "Camera|SpringArm")
	float SprintArmLength = 350.f;

	// ── State 状态 ─────────────────────────────────────

	float TargetFOV = 90.f;
	float TargetArmLength = 300.f;
	EGait CurrentGait = EGait::Run;

	// ── Cached References 缓存引用 ──────────────────────

	UPROPERTY()
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY()
	TObjectPtr<UCharacterStateComponent> StateComponent;

	FDelegateHandle StateChangedHandle;
};
