// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify/AnimNotify_GroundedEntryState.h"

#include "Characters/AnonCharacter.h"

void UAnimNotify_GroundedEntryState::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                            const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AAnonCharacter* Character = Cast<AAnonCharacter>(MeshComp->GetOwner()))
	{
		Character->SetGroundedEntryState(GroundedEntryState);
	}
}

FString UAnimNotify_GroundedEntryState::GetNotifyName_Implementation() const
{
	FString Name(TEXT("Grounded Entry State: "));
	Name.Append(GetEnumerationToString(GroundedEntryState));
	
	return Name;
}
