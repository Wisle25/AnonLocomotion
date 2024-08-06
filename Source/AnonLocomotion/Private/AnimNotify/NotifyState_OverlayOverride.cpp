// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotify/NotifyState_OverlayOverride.h"

#include "Characters/AnonCharacter.h"

void UNotifyState_OverlayOverride::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                               float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AAnonCharacter* Character = Cast<AAnonCharacter>(MeshComp->GetOwner()))
	{
		Character->SetOverlayOverrideState(OverlayOverrideState);
	}
}

void UNotifyState_OverlayOverride::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (AAnonCharacter* Character = Cast<AAnonCharacter>(MeshComp->GetOwner()))
	{
		Character->SetOverlayOverrideState(0);
	}
}

FString UNotifyState_OverlayOverride::GetNotifyName_Implementation() const
{
	FString Name(TEXT("Overlay Override State: "));
	Name.Append(FString::FromInt(OverlayOverrideState));
	
	return Name;
}
