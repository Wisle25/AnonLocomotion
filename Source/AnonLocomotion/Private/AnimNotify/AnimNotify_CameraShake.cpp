// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotify/AnimNotify_CameraShake.h"

void UAnimNotify_CameraShake::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (const APawn* OwnerPawn = Cast<APawn>(MeshComp->GetOwner()))
	{
		if (APlayerController* OwnerController = Cast<APlayerController>(OwnerPawn->GetController()))
		{
			OwnerController->ClientStartCameraShake(ShakeClass, Scale);
		}
	}
}
