// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Data/LocomotionStruct.h"
#include "Library/LocomotionEnumHelper.h"
#include "AnonAnimInstance.generated.h"

class AAnonCharacter;

UCLASS()
class ANONLOCOMOTION_API UAnonAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// ==================== Lifecycles ==================== //
	
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// ==================== Transition ==================== //
	
	UFUNCTION(BlueprintCallable, Category = "ALS|Transition")
	void PlayTransition(const FDynamicMontageParams& Parameters);

	UFUNCTION(BlueprintCallable, Category = "ALS|Transition")
	void PlayTransitionChecked(const FDynamicMontageParams& Parameters);

	void PlayDynamicTransition(float ReTriggerDelay, const FDynamicMontageParams& Parameters);

	// ==================== Event ==================== //
	
	void OnJumped();

	UFUNCTION(BlueprintCallable, Category = "ALS|Event")
	void OnPivot();

	// ==================== Grounded ==================== //
	
	UFUNCTION(BlueprintCallable, Category = "ALS|Grounded")
	FORCEINLINE void SetTrackedHipsDirection(const EHipsDirection HipsDirection)
	{
		Grounded.TrackedHipsDirection = HipsDirection;
	}

	UFUNCTION(BlueprintCallable, Category = "ALS|Grounded")
	void SetGroundedEntryState(EGroundedEntryState NewState)
	{
		GroundedEntryState = NewState;
	}

	/** Enable Movement Animations if IsMoving and HasMovementInput, or if the Speed is greater than 150. */
	bool ShouldMoveCheck() const;

	/** Only perform a Rotate In Place Check if the character is Aiming or in First Person. */
	bool CanRotateInPlace() const;

	/**
	 * Only perform a Turn In Place check if the character is looking toward the camera in Third Person,
	 * and if the "Enable Transition" curve is fully weighted. The Enable_Transition curve is modified within certain
	 * states of the AnimBP so that the character can only turn while in those states.
	 */
	bool CanTurnInPlace() const;

	/**
	 * Only perform a Dynamic Transition check if the "Enable Transition" curve is fully weighted.
	 * The Enable_Transition curve is modified within certain states of the AnimBP so
	 * that the character can only transition while in those states.
	 */
	bool CanDynamicTransition() const;

private:
	// ==================== Delaying ==================== //
	
	void PlayDynamicTransitionDelay();
	void OnJumpedDelay();
	void OnPivotDelay();

	// ==================== Update Values ==================== //

	void UpdateAimingValues(float DeltaSeconds);
	void UpdateLayerValues();
	void UpdateFootIK(float DeltaSeconds);
	void UpdateMovementValues(float DeltaSeconds);
	void UpdateRotationValues();
	void UpdateInAirValues(float DeltaSeconds);
	void UpdateRagdollValues();

	// ==================== Foot IK ==================== //

	void SetFootLocking(float DeltaSeconds, FName EnableFootIKCurve, FName FootLockCurve, FName IKFootBone,
                          float& CurFootLockAlpha, bool& UseFootLockCurve,
                          FVector& CurFootLockLoc, FRotator& CurFootLockRot) const;

	void SetFootLockOffsets(float DeltaSeconds, FVector& LocalLoc, FRotator& LocalRot) const;

	void SetPelvisIKOffset(float DeltaSeconds, const FVector& FootOffsetLTarget, const FVector& FootOffsetRTarget);

	void ResetIKOffsets(float DeltaSeconds);

	void SetFootOffsets(float DeltaSeconds, FName EnableFootIKCurve, FName IKFootBone, FName RootBone,
                          FVector& CurLocationTarget, FVector& CurLocationOffset, FRotator& CurRotationOffset) const;

	// ==================== Grounded ==================== //

	void RotateInPlaceCheck();
	void TurnInPlaceCheck(float DeltaSeconds);
	void DynamicTransitionCheck();
	void TurnInPlace(const FRotator& TargetRotation, float PlayRateScale, float StartTime, bool OverrideCurrent);

	FVelocityBlend CalculateVelocityBlend() const;
	
	// ==================== Movement ==================== //

	FVector CalculateRelativeAccelerationAmount() const;

	float CalculateStrideBlend() const;
	float CalculateWalkRunBlend() const;
	float CalculateStandingPlayRate() const;
	float CalculateDiagonalScaleAmount() const;
	float CalculateCrouchingPlayRate() const;
	float CalculateLandPrediction() const;
	
	FLeanAmount CalculateAirLeanAmount() const;

	EMovementDirection CalculateMovementDirection() const;

	// ==================== Util ==================== //

	float GetAnimCurveClamped(const FName& Name, float Bias, float ClampMin, float ClampMax) const;

