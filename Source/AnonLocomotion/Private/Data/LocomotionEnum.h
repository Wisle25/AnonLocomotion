#pragma once

#include "CoreMinimal.h"

/* Returns the enumeration index. */
template <typename Enumeration>
static FORCEINLINE int32 GetEnumerationIndex(const Enumeration InValue)
{
	return StaticEnum<Enumeration>()->GetIndexByValue(static_cast<int64>(InValue));
}

/* Returns the enumeration value as string. */
template <typename Enumeration>
static FORCEINLINE FString GetEnumerationToString(const Enumeration InValue)
{
	return StaticEnum<Enumeration>()->GetNameStringByValue(static_cast<int64>(InValue));
}

/**
 * Character gait state. Note: Also edit related struct in ALSStructEnumLibrary if you add new enums
 */
UENUM(BlueprintType)
enum class EGait : uint8
{
	Walking,
	Running,
	Sprinting
};

/**
 * Character movement action state. Note: Also edit related struct in ALSStructEnumLibrary if you add new enums
 */
UENUM(BlueprintType)
enum class EMovementAction : uint8
{
	None,
	Rolling,
	GettingUp
};

/**
 * Character movement state. Note: Also edit related struct in ALSStructEnumLibrary if you add new enums
 */
UENUM(BlueprintType)
enum class EMovementState : uint8
{
	None,
	Grounded,
	InAir,
	Ragdoll
};

/**
 * Character overlay state. Note: Also edit related struct in ALSStructEnumLibrary if you add new enums
 */
UENUM(BlueprintType)
enum class EOverlayState : uint8
{
	Default,
	Masculine,
	Feminine,
	Injured,
	HandsTied,
	Rifle,
	PistolOneHanded,
	PistolTwoHanded,
	Bow,
	Torch,
	Binoculars,
	Box,
	Barrel
};

/**
 * Character rotation mode. Note: Also edit related struct in ALSStructEnumLibrary if you add new enums
 */
UENUM(BlueprintType)
enum class ERotationMode : uint8
{
	VelocityDirection,
	LookingDirection,
	Aiming
};

/**
 * Character stance. Note: Also edit related struct in ALSStructEnumLibrary if you add new enums
 */
UENUM(BlueprintType)
enum class EStance : uint8
{
	Standing,
	Crouching
};

/**
 * Character view mode. Note: Also edit related struct in ALSStructEnumLibrary if you add new enums
 */
UENUM(BlueprintType)
enum class EViewMode : uint8
{
	ThirdPerson,
	FirstPerson
};

UENUM(BlueprintType)
enum class EAnimFeatureExample : uint8
{
	StrideBlending,
	AdditiveBlending,
	SprintImpulse
};

UENUM(BlueprintType)
enum class EFootstepType : uint8
{
	Step,
	WalkRun,
	Jump,
	Land
};

UENUM(BlueprintType)
enum class EGroundedEntryState : uint8
{
	None,
	Roll
};

UENUM(BlueprintType)
enum class EHipsDirection : uint8
{
	F,
	B,
	RF,
	RB,
	LF,
	LB
};

UENUM(BlueprintType)
enum class EMantleType : uint8
{
	HighMantle,
	LowMantle,
	FallingCatch
};

UENUM(BlueprintType)
enum class EMovementDirection : uint8
{
	Forward,
	Right,
	Left,
	Backward
};

UENUM(BlueprintType)
enum class ESpawnType : uint8
{
	Location,
	Attached
};
