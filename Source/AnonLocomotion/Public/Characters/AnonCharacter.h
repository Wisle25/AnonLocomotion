// Fill out your copyright notice in the Description page of Project Settings.
// ReSharper disable CppRedundantAccessSpecifier

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Data/LocomotionEnum.h"
#include "Data/LocomotionStruct.h"
#include "AnonCharacter.generated.h"

class UTraversalComponent;
class UMotionWarpingComponent;
class AAnonPlayerController;
struct FInputActionValue;
class UAnonCharacterMovement;
class UAnonPlayerCameraBehavior;

DECLARE_MULTICAST_DELEGATE(FJumpPressedSignature);
DECLARE_MULTICAST_DELEGATE(FOnJumpedSignature);
DECLARE_MULTICAST_DELEGATE_OneParam(FRagdollStateChangedSignature, bool);

UCLASS()
class ANONLOCOMOTION_API AAnonCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	explicit AAnonCharacter(const FObjectInitializer& ObjectInitializer);

	// ==================== Lifecycles ==================== //
	
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:
	// ==================== References ==================== //

	TWeakObjectPtr<AAnonPlayerController> AnonPlayerController;
	
	// ==================== Ragdoll System ==================== //
	
	FRagdollStateChangedSignature RagdollStateChangedDelegate;
	
	/** If the skeleton uses a reversed pelvis bone, flip the calculation operator */
	UPROPERTY(EditDefaultsOnly, Category = "ALS|Ragdoll System")
	bool bReversedPelvis = false;

	/** If player hits to the ground with a specified amount of velocity, switch to ragdoll state */
	UPROPERTY(EditDefaultsOnly, Category = "ALS|Ragdoll System")
	bool bRagdollOnLand = true;

	bool bRagdollOnGround = false;
	bool bRagdollFaceUp = false;

	/** If player hits to the ground with an amount of velocity greater than specified value, switch to ragdoll state */
	UPROPERTY(EditDefaultsOnly, Category = "ALS|Ragdoll System", meta = (EditCondition ="bRagdollOnLand"))
	float RagdollOnLandVelocity = 1000.0f;

	FVector LastRagdollVelocity = FVector::ZeroVector;

	UPROPERTY(Replicated)
	FVector TargetRagdollLocation = FVector::ZeroVector;

	/* Server ragdoll pull force storage*/
	float ServerRagdollPull = 0.0f;

	/* Dedicated server mesh default visibility based anim tick option*/
	EVisibilityBasedAnimTickOption DefVisBasedTickOp;

	bool bPreRagdollURO = false;

	/** Get Up Montages */
	UPROPERTY(EditDefaultsOnly, Category="ALS|Ragdoll System")
	TMap<FName, TSoftObjectPtr<UAnimMontage>> GetUpMontage;

	/** Get required get up animation according to character's state */
	UAnimMontage* GetGetUpAnimation(bool bRagdollFaceUpState);
	
	void RagdollStart();
	void RagdollUpdate(float DeltaTime);
	void RagdollEnd();
	
	void SetActorLocationDuringRagdoll(float DeltaTime);

	//-- Ragdoll Replication --//
	
	UFUNCTION(Server, Unreliable)
	void Server_SetMeshLocationDuringRagdoll(FVector MeshLocation);

	void ReplicatedRagdollStart();

	UFUNCTION(Server, Reliable)
	void Server_RagdollStart();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RagdollStart();
	
	void ReplicatedRagdollEnd();

	UFUNCTION(Server, Reliable)
	void Server_RagdollEnd(FVector CharacterLocation);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RagdollEnd(FVector CharacterLocation);
	
	// ==================== Character States ==================== //

	UPROPERTY(ReplicatedUsing = OnRep_OverlayState)
	EOverlayState OverlayState = EOverlayState::Default;
	
	EGroundedEntryState GroundedEntryState;
	
	EMovementState MovementState = EMovementState::None;
	EMovementState PrevMovementState = EMovementState::None;
	
	EMovementAction MovementAction = EMovementAction::None;

	UPROPERTY(ReplicatedUsing = OnRep_RotationMode)
	ERotationMode RotationMode = ERotationMode::LookingDirection;

	UPROPERTY(EditAnywhere, Replicated, Category = "ALS|Character States")
	ERotationMode DesiredRotationMode = ERotationMode::LookingDirection;

	EGait Gait = EGait::Walking;
	
	UPROPERTY(EditAnywhere, Replicated, Category = "ALS|Character States")
	EGait DesiredGait = EGait::Running;

	EStance Stance = EStance::Standing;

	UPROPERTY(EditAnywhere, Replicated, Category = "ALS|Character States")
	EStance DesiredStance = EStance::Standing;
	
	UPROPERTY(ReplicatedUsing = OnRep_ViewMode)
	EViewMode ViewMode = EViewMode::ThirdPerson;
	
	int32 OverlayOverrideState = 0;

