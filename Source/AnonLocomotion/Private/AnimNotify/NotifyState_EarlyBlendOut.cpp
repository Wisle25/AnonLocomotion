// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotify/NotifyState_EarlyBlendOut.h"

#include "Characters/AnonCharacter.h"

void UNotifyState_EarlyBlendOut::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                           float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	
	if (!MeshComp || !AnimInstance) return;

	const AAnonCharacter* OwnerCharacter = Cast<AAnonCharacter>(MeshComp->GetOwner());
	if (!OwnerCharacter) return;

	bool bStopMontage = false;
	if (bCheckMovementState && OwnerCharacter->GetMovementState() == MovementStateEquals)
	{
		bStopMontage = true;
	}
	else if (bCheckStance && OwnerCharacter->GetStance() == StanceEquals)
	{
		bStopMontage = true;
	}
	else if (bCheckMovementInput && OwnerCharacter->HasMovementInput())
	{
		bStopMontage = true;
	}

	if (bStopMontage)
	{
		AnimInstance->Montage_Stop(BlendOutTime, ThisMontage);
	}
}

FString UNotifyState_EarlyBlendOut::GetNotifyName_Implementation() const
{
	return FString(TEXT("Early Blend Out"));
}
