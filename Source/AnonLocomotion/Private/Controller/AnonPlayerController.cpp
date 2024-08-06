// Fill out your copyright notice in the Description page of Project Settings.

#include "Controller/AnonPlayerController.h"

#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Camera/AnonPlayerCameraManager.h"
#include "Characters/AnonCharacter.h"

void AAnonPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);
	PossessedCharacter = Cast<AAnonCharacter>(NewPawn);

	if (!IsRunningDedicatedServer())
	{
		SetupCamera();
	}

	SetupInputs();
}

void AAnonPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	PossessedCharacter = Cast<AAnonCharacter>(GetPawn());
	SetupCamera();
	SetupInputs();
}

void AAnonPlayerController::SetupInputs() const
{
	if (PossessedCharacter.IsValid())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			FModifyContextOptions Options;
			Options.bForceImmediately = 1;
			Subsystem->AddMappingContext(DefaultInputMappingContext.LoadSynchronous(), 1, Options);
		}
	}
}

UInputMappingContext* AAnonPlayerController::GetDefaultInput() const
{
	return DefaultInputMappingContext.LoadSynchronous();
}

void AAnonPlayerController::SetupCamera() const
{
	// Call "OnPossess" in Player Camera Manager when possessing a pawn
	AAnonPlayerCameraManager* CastedMgr = Cast<AAnonPlayerCameraManager>(PlayerCameraManager);
	if (PossessedCharacter.IsValid() && CastedMgr)
	{
		CastedMgr->OnPossess(PossessedCharacter.Get());
	}
}
