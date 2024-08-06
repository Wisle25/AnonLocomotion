// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotify/NotifyState_MovementAction.h"

#include "Characters/AnonCharacter.h"

void UNotifyState_MovementAction::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                             float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AAnonCharacter* BaseCharacter = Cast<AAnonCharacter>(MeshComp->GetOwner()))
	{
		BaseCharacter->SetMovementAction(MovementAction);
	}
}

void UNotifyState_MovementAction::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
											  const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	AAnonCharacter* BaseCharacter = Cast<AAnonCharacter>(MeshComp->GetOwner());
	if (BaseCharacter && BaseCharacter->GetMovementAction() == MovementAction)
	{
		BaseCharacter->SetMovementAction(EMovementAction::None);
	}
}

FString UNotifyState_MovementAction::GetNotifyName_Implementation() const
{
	FString Name(TEXT("Movement Action: "));
	Name.Append(GetEnumerationToString(MovementAction));
	return Name;
}