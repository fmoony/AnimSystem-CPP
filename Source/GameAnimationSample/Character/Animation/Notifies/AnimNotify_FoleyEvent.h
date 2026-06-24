// Copyright 2024 Locomotion System. All Rights Reserved.
// 脚步音效通知 — 连接动画帧与 EventBus Footstep anim notify — bridges animation frames to EventBus

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Character/Core/CharacterEnums.h"
#include "AnimNotify_FoleyEvent.generated.h"

/**
 * 脚步音效通知 Footstep anim notify
 *
 * 挂在动画序列的脚落地帧上。Notified 时发布 FFootstepEvent 到 EventBus，
 * FoleyComponent 订阅并播放对应音效。
 */
UCLASS()
class UAnimNotify_FoleyEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	/** 哪只脚 Which foot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foley")
	EFootstepSide Side = EFootstepSide::Left;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
