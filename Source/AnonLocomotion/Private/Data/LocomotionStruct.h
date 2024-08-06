#pragma once

#include "CoreMinimal.h"
#include "LocomotionEnum.h"
#include "LocomotionStruct.generated.h"

class UCurveVector;
class UAnimSequenceBase;
class UCurveFloat;
class UNiagaraSystem;
class UMaterialInterface;
class USoundBase;

USTRUCT(BlueprintType)
struct FMovementSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=MovementSettings)
	float WalkSpeed = 0.f;

	UPROPERTY(EditAnywhere, Category=MovementSettings)
	float RunSpeed = 0.f;

	UPROPERTY(EditAnywhere, Category=MovementSettings)
	float SprintSpeed = 0.f;

	UPROPERTY(EditAnywhere, Category=MovementSettings)
	TObjectPtr<UCurveVector> MovementCurve = nullptr;

	UPROPERTY(EditAnywhere, Category=MovementSettings)
	TObjectPtr<UCurveFloat> RotationRateCurve = nullptr;

	float GetSpeedForGait(const EGait Gait) const
	{
		switch (Gait)
		{
		case EGait::Running:
			return RunSpeed;
		case EGait::Sprinting:
			return SprintSpeed;
		case EGait::Walking:
			return WalkSpeed;
		default:
			return RunSpeed;
		}
	}
};

USTRUCT(BlueprintType)
struct FMovementStanceSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=MovementSettings)
	FMovementSettings Standing;

	UPROPERTY(EditAnywhere, Category=MovementSettings)
	FMovementSettings Crouching;
};

USTRUCT(BlueprintType)
struct FMovementStateSettings : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=MovementSettings)
	FMovementStanceSettings VelocityDirection;

	UPROPERTY(EditAnywhere, Category=MovementSettings)
	FMovementStanceSettings LookingDirection;

	UPROPERTY(EditAnywhere, Category=MovementSettings)
	FMovementStanceSettings Aiming;
};

USTRUCT(BlueprintType)
struct FRotateInPlaceAsset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=RotationSystem)
	TObjectPtr<UAnimSequenceBase> Animation = nullptr;

	UPROPERTY(EditAnywhere, Category=RotationSystem)
	FName SlotName = NAME_None;

	UPROPERTY(EditAnywhere, Category=RotationSystem)
	float SlowTurnRate = 90.f;

	UPROPERTY(EditAnywhere, Category=RotationSystem)
	float FastTurnRate = 90.f;

	UPROPERTY(EditAnywhere, Category=RotationSystem)
	float SlowPlayRate = 1.f;

	UPROPERTY(EditAnywhere, Category=RotationSystem)
	float FastPlayRate = 1.f;
};

USTRUCT(BlueprintType)
struct FHitFX : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Surface)
	TEnumAsByte<EPhysicalSurface> SurfaceType = EPhysicalSurface::SurfaceType_Default;

	UPROPERTY(EditAnywhere, Category=Sound)
	TSoftObjectPtr<USoundBase> Sound = nullptr;

	UPROPERTY(EditAnywhere, Category=Sound)
	ESpawnType SoundSpawnType = ESpawnType::Location;

	UPROPERTY(EditAnywhere, Category=Sound, meta = (EditCondition = "SoundSpawnType == ESpawnType::Attached"))
	TEnumAsByte<EAttachLocation::Type> SoundAttachmentType = EAttachLocation::KeepRelativeOffset;

	UPROPERTY(EditAnywhere, Category=Sound)
	FVector SoundLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category=Sound)
	FRotator SoundRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category=Decal)
	TSoftObjectPtr<UMaterialInterface> DecalMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category=Decal)
	ESpawnType DecalSpawnType = ESpawnType::Location;

	UPROPERTY(EditAnywhere, Category=Decal, meta = (EditCondition = "DecalSpawnType == ESpawnType::Attached"))
	TEnumAsByte<EAttachLocation::Type> DecalAttachmentType = EAttachLocation::KeepRelativeOffset;

	UPROPERTY(EditAnywhere, Category=Decal)
	float DecalLifeSpan = 10.f;

	UPROPERTY(EditAnywhere, Category=Decal)
	FVector DecalSize = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category=Decal)
	FVector DecalLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category=Decal)
	FRotator DecalRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category=Niagara)
	TSoftObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

	UPROPERTY(EditAnywhere, Category=Niagara)
	ESpawnType NiagaraSpawnType = ESpawnType::Location;

	UPROPERTY(EditAnywhere, Category=Niagara, meta = (EditCondition = "NiagaraSpawnType == ESpawnType::Attached"))
	TEnumAsByte<EAttachLocation::Type> NiagaraAttachmentType = EAttachLocation::KeepRelativeOffset;

	UPROPERTY(EditAnywhere, Category=Niagara)
	FVector NiagaraLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category=Niagara)
	FRotator NiagaraRotationOffset = FRotator::ZeroRotator;
};

USTRUCT(BlueprintType)
struct FDynamicMontageParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ALS|Dynamic Transition")
	TObjectPtr<UAnimSequenceBase> Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ALS|Dynamic Transition")
	float BlendInTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ALS|Dynamic Transition")
	float BlendOutTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ALS|Dynamic Transition")
	float PlayRate = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ALS|Dynamic Transition")
	float StartTime = 0.f;
};

USTRUCT(BlueprintType)
struct FLeanAmount
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float LR = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float FB = 0.f;
};

USTRUCT(BlueprintType)
struct FVelocityBlend
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float F = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float B = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float L = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float R = 0.f;
};

