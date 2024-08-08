// ReSharper disable CppUE4CodingStandardNamingViolationWarning

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ETraversalState : uint8
{
	FreeRoam,
	ReadyToClimb,
	Climb,
	Mantle,
	Vault
};

UENUM(BlueprintType)
enum class ETraversalAction : uint8
{
	NoAction,
	
	BracedClimb,
	BC_FallingClimb,
	BC_ClimbUp,
	BC_HopUp,
	BC_HopLeft,
	BC_HopRight,
	BC_HopLeftUp,
	BC_HopRightUp,
	BC_HopDown,

	FreeHang,
	FH_FallingClimb,
	FH_ClimbUp,
	FH_HopLeft,
	FH_HopRight,
	FH_HopLeftUp,
	FH_HopRightUp,
	FH_HopDown,

	CornerMove,
	Mantle,
	Vault
};

UENUM(BlueprintType)
enum class ETraversalDirection : uint8
{
	NoDirection,
	Left,
	Right,
	Forward,
	Backward,
	ForwardLeft,
	ForwardRight,
	BackwardLeft,
	BackwardRight
};

UENUM(BlueprintType)
enum class EClimbStyle : uint8
{
	BracedClimb,
	FreeHang
};