public:
	// ==================== References ==================== //
	TWeakObjectPtr<AAnonCharacter> Character;

	// ==================== Character Information ==================== //
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information", Meta = (
		ShowOnlyInnerProperties))
	FAnimCharacterInformation CharacterInformation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FMovementState MovementState = EMovementState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FMovementAction MovementAction = EMovementAction::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FRotationMode RotationMode = ERotationMode::VelocityDirection;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FGait Gait = EGait::Walking;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FStance Stance = EStance::Standing;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FOverlayState OverlayState = EOverlayState::Default;

	// ==================== Anim Graph - Grounded ==================== //
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded", Meta = (
		ShowOnlyInnerProperties))
	FAnimGraphGrounded Grounded;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FVelocityBlend VelocityBlend;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FLeanAmount LeanAmount;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FVector RelativeAccelerationAmount = FVector::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FGroundedEntryState GroundedEntryState = EGroundedEntryState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FMovementDirection MovementDirection = EMovementDirection::Forward;

	// ==================== Anim Graph - In Air ==================== //
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - In Air", Meta = (
		ShowOnlyInnerProperties))
	FAnimGraphInAir InAir;

	// ==================== Anim Graph - Aiming Values ==================== //
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Aiming Values", Meta = (
		ShowOnlyInnerProperties))
	FAnimGraphAimingValues AimingValues;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Aiming Values")
	FVector2D SmoothedAimingAngle = FVector2D::ZeroVector;

	// ==================== Anim Graph - Ragdoll ==================== //
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Ragdoll")
	float FlailRate = 0.0f;

	// ==================== Anim Graph - Layer Blending ==================== //
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Layer Blending", Meta = (
		ShowOnlyInnerProperties))
	FAnimGraphLayerBlending LayerBlendingValues;

	// ==================== Anim Graph - Foot IK ==================== //
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Foot IK", Meta = (
		ShowOnlyInnerProperties))
	FAnimGraphFootIK FootIKValues;

	// ==================== Turn In Place ==================== //
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Turn In Place", Meta = (
		ShowOnlyInnerProperties))
	FAnimTurnInPlace TurnInPlaceValues;

	// ==================== Rotate In Place ==================== //
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Rotate In Place", Meta = (
		    ShowOnlyInnerProperties))
	FAnimRotateInPlace RotateInPlace;

	// ==================== Configuration ==================== //
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Main Configuration", Meta = (
		ShowOnlyInnerProperties))
	FAnimConfiguration Config;

	// ==================== Blend Curves ==================== //

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> DiagonalScaleAmountCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_N_Walk;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_N_Run;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_C_Walk;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> LandPredictionCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> LeanInAirCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveVector> YawOffset_FB;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveVector> YawOffset_LR;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Dynamic Transition")
	TObjectPtr<UAnimSequenceBase> TransitionAnim_R;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Dynamic Transition")
	TObjectPtr<UAnimSequenceBase> TransitionAnim_L;

	// ==================== IK Bone Names ==================== //
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Anim Graph - Foot IK")
	FName IkFootL_BoneName = FName(TEXT("ik_foot_l"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Anim Graph - Foot IK")
	FName IkFootR_BoneName = FName(TEXT("ik_foot_r"));

private:
	FTimerHandle OnPivotTimer;

	FTimerHandle PlayDynamicTransitionTimer;

	FTimerHandle OnJumpedTimer;

	bool bCanPlayDynamicTransition = true;
};