USTRUCT(BlueprintType)
struct FTurnInPlaceAsset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	TObjectPtr<UAnimSequenceBase> Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	float AnimatedAngle = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	FName SlotName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	float PlayRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	bool ScaleTurnAngle = true;
};

USTRUCT(BlueprintType)
struct FAnimCharacterInformation
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	FRotator AimingRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	FRotator CharacterActorRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	FVector RelativeVelocityDirection = FVector::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	FVector Acceleration = FVector::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	FVector MovementInput = FVector::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	bool bIsMoving = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	bool bHasMovementInput = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	float Speed = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	float MovementInputAmount = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	float AimYawRate = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	float ZoomAmount = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	EMovementState PrevMovementState = EMovementState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	EViewMode ViewMode = EViewMode::ThirdPerson;
};

USTRUCT(BlueprintType)
struct FAnimGraphGrounded
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "ALS|Anim Graph - Grounded")
	EHipsDirection TrackedHipsDirection = EHipsDirection::F;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	bool bShouldMove = false; // Should be false initially

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	bool bRotateL = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	bool bRotateR = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "ALS|Anim Graph - Grounded")
	bool bPivot = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float RotateRate = 1.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float RotationScale = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float DiagonalScaleAmount = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float WalkRunBlend = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float StandingPlayRate = 1.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float CrouchingPlayRate = 1.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float StrideBlend = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float FYaw = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float BYaw = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float LYaw = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float RYaw = 0.f;
};

USTRUCT(BlueprintType)
struct FAnimGraphInAir
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - In Air")
	bool bJumped = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - In Air")
	float JumpPlayRate = 1.2f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - In Air")
	float FallSpeed = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - In Air")
	float LandPrediction = 1.f;
};

USTRUCT(BlueprintType)
struct FAnimGraphAimingValues
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Aiming Values")
	FRotator SmoothedAimingRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Aiming Values")
	FRotator SpineRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Aiming Values")
	FVector2D AimingAngle = FVector2D::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Aiming Values")
	float AimSweepTime = 0.5f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Aiming Values")
	float InputYawOffsetTime = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Aiming Values")
	float ForwardYawTime = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Aiming Values")
	float LeftYawTime = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Aiming Values")
	float RightYawTime = 0.f;
};

USTRUCT(BlueprintType)
struct FAnimGraphLayerBlending
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	int32 OverlayOverrideState = 0;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float EnableAimOffset = 1.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float BasePose_N = 1.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float BasePose_CLF = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Arm_L = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Arm_L_Add = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Arm_L_LS = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Arm_L_MS = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Arm_R = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Arm_R_Add = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Arm_R_LS = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Arm_R_MS = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Hand_L = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Hand_R = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Legs = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Legs_Add = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Pelvis = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Pelvis_Add = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Spine = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Spine_Add = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Head = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Head_Add = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float EnableHandIK_L = 1.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float EnableHandIK_R = 1.f;
};

USTRUCT(BlueprintType)
struct FAnimGraphFootIK
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	float FootLock_L_Alpha = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	float FootLock_R_Alpha = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	bool UseFootLockCurve_L = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	bool UseFootLockCurve_R = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FVector FootLock_L_Location = FVector::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FVector TargetFootLock_R_Location = FVector::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FVector FootLock_R_Location = FVector::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FRotator TargetFootLock_L_Rotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FRotator FootLock_L_Rotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FRotator TargetFootLock_R_Rotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FRotator FootLock_R_Rotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FVector FootOffset_L_Location = FVector::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FVector FootOffset_R_Location = FVector::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FRotator FootOffset_L_Rotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FRotator FootOffset_R_Rotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FVector PelvisOffset = FVector::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	float PelvisAlpha = 0.f;
};

USTRUCT(BlueprintType)
struct FAnimTurnInPlace
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	float TurnCheckMinAngle = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	float Turn180Threshold = 130.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	float AimYawRateLimit = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	float ElapsedDelayTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	float MinAngleDelay = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	float MaxAngleDelay = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	FTurnInPlaceAsset N_TurnIP_L90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	FTurnInPlaceAsset N_TurnIP_R90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	FTurnInPlaceAsset N_TurnIP_L180;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	FTurnInPlaceAsset N_TurnIP_R180;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	FTurnInPlaceAsset CLF_TurnIP_L90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	FTurnInPlaceAsset CLF_TurnIP_R90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	FTurnInPlaceAsset CLF_TurnIP_L180;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	FTurnInPlaceAsset CLF_TurnIP_R180;
};

USTRUCT(BlueprintType)
struct FAnimRotateInPlace
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Rotate In Place")
	float RotateMinThreshold = -50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Rotate In Place")
	float RotateMaxThreshold = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Rotate In Place")
	float AimYawRateMinRange = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Rotate In Place")
	float AimYawRateMaxRange = 270.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Rotate In Place")
	float MinPlayRate = 1.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Rotate In Place")
	float MaxPlayRate = 3.f;
};

USTRUCT(BlueprintType)
struct FAnimConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float AnimatedWalkSpeed = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float AnimatedRunSpeed = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float AnimatedSprintSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float AnimatedCrouchSpeed = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float VelocityBlendInterpSpeed = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float GroundedLeanInterpSpeed = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float InAirLeanInterpSpeed = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float SmoothedAimingRotationInterpSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float InputYawOffsetInterpSpeed = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float TriggerPivotSpeedLimit = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float FootHeight = 13.5f;

	/** Threshold value for activating dynamic transition on various animations */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float DynamicTransitionThreshold = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float IK_TraceDistanceAboveFoot = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float IK_TraceDistanceBelowFoot = 45.f;
};

