// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Data/LocomotionEnum.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AnimNotify_Footstep.generated.h"

UCLASS()
class ANONLOCOMOTION_API UAnimNotify_Footstep : public UAnimNotify
{
	GENERATED_BODY()

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

public:
	UPROPERTY(EditAnywhere, Category = "Settings")
	TObjectPtr<UDataTable> HitDataTable;

	UPROPERTY(EditAnywhere, Category = "Socket")
	FName FootSocketName = TEXT("foot_r");

	UPROPERTY(EditAnywhere, Category = "Trace")
	TEnumAsByte<ETraceTypeQuery> TraceChannel;

	UPROPERTY(EditAnywhere, Category = "Trace")
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType;

	UPROPERTY(EditAnywhere, Category = "Trace")
	float TraceLength = 50.f;

	UPROPERTY(EditAnywhere, Category = "Decal")
	bool bSpawnDecal = false;

	UPROPERTY(EditAnywhere, Category = "Decal")
	bool bMirrorDecalX = false;

	UPROPERTY(EditAnywhere, Category = "Decal")
	bool bMirrorDecalY = false;

	UPROPERTY(EditAnywhere, Category = "Decal")
	bool bMirrorDecalZ = false;

	UPROPERTY(EditAnywhere, Category = "Sound")
	bool bSpawnSound = true;
	
	UPROPERTY(EditAnywhere, Category = "Sound")
	FName SoundParameterName = TEXT("FootstepType");

	UPROPERTY(EditAnywhere, Category = "Sound")
	EFootstepType FootstepType = EFootstepType::Step;

	UPROPERTY(EditAnywhere, Category = "Sound")
	bool bOverrideMaskCurve = false;

	UPROPERTY(EditAnywhere, Category = "Sound")
	float VolumeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Sound")
	float PitchMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	bool bSpawnNiagara = false;
};
