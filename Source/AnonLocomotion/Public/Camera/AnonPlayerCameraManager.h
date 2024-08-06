// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "AnonPlayerCameraManager.generated.h"

class AAnonCharacter;

UCLASS()
class ANONLOCOMOTION_API AAnonPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
public:
	AAnonPlayerCameraManager();
	
	void OnPossess(AAnonCharacter* NewCharacter);
	
	float GetCameraBehaviorParam(FName CurveName) const;

protected:
	// ======================== References ======================== //
	
	TWeakObjectPtr<AAnonCharacter> ControlledCharacter;
	// TWeakObjectPtr<UAnonAn>

	UPROPERTY(VisibleAnywhere, Category = "ALS|Camera")
	TObjectPtr<USkeletalMeshComponent> CameraBehavior;

	// ======================== Managers ======================== //
	
	virtual void UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime) override;
	
	static FVector CalculateAxisIndependentLag(
		FVector CurrentLocation, FVector TargetLocation, FRotator CameraRotation, FVector LagSpeeds, float DeltaTime);
	
	bool CustomCameraBehavior(float DeltaTime, FVector& Location, FRotator& Rotation, float& FOV);

	UPROPERTY(VisibleAnywhere, Category = "ALS|Camera")
	FVector RootLocation;

	UPROPERTY(VisibleAnywhere, Category = "ALS|Camera")
	FTransform SmoothedPivotTarget;

	UPROPERTY(VisibleAnywhere, Category = "ALS|Camera")
	FVector PivotLocation;

	UPROPERTY(VisibleAnywhere, Category = "ALS|Camera")
	FVector TargetCameraLocation;

	UPROPERTY(VisibleAnywhere, Category = "ALS|Camera")
	FRotator TargetCameraRotation;
};
