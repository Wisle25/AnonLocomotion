// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AnonPlayerController.generated.h"

struct FInputActionValue;
class UInputMappingContext;
class AAnonCharacter;

UCLASS()
class ANONLOCOMOTION_API AAnonPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void OnPossess(APawn* NewPawn) override;
	virtual void OnRep_Pawn() override;

private:
	/** Main character reference */
	TWeakObjectPtr<AAnonCharacter> PossessedCharacter;

	void SetupCamera() const;

	// ==================== Input ==================== //
	
	UPROPERTY(EditDefaultsOnly, Category = "ALS|Input")
	TSoftObjectPtr<UInputMappingContext> DefaultInputMappingContext;
	
	void SetupInputs() const;

public:
	UInputMappingContext* GetDefaultInput() const;
};
