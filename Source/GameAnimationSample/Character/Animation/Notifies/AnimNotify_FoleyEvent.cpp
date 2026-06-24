// Copyright 2024 Locomotion System. All Rights Reserved.
// 脚步音效通知实现 Footstep anim notify implementation

#include "Character/Animation/Notifies/AnimNotify_FoleyEvent.h"
#include "Character/Core/CharacterEventBus.h"
#include "GameFramework/Actor.h"

void UAnimNotify_FoleyEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	// 查找 EventBus 并发布脚步事件 Find EventBus and publish footstep event
	UCharacterEventBus* EventBus = Owner->FindComponentByClass<UCharacterEventBus>();
	if (EventBus)
	{
		EventBus->PublishFootstep(Side);
	}
}
