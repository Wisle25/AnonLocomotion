// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Data/LocomotionEnum.h"
#include "Data/LocomotionStruct.h"
#include "AnonCharacterMovement.generated.h"

UCLASS(ClassGroup=(Anon))
class ANONLOCOMOTION_API UAnonCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class ANONLOCOMOTION_API FSavedMove_Anon final : public FSavedMove_Character
	{
	public:

		typedef FSavedMove_Character Super;

		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel,
								FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* Character) override;

		// Walk Speed Update
		uint8 bSavedRequestMovementSettingsChange : 1;
		EGait SavedAllowedGait = EGait::Walking;
	};

	class ANONLOCOMOTION_API FNetworkPredictionData_Client_Anon final : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_Anon(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
	};

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity) override;

public:
	// Movement Settings Override
	virtual void PhysWalking(float DeltaTime, int32 Iterations) override;
	virtual float GetMaxAcceleration() const override;
	virtual float GetMaxBrakingDeceleration() const override;
	
	// Movement Settings Variables
	uint8 bRequestMovementSettingsChange = 1;
	
	EGait AllowedGait = EGait::Walking;
	
	FMovementSettings CurrentMovementSettings;

	// Set Movement Curve (Called in every instance)
	float GetMappedSpeed() const;

	
	void SetMovementSettings(const FMovementSettings& NewMovementSettings);

	// Set Max Walking Speed (Called from the owning client)
	void SetAllowedGait(EGait NewAllowedGait);

	UFUNCTION(Reliable, Server)
	void Server_SetAllowedGait(EGait NewAllowedGait);
};
