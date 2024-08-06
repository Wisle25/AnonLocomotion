// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/LocomotionEnum.h"
#include "LocomotionEnumHelper.generated.h"

USTRUCT(BlueprintType)
struct FMovementState
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	EMovementState State = EMovementState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	bool bNone = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	bool bGrounded = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	bool bInAir = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	bool bRagdoll = false;

public:
	FMovementState()
	{
	}

	FMovementState(const EMovementState InitialState) { *this = InitialState; }

	FORCEINLINE const bool& None() const { return bNone; }
	FORCEINLINE const bool& Grounded() const { return bGrounded; }
	FORCEINLINE const bool& InAir() const { return bInAir; }
	FORCEINLINE const bool& Ragdoll() const { return bRagdoll; }

	operator EMovementState() const { return State; }

	void operator=(const EMovementState NewState)
	{
		State = NewState;
		bNone = State == EMovementState::None;
		bGrounded = State == EMovementState::Grounded;
		bInAir = State == EMovementState::InAir;
		bRagdoll = State == EMovementState::Ragdoll;
	}
};

USTRUCT(BlueprintType)
struct FStance
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Character States")
	EStance Stance = EStance::Standing;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Character States")
	bool bStanding = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Character States")
	bool bCrouching = false;

public:
	FStance()
	{
	}

	FStance(const EStance InitialStance) { *this = InitialStance; }

	FORCEINLINE const bool& Standing() const { return bStanding; }
	FORCEINLINE const bool& Crouching() const { return bCrouching; }

	operator EStance() const { return Stance; }

	void operator=(const EStance NewStance)
	{
		Stance = NewStance;
		bStanding = Stance == EStance::Standing;
		bCrouching = Stance == EStance::Crouching;
	}
};

USTRUCT(BlueprintType)
struct FRotationMode
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Rotation System")
	ERotationMode RotationMode = ERotationMode::VelocityDirection;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Rotation System")
	bool bVelocityDirection = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Rotation System")
	bool bLookingDirection = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Rotation System")
	bool bAiming = false;

public:
	FRotationMode()
	{
	}

	FRotationMode(const ERotationMode InitialRotationMode) { *this = InitialRotationMode; }

	FORCEINLINE const bool& VelocityDirection() const { return bVelocityDirection; }
	FORCEINLINE const bool& LookingDirection() const { return bLookingDirection; }
	FORCEINLINE const bool& Aiming() const { return bAiming; }

	operator ERotationMode() const { return RotationMode; }

	void operator=(const ERotationMode NewRotationMode)
	{
		RotationMode = NewRotationMode;
		bVelocityDirection = RotationMode == ERotationMode::VelocityDirection;
		bLookingDirection = RotationMode == ERotationMode::LookingDirection;
		bAiming = RotationMode == ERotationMode::Aiming;
	}
};

USTRUCT(BlueprintType)
struct FMovementDirection
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	EMovementDirection MovementDirection = EMovementDirection::Forward;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	bool bForward = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	bool bRight = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	bool bLeft = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	bool bBackward = false;

public:
	FMovementDirection()
	{
	}

	FMovementDirection(const EMovementDirection InitialMovementDirection)
	{
		*this = InitialMovementDirection;
	}

	FORCEINLINE const bool& Forward() const { return bForward; }
	FORCEINLINE const bool& Right() const { return bRight; }
	FORCEINLINE const bool& Left() const { return bLeft; }
	FORCEINLINE const bool& Backward() const { return bBackward; }

	operator EMovementDirection() const { return MovementDirection; }

	void operator=(const EMovementDirection NewMovementDirection)
	{
		MovementDirection = NewMovementDirection;
		bForward = MovementDirection == EMovementDirection::Forward;
		bRight = MovementDirection == EMovementDirection::Right;
		bLeft = MovementDirection == EMovementDirection::Left;
		bBackward = MovementDirection == EMovementDirection::Backward;
	}
};

USTRUCT(BlueprintType)
struct FMovementAction
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	EMovementAction Action = EMovementAction::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	bool bNone = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	bool bRolling = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	bool bGettingUp = false;

public:
	FMovementAction()
	{
	}

	FMovementAction(const EMovementAction InitialAction) { *this = InitialAction; }

	FORCEINLINE const bool& None() const { return bNone; }
	FORCEINLINE const bool& Rolling() const { return bRolling; }
	FORCEINLINE const bool& GettingUp() const { return bGettingUp; }

	operator EMovementAction() const { return Action; }

	void operator=(const EMovementAction NewAction)
	{
		Action = NewAction;
		bNone = Action == EMovementAction::None;
		bRolling = Action == EMovementAction::Rolling;
		bGettingUp = Action == EMovementAction::GettingUp;
	}
};