public:
	//-- Movement State --//
	
	void SetMovementState(EMovementState NewState, bool bForce = false);
	
	FORCEINLINE EMovementState GetMovementState() const { return MovementState; }
	FORCEINLINE EMovementState GetPrevMovementState() const { return PrevMovementState; }

	//-- Movement Action --//
	
	void SetMovementAction(EMovementAction NewAction, bool bForce = false);
	
	FORCEINLINE EMovementAction GetMovementAction() const { return MovementAction; }

	//-- Stance --//
	
	void SetStance(EStance NewStance, bool bForce = false);
	void SetDesiredStance(EStance NewStance);
	
	UFUNCTION(Server, Reliable)
	void Server_SetDesiredStance(EStance NewStance);
	
	FORCEINLINE EStance GetStance() const { return Stance; }
	FORCEINLINE EStance GetDesiredStance() const { return DesiredStance; }

	//-- Overlay Override --//
	
	void SetOverlayOverrideState(int32 NewState);
	
	FORCEINLINE int32 GetOverlayOverrideState() const { return OverlayOverrideState; }

	//-- Gait --//
	
	void SetGait(EGait NewGait, bool bForce = false);
	void SetDesiredGait(EGait NewGait);

	UFUNCTION(Server, Reliable)
	void Server_SetDesiredGait(EGait NewGait);

	FORCEINLINE EGait GetGait() const { return Gait; }
	FORCEINLINE EGait GetDesiredGait() const { return DesiredGait; }

	//-- Rotation Mode --//
	
	void SetRotationMode(ERotationMode NewRotationMode, bool bForce = false);
    void SetDesiredRotationMode(ERotationMode NewRotMode);

	UFUNCTION(Server, Reliable)
	void Server_SetRotationMode(ERotationMode NewRotationMode, bool bForce);
	
	FORCEINLINE ERotationMode GetRotationMode() const { return RotationMode; }
    FORCEINLINE ERotationMode GetDesiredRotationMode() const { return DesiredRotationMode; }

    UFUNCTION(Server, Reliable)
    void Server_SetDesiredRotationMode(ERotationMode NewRotMode);
	
	//-- View Mode --//
	
	void SetViewMode(EViewMode NewViewMode, bool bForce = false);

	UFUNCTION(Server, Reliable)
	void Server_SetViewMode(EViewMode NewViewMode, bool bForce);
	
	EViewMode GetViewMode() const { return ViewMode; }

	//-- Overlay State --//
	
	void SetOverlayState(EOverlayState NewState, bool bForce = false);
	void SetGroundedEntryState(EGroundedEntryState NewState);

	UFUNCTION(Server, Reliable)
	void Server_SetOverlayState(EOverlayState NewState, bool bForce);
	
	FORCEINLINE EOverlayState GetOverlayState() const { return OverlayState; }
	FORCEINLINE EGroundedEntryState GetGroundedEntryState() const { return GroundedEntryState; }

	// ==================== Jumping ==================== //

public:
	FJumpPressedSignature JumpPressedDelegate;
	FOnJumpedSignature OnJumpedDelegate;
	
protected:
	void EventOnLanded();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnLanded();

	void EventOnJumped();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnJumped();

	virtual void OnJumped_Implementation() override;
	virtual void Landed(const FHitResult& Hit) override;

	void OnLandFrictionReset() const;

	// ==================== General Montage ==================== //
	
	UFUNCTION(Server, Reliable)
	void Server_PlayMontage(UAnimMontage* Montage, float PlayRate);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMontage(UAnimMontage* Montage, float PlayRate);
	
	void Replicated_PlayMontage(UAnimMontage* Montage, float PlayRate);
	
