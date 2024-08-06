// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Data/LocomotionEnum.h"
#include "AnonPlayerCameraBehavior.generated.h"

UCLASS()
class ANONLOCOMOTION_API UAnonPlayerCameraBehavior : public UAnimInstance
{
	GENERATED_BODY()

public:
	void SetRotationMode(ERotationMode RotationMode);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	EMovementState MovementState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	EMovementAction MovementAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	bool bLookingDirection = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	bool bVelocityDirection = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	bool bAiming = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	EGait Gait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	EStance Stance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	EViewMode ViewMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	bool bRightShoulder = false;

};
