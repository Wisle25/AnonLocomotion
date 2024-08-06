// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Data/LocomotionEnum.h"
#include "AnimNotify_GroundedEntryState.generated.h"

UCLASS()
class ANONLOCOMOTION_API UAnimNotify_GroundedEntryState : public UAnimNotify
{
	GENERATED_BODY()

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	EGroundedEntryState GroundedEntryState = EGroundedEntryState::None;
};