protected:
	// ==================== Rotation System ==================== //

	FRotator TargetRotation = FRotator::ZeroRotator;
	FRotator InAirRotation = FRotator::ZeroRotator;

	float YawOffset = 0.0f;
	
	float CalculateGroundedRotationRate() const;
	
	void SetActorLocationAndTargetRotation(const FVector& NewLocation, const FRotator& NewRotation);
	void SmoothCharacterRotation(const FRotator& Target, float TargetInterpSpeed, float ActorInterpSpeed, float DeltaTime);
	void LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed, float DeltaTime);

protected:
	// ==================== Breakfall System ==================== //

	/** If player hits to the ground with a specified amount of velocity, switch to breakfall state */
	UPROPERTY(EditDefaultsOnly, Category = "ALS|Breakfall System")
	bool bBreakfallOnLand = true;

	/** If player hits to the ground with an amount of velocity greater than specified value, switch to breakfall state */
	UPROPERTY(EditDefaultsOnly, Category = "ALS|Breakfall System", meta = (EditCondition ="bBreakfallOnLand"))
	float BreakfallOnLandVelocity = 700.0f;
	
	/** BP implementable function that called when Breakfall starts */
	void OnBreakfall();

protected:
	// ==================== Movement System ==================== //

	UPROPERTY()
	TObjectPtr<UAnonCharacterMovement> AnonCharacterMovement;

	UPROPERTY(EditDefaultsOnly, Category="ALS|Movement System")
	TMap<FName, TSoftObjectPtr<UAnimMontage>> RollMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "ALS|Movement System")
	FDataTableRowHandle MovementModel;
	
	FMovementStateSettings MovementData;

	FMovementSettings GetTargetMovementSettings() const;

	EGait GetAllowedGait() const;
	EGait GetActualGait(EGait AllowedGait) const;

	bool CanSprint() const;
	
	void SetMovementModel();
	void UpdateCharacterMovement();
	void UpdateGroundedRotation(float DeltaTime);
	void UpdateInAirRotation(float DeltaTime);
	
public:
	
	/** Implement on BP to get required roll animation according to character's state */
	UAnimMontage* GetRollAnimation();

protected:
	// ==================== Utility ==================== //

	float GetAnimCurveValue(FName CurveName) const;

	void SetVisibleMesh(USkeletalMesh* NewSkeletalMesh);

	UFUNCTION(Server, Reliable)
	void Server_SetVisibleMesh(USkeletalMesh* NewSkeletalMesh);

protected:
	// ==================== Camera System ==================== //
	
	UPROPERTY(EditDefaultsOnly, Category = "ALS|Camera System")
	float ThirdPersonFOV = 90.0f;

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Camera System")
	float FirstPersonFOV = 90.0f;

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Camera System")
	bool bRightShoulder = false;

public:
	void SetRightShoulder(bool bNewRightShoulder);
	
	FORCEINLINE bool IsRightShoulder() const { return bRightShoulder; }
	
	virtual ECollisionChannel GetThirdPersonTraceParams(FVector& TraceOrigin, float& TraceRadius);
	virtual FTransform GetThirdPersonPivotTarget();
	virtual FVector GetFirstPersonCameraTarget();
	
	FORCEINLINE void SetCameraBehavior(UAnonPlayerCameraBehavior* CamBeh) { CameraBehavior = CamBeh; }
	FORCEINLINE void GetCameraParameters(float& TP_FOVOut, float& FP_FOVOut, bool& bRightShoulderOut) const
	{
		TP_FOVOut = ThirdPersonFOV;
		FP_FOVOut = FirstPersonFOV;
		bRightShoulderOut = bRightShoulder;
	}

protected:
	// ==================== Essential Information Getters/Setters ==================== //

	FVector Acceleration = FVector::ZeroVector;

	bool bIsMoving = false;
	bool bHasMovementInput = false;

	FRotator LastVelocityRotation;
	FRotator LastMovementInputRotation;

	float Speed = 0.0f;
	float MovementInputAmount = 0.0f;
	float AimYawRate = 0.0f;

	void SetEssentialValues(float DeltaTime);
	
