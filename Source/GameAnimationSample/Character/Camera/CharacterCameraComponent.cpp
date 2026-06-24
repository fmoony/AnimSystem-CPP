// Copyright 2024 Locomotion System. All Rights Reserved.
// 角色相机组件实现 Character camera component implementation

#include "Character/Camera/CharacterCameraComponent.h"
#include "Character/State/CharacterStateComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Character.h"

UCharacterCameraComponent::UCharacterCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCharacterCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	// 查找 SpringArm Find SpringArm component
	AActor* Owner = GetOwner();
	if (Owner)
	{
		SpringArm = Owner->FindComponentByClass<USpringArmComponent>();
		StateComponent = Owner->FindComponentByClass<UCharacterStateComponent>();
	}

	if (StateComponent)
	{
		// 订阅状态变化 → 调整 FOV 和 SpringArm
		StateChangedHandle = StateComponent->OnStateChanged.AddWeakLambda(this,
			[this](const FCharacterStateSnapshot& Snapshot)
			{
				CurrentGait = Snapshot.Gait;
				// 根据步态和旋转模式设定目标 FOV
				// Set target FOV based on gait and rotation mode
				if (Snapshot.RotationMode == ERotationMode::LookingDirection)
				{
					TargetFOV = AimFOV;
				}
				else if (Snapshot.Gait == EGait::Sprint)
				{
					TargetFOV = SprintFOV;
				}
				else
				{
					TargetFOV = NormalFOV;
				}

				// 冲刺时拉远弹簧臂 Pull spring arm back during sprint
				TargetArmLength = (Snapshot.Gait == EGait::Sprint)
					? SprintArmLength : NormalArmLength;
			});
	}

	// 初始化 Init defaults
	TargetFOV = NormalFOV;
	TargetArmLength = NormalArmLength;
	SetFieldOfView(NormalFOV);
}

void UCharacterCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// FOV 平滑插值 Smooth FOV interpolation
	const float CurrentFOV = FieldOfView;
	const float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, FOVInterpSpeed);
	SetFieldOfView(NewFOV);

	// SpringArm 长度平滑插值 Smooth spring arm length interpolation
	if (SpringArm)
	{
		const float NewLength = FMath::FInterpTo(
			SpringArm->TargetArmLength, TargetArmLength, DeltaTime, 3.f);
		SpringArm->TargetArmLength = NewLength;
	}
}