USTRUCT(BlueprintType)
struct FGait
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	EGait Gait = EGait::Walking;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	bool bWalking = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	bool bRunning = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	bool bSprinting = false;

public:
	FGait()
	{
	}

	FGait(const EGait InitialGait) { *this = InitialGait; }

	FORCEINLINE const bool& Walking() const { return bWalking; }
	FORCEINLINE const bool& Running() const { return bRunning; }
	FORCEINLINE const bool& Sprinting() const { return bSprinting; }

	operator EGait() const { return Gait; }

	void operator=(const EGait NewGait)
	{
		Gait = NewGait;
		bWalking = Gait == EGait::Walking;
		bRunning = Gait == EGait::Running;
		bSprinting = Gait == EGait::Sprinting;
	}
};

USTRUCT(BlueprintType)
struct FOverlayState
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Character States")
	EOverlayState State = EOverlayState::Default;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Character States")
	bool bDefault = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Character States")
	bool bMasculine = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Character States")
	bool bFeminine = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Character States")
	bool bInjured = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Character States")
	bool bHandsTied = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Character States")
	bool bRifle = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Character States")
	bool bPistolOneHanded = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Character States")
	bool bPistolTwoHanded = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Character States")
	bool bBow = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Character States")
	bool bTorch = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Character States")
	bool bBinoculars = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Character States")
	bool bBox = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Character States")
	bool bBarrel = false;

public:
	FOverlayState()
	{
	}

	FOverlayState(const EOverlayState InitialState) { *this = InitialState; }

	FORCEINLINE const bool& Default() const { return bDefault; }
	FORCEINLINE const bool& Masculine() const { return bMasculine; }
	FORCEINLINE const bool& Feminine() const { return bFeminine; }
	FORCEINLINE const bool& Injured() const { return bInjured; }
	FORCEINLINE const bool& HandsTied() const { return bHandsTied; }
	FORCEINLINE const bool& Rifle() const { return bRifle; }
	FORCEINLINE const bool& PistolOneHanded() const { return bPistolOneHanded; }
	FORCEINLINE const bool& PistolTwoHanded() const { return bPistolTwoHanded; }
	FORCEINLINE const bool& Bow() const { return bBow; }
	FORCEINLINE const bool& Torch() const { return bTorch; }
	FORCEINLINE const bool& Binoculars() const { return bBinoculars; }
	FORCEINLINE const bool& Box() const { return bBox; }
	FORCEINLINE const bool& Barrel() const { return bBarrel; }

	operator EOverlayState() const { return State; }

	void operator=(const EOverlayState NewAction)
	{
		State = NewAction;
		bDefault = State == EOverlayState::Default;
		bMasculine = State == EOverlayState::Masculine;
		bFeminine = State == EOverlayState::Feminine;
		bInjured = State == EOverlayState::Injured;
		bHandsTied = State == EOverlayState::HandsTied;
		bRifle = State == EOverlayState::Rifle;
		bPistolOneHanded = State == EOverlayState::PistolOneHanded;
		bPistolTwoHanded = State == EOverlayState::PistolTwoHanded;
		bBow = State == EOverlayState::Bow;
		bTorch = State == EOverlayState::Torch;
		bBinoculars = State == EOverlayState::Binoculars;
		bBox = State == EOverlayState::Box;
		bBarrel = State == EOverlayState::Barrel;
	}
};

USTRUCT(BlueprintType)
struct FGroundedEntryState
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Breakfall System")
	EGroundedEntryState State = EGroundedEntryState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Breakfall System")
	bool bNone = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Breakfall System")
	bool bRoll = false;

public:
	FGroundedEntryState()
	{
	}

	FGroundedEntryState(const EGroundedEntryState InitialState) { *this = InitialState; }

	FORCEINLINE const bool& None() const { return bNone; }
	FORCEINLINE const bool& Roll() const { return bRoll; }

	operator EGroundedEntryState() const { return State; }

	void operator=(const EGroundedEntryState NewState)
	{
		State = NewState;
		bNone = State == EGroundedEntryState::None;
		bRoll = State == EGroundedEntryState::Roll;
	}
};