public:
	FORCEINLINE bool IsMoving() const { return bIsMoving; }
	FORCEINLINE bool HasMovementInput() const { return bHasMovementInput; }
	
	FORCEINLINE float GetAimYawRate() const { return AimYawRate; }
	FORCEINLINE float GetSpeed() const { return Speed; }
	FORCEINLINE float GetMovementInputAmount() const { return MovementInputAmount; }
	
	FORCEINLINE FVector GetAcceleration() const { return Acceleration; }
	FORCEINLINE FVector GetMovementInput() const { return ReplicatedCurrentAcceleration; }

	FORCEINLINE FRotator GetAimingRotation() const { return AimingRotation; }

protected:
	// ==================== Input ==================== //
	
	UPROPERTY(EditDefaultsOnly, Category = "ALS|Input")
	float LookUpDownRate = 1.25f;

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Input")
	float LookLeftRightRate = 1.25f;

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Input")
	float RollDoubleTapTimeout = 0.3f;

	bool bBreakFall = false;
	bool bSprintHeld = false;

public:
	UFUNCTION()
	void MoveAction(const FInputActionValue& InputValue);
	
	UFUNCTION()
	void LookAction(const FInputActionValue& InputValue);
	
	UFUNCTION()
	void JumpAction(const FInputActionValue& InputValue);
	
	UFUNCTION()
	void SprintAction(const FInputActionValue& InputValue);
	
	UFUNCTION()
	void AimAction(const FInputActionValue& InputValue);
	
	UFUNCTION()
	void CameraTapAction();
	
	UFUNCTION()
	void CameraHeldAction();
	
	UFUNCTION()
	void StanceAction();
	
	UFUNCTION()
	void WalkAction();
	
	UFUNCTION()
	void RagdollAction();
	
	UFUNCTION()
	void VelocityDirectionAction();
	
	UFUNCTION()
	void LookingDirectionAction();

	UFUNCTION()
	void ChangeOverlayAction();

protected:
	// ==================== State Changes ==================== //

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;
	virtual void OnMovementStateChanged(EMovementState PreviousState);
	virtual void OnMovementActionChanged(EMovementAction PreviousAction);
	virtual void OnStanceChanged(EStance PreviousStance);
	virtual void OnRotationModeChanged(ERotationMode PreviousRotationMode);
	virtual void OnGaitChanged(EGait PreviousGait);
	virtual void OnViewModeChanged(EViewMode PreviousViewMode);
	virtual void OnOverlayStateChanged(EOverlayState PreviousState);
	virtual void OnVisibleMeshChanged(const USkeletalMesh* PreviousSkeletalMesh);
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	void ForceUpdateCharacterState();
	
	//-- Replication --//
	
	float EasedMaxAcceleration = 0.0f;

	UPROPERTY(Replicated)
	FVector ReplicatedCurrentAcceleration = FVector::ZeroVector;

	UPROPERTY(Replicated)
	FRotator ReplicatedControlRotation = FRotator::ZeroRotator;

	/** Replicated Skeletal Mesh Information*/
	UPROPERTY(EditAnywhere, Category = "ALS|Skeletal Mesh", ReplicatedUsing = OnRep_VisibleMesh)
	TObjectPtr<USkeletalMesh> VisibleMesh;
	
	UFUNCTION(Category = "ALS|Replication")
	void OnRep_RotationMode(ERotationMode PrevRotMode);

	UFUNCTION(Category = "ALS|Replication")
	void OnRep_ViewMode(EViewMode PrevViewMode);

	UFUNCTION(Category = "ALS|Replication")
	void OnRep_OverlayState(EOverlayState PrevOverlayState);

	UFUNCTION(Category = "ALS|Replication")
	void OnRep_VisibleMesh(const USkeletalMesh* PreviousSkeletalMesh);
	
	//-- Cached Variables --//

	FVector PreviousVelocity = FVector::ZeroVector;

	float PreviousAimYaw = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Camera")
	TObjectPtr<UAnonPlayerCameraBehavior> CameraBehavior;

	/** Last time the 'first' crouch/roll button is pressed */
	float LastStanceInputTime = 0.0f;

	/* Timer to manage reset of braking friction factor after on landed event */
	FTimerHandle OnLandedFrictionResetTimer;

	/* Smooth out aiming by interping control rotation*/
	FRotator AimingRotation = FRotator::ZeroRotator;

	/** We won't use curve based movement and a few other features on networked games */
	bool bEnableNetworkOptimizations = false;

protected:
	// ==================== Components ==================== //

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UMotionWarpingComponent> MotionWarping;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTraversalComponent> Traversal;
};
