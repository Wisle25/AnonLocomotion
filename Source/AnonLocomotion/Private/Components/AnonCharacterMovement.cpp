// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/AnonCharacterMovement.h"

#include "Curves/CurveVector.h"
#include "GameFramework/Character.h"


void UAnonCharacterMovement::OnMovementUpdated(float DeltaTime, const FVector& OldLocation,
                                               const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaTime, OldLocation, OldVelocity);

	if (!CharacterOwner)
	{
		return;
	}

	// Set Movement Settings
	if (bRequestMovementSettingsChange)
	{
		const float UpdateMaxWalkSpeed = CurrentMovementSettings.GetSpeedForGait(AllowedGait);
		MaxWalkSpeed = UpdateMaxWalkSpeed;
		MaxWalkSpeedCrouched = UpdateMaxWalkSpeed;

		bRequestMovementSettingsChange = false;
	}
}

void UAnonCharacterMovement::PhysWalking(float deltaTime, int32 Iterations)
{
	if (CurrentMovementSettings.MovementCurve)
	{
		// Update the Ground Friction using the Movement Curve.
		// This allows for fine control over movement behavior at each speed.
		GroundFriction = CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).Z;
	}
	Super::PhysWalking(deltaTime, Iterations);
}

float UAnonCharacterMovement::GetMaxAcceleration() const
{
	// Update the Acceleration using the Movement Curve.
	// This allows for fine control over movement behavior at each speed.
	if (!IsMovingOnGround() || !CurrentMovementSettings.MovementCurve)
	{
		return Super::GetMaxAcceleration();
	}
	
	return CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).X;
}

float UAnonCharacterMovement::GetMaxBrakingDeceleration() const
{
	// Update the Deceleration using the Movement Curve.
	// This allows for fine control over movement behavior at each speed.
	if (!IsMovingOnGround() || !CurrentMovementSettings.MovementCurve)
	{
		return Super::GetMaxBrakingDeceleration();
	}
	return CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).Y;
}

void UAnonCharacterMovement::UpdateFromCompressedFlags(uint8 Flags) // Client only
{
	Super::UpdateFromCompressedFlags(Flags);

	bRequestMovementSettingsChange = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

class FNetworkPredictionData_Client* UAnonCharacterMovement::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);

	if (!ClientPredictionData)
	{
		UAnonCharacterMovement* MutableThis = const_cast<UAnonCharacterMovement*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Anon(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UAnonCharacterMovement::FSavedMove_Anon::Clear()
{
	Super::Clear();

	bSavedRequestMovementSettingsChange = false;
	SavedAllowedGait = EGait::Walking;
}

uint8 UAnonCharacterMovement::FSavedMove_Anon::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedRequestMovementSettingsChange)
	{
		Result |= FLAG_Custom_0;
	}

	return Result;
}

void UAnonCharacterMovement::FSavedMove_Anon::SetMoveFor(ACharacter* Character, float InDeltaTime,
                                                               FVector const& NewAccel,
                                                               FNetworkPredictionData_Client_Character&
                                                               ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UAnonCharacterMovement* CharacterMovement = Cast<UAnonCharacterMovement>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		bSavedRequestMovementSettingsChange = CharacterMovement->bRequestMovementSettingsChange;
		SavedAllowedGait = CharacterMovement->AllowedGait;
	}
}

void UAnonCharacterMovement::FSavedMove_Anon::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	UAnonCharacterMovement* CharacterMovement = Cast<UAnonCharacterMovement>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		CharacterMovement->AllowedGait = SavedAllowedGait;
	}
}

UAnonCharacterMovement::FNetworkPredictionData_Client_Anon::FNetworkPredictionData_Client_Anon(
	const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UAnonCharacterMovement::FNetworkPredictionData_Client_Anon::AllocateNewMove()
{
	return MakeShared<FSavedMove_Anon>();
}

void UAnonCharacterMovement::Server_SetAllowedGait_Implementation(const EGait NewAllowedGait)
{
	AllowedGait = NewAllowedGait;
}

float UAnonCharacterMovement::GetMappedSpeed() const
{
	// Map the character's current speed to the configured movement speeds with a range of 0-3,
	// with 0 = stopped, 1 = the Walk Speed, 2 = the Run Speed, and 3 = the Sprint Speed.
	// This allows us to vary the movement speeds but still use the mapped range in calculations for consistent results

	const float Speed = Velocity.Size2D();
	const float LocWalkSpeed = CurrentMovementSettings.WalkSpeed;
	const float LocRunSpeed = CurrentMovementSettings.RunSpeed;
	const float LocSprintSpeed = CurrentMovementSettings.SprintSpeed;

	if (Speed > LocRunSpeed)
	{
		return FMath::GetMappedRangeValueClamped<float, float>({LocRunSpeed, LocSprintSpeed}, {2.0f, 3.0f}, Speed);
	}

	if (Speed > LocWalkSpeed)
	{
		return FMath::GetMappedRangeValueClamped<float, float>({LocWalkSpeed, LocRunSpeed}, {1.0f, 2.0f}, Speed);
	}

	return FMath::GetMappedRangeValueClamped<float, float>({0.0f, LocWalkSpeed}, {0.0f, 1.0f}, Speed);
}

void UAnonCharacterMovement::SetMovementSettings(const FMovementSettings& NewMovementSettings)
{
	// Set the current movement settings from the owner
	CurrentMovementSettings = NewMovementSettings;
	bRequestMovementSettingsChange = true;
}

void UAnonCharacterMovement::SetAllowedGait(const EGait NewAllowedGait)
{
	if (AllowedGait != NewAllowedGait)
	{
		if (PawnOwner->IsLocallyControlled())
		{
			AllowedGait = NewAllowedGait;
			if (GetCharacterOwner()->GetLocalRole() == ROLE_AutonomousProxy)
			{
				Server_SetAllowedGait(NewAllowedGait);
			}
			bRequestMovementSettingsChange = true;
			return;
		}
		if (!PawnOwner->HasAuthority())
		{
			const float UpdateMaxWalkSpeed = CurrentMovementSettings.GetSpeedForGait(AllowedGait);
			MaxWalkSpeed = UpdateMaxWalkSpeed;
			MaxWalkSpeedCrouched = UpdateMaxWalkSpeed;
		}
	}
}