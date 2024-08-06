// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/AnonCharacter.h"

#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Camera/AnonPlayerCameraBehavior.h"
#include "Components/AnonCharacterMovement.h"
#include "Components/CapsuleComponent.h"
#include "Controller/AnonPlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NavAreas/NavArea_Obstacle.h"
#include "Net/UnrealNetwork.h"

const FName NAME_FP_Camera(TEXT("FP_Camera"));
const FName NAME_Pelvis(TEXT("Pelvis"));
const FName NAME_RagdollPose(TEXT("RagdollPose"));
const FName NAME_RotationAmount(TEXT("RotationAmount"));
const FName NAME_YawOffset(TEXT("YawOffset"));
const FName NAME_pelvis(TEXT("pelvis"));
const FName NAME_root(TEXT("root"));
const FName NAME_spine_03(TEXT("spine_03"));

AAnonCharacter::AAnonCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UAnonCharacterMovement>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = 0;
	bReplicates = true;
	SetReplicatingMovement(true);

	/** Capsule Component **/
	GetCapsuleComponent()->SetAreaClassOverride(UNavArea_Obstacle::StaticClass());

	/** Movement **/
	GetCharacterMovement()->MaxAcceleration = 1500.f;
	GetCharacterMovement()->BrakingFrictionFactor = 0.f;
	GetCharacterMovement()->SetCrouchedHalfHeight(60.f);
	GetCharacterMovement()->MinAnalogWalkSpeed = 25.f;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	GetCharacterMovement()->AirControl = .15f;
	GetCharacterMovement()->BrakingDecelerationFlying = 1000.f;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->NavAgentProps.bCanFly = true;
}
// ==================== Lifecycles ==================== //

void AAnonCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	AnonCharacterMovement = Cast<UAnonCharacterMovement>(Super::GetMovementComponent());
}

void AAnonCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAnonCharacter, TargetRagdollLocation);
	DOREPLIFETIME_CONDITION(AAnonCharacter, ReplicatedCurrentAcceleration, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AAnonCharacter, ReplicatedControlRotation, COND_SkipOwner);

	DOREPLIFETIME(AAnonCharacter, DesiredGait);
	DOREPLIFETIME_CONDITION(AAnonCharacter, DesiredStance, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AAnonCharacter, DesiredRotationMode, COND_SkipOwner);

	DOREPLIFETIME_CONDITION(AAnonCharacter, RotationMode, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AAnonCharacter, OverlayState, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AAnonCharacter, ViewMode, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AAnonCharacter, VisibleMesh, COND_SkipOwner);
}

void AAnonCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AnonPlayerController = Cast<AAnonPlayerController>(NewController);
}

void AAnonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->ClearActionEventBindings();
		EnhancedInputComponent->ClearActionValueBindings();
		EnhancedInputComponent->ClearDebugKeyBindings();
		
		const TArray<FEnhancedActionKeyMapping>& Mappings = AnonPlayerController->GetDefaultInput()->GetMappings();
		
		// There may be more than one keymapping assigned to one action. So, first filter duplicate action entries to prevent multiple delegate bindings
		TSet<const UInputAction*> UniqueActions;
		for (const FEnhancedActionKeyMapping& Keymapping : Mappings)
		{
			UniqueActions.Add(Keymapping.Action);
		}
		for (const UInputAction* UniqueAction : UniqueActions)
		{
			EnhancedInputComponent->BindAction(UniqueAction, ETriggerEvent::Triggered, this, UniqueAction->GetFName());
		}
	}
}

void AAnonCharacter::BeginPlay()
{
	Super::BeginPlay();

	// If we're in networked game, disable curved movement
	bEnableNetworkOptimizations = !IsNetMode(NM_Standalone);

	// Make sure the mesh and animbp update after the CharacterBP to ensure it gets the most recent values.
	GetMesh()->AddTickPrerequisiteActor(this);

	// Set the Movement Model
	SetMovementModel();

	// Force update states to use the initial desired values.
	ForceUpdateCharacterState();

	if (Stance == EStance::Standing)
	{
		UnCrouch();
	}
	else if (Stance == EStance::Crouching)
	{
		Crouch();
	}

	// Set default rotation values.
	TargetRotation = GetActorRotation();
	LastVelocityRotation = TargetRotation;
	LastMovementInputRotation = TargetRotation;

	if (GetMesh()->GetAnimInstance() && GetLocalRole() == ROLE_SimulatedProxy)
	{
		GetMesh()->GetAnimInstance()->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
	}

	AnonCharacterMovement->SetMovementSettings(GetTargetMovementSettings());
}

void AAnonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Set required values
	SetEssentialValues(DeltaTime);

	if (MovementState == EMovementState::Grounded)
	{
		UpdateCharacterMovement();
		UpdateGroundedRotation(DeltaTime);
	}
	else if (MovementState == EMovementState::InAir)
	{
		UpdateInAirRotation(DeltaTime);
	}
	else if (MovementState == EMovementState::Ragdoll)
	{
		RagdollUpdate(DeltaTime);
	}

	// Cache values
	PreviousVelocity = GetVelocity();
	PreviousAimYaw = AimingRotation.Yaw;
}

// ==================== Ragdoll System ==================== //

UAnimMontage* AAnonCharacter::GetGetUpAnimation(bool bRagdollFaceUpState)
{
	FName GetUpName;

	if (bRagdollFaceUpState)
	{
		if (OverlayState == EOverlayState::Default || OverlayState == EOverlayState::Masculine || OverlayState == EOverlayState::Feminine)
		{
			GetUpName = "BackDefault";
		}
	
		if (OverlayState == EOverlayState::Injured || OverlayState == EOverlayState::Bow || OverlayState == EOverlayState::Torch || OverlayState == EOverlayState::Barrel)
		{
			GetUpName = "BackLH";
		}

		if (OverlayState == EOverlayState::HandsTied || OverlayState == EOverlayState::Box)
		{
			GetUpName = "Back2H";
		}

		if (OverlayState == EOverlayState::Rifle || OverlayState == EOverlayState::PistolOneHanded || OverlayState == EOverlayState::PistolTwoHanded || OverlayState == EOverlayState::Binoculars)
		{
			GetUpName = "BackRH";
		}
	}
	else
	{
		if (OverlayState == EOverlayState::Default || OverlayState == EOverlayState::Masculine || OverlayState == EOverlayState::Feminine)
		{
			GetUpName = "FrontDefault";
		}
	
		if (OverlayState == EOverlayState::Injured || OverlayState == EOverlayState::Bow || OverlayState == EOverlayState::Torch || OverlayState == EOverlayState::Barrel)
		{
			GetUpName = "FrontLH";
		}

		if (OverlayState == EOverlayState::HandsTied || OverlayState == EOverlayState::Box)
		{
			GetUpName = "Front2H";
		}

		if (OverlayState == EOverlayState::Rifle || OverlayState == EOverlayState::PistolOneHanded || OverlayState == EOverlayState::PistolTwoHanded || OverlayState == EOverlayState::Binoculars)
		{
			GetUpName = "FrontRH";
		}
	}
	
	return GetUpMontage.Contains(GetUpName) ? GetUpMontage[GetUpName].LoadSynchronous() : nullptr;
}

void AAnonCharacter::RagdollStart()
{
	if (RagdollStateChangedDelegate.IsBound())
	{
		RagdollStateChangedDelegate.Broadcast(true);
	}

	/** When Networked, disables replicate movement reset TargetRagdollLocation and ServerRagdollPull variable
	and if the host is a dedicated server, change character mesh optimisation option to avoid z-location bug*/
	AnonCharacterMovement->bIgnoreClientMovementErrorChecksAndCorrection = 1;

	if (UKismetSystemLibrary::IsDedicatedServer(GetWorld()))
	{
		DefVisBasedTickOp = GetMesh()->VisibilityBasedAnimTickOption;
		GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}
	TargetRagdollLocation = GetMesh()->GetSocketLocation(NAME_Pelvis);
	ServerRagdollPull = 0;

	// Disable URO
	bPreRagdollURO = GetMesh()->bEnableUpdateRateOptimizations;
	GetMesh()->bEnableUpdateRateOptimizations = false;

	// Step 1: Clear the Character Movement Mode and set the Movement State to Ragdoll
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	SetMovementState(EMovementState::Ragdoll);

	// Step 2: Disable capsule collision and enable mesh physics simulation starting from the pelvis.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionObjectType(ECC_PhysicsBody);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetAllBodiesBelowSimulatePhysics(NAME_Pelvis, true, true);

	// Step 3: Stop any active montages.
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Stop(0.2f);
	}

	// Fixes character mesh is showing default A pose for a split-second just before ragdoll ends in listen server games
	GetMesh()->bOnlyAllowAutonomousTickPose = true;

	SetReplicateMovement(false);
}

void AAnonCharacter::RagdollUpdate(float DeltaTime)
{
	GetMesh()->bOnlyAllowAutonomousTickPose = false;

	// Set the Last Ragdoll Velocity.
	const FVector NewRagdollVel = GetMesh()->GetPhysicsLinearVelocity(NAME_root);
	LastRagdollVelocity = (NewRagdollVel != FVector::ZeroVector || IsLocallyControlled())
		                      ? NewRagdollVel
		                      : LastRagdollVelocity / 2;

	// Use the Ragdoll Velocity to scale the ragdoll's joint strength for physical animation.
	const float SpringValue = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 1000.0f}, {0.0f, 25000.0f},
	                                                            LastRagdollVelocity.Size());
	GetMesh()->SetAllMotorsAngularDriveParams(SpringValue, 0.0f, 0.0f, false);

	// Disable Gravity if falling faster than -4000 to prevent continual acceleration.
	// This also prevents the ragdoll from going through the floor.
	const bool bEnableGrav = LastRagdollVelocity.Z > -4000.0f;
	GetMesh()->SetEnableGravity(bEnableGrav);

	// Update the Actor location to follow the ragdoll.
	SetActorLocationDuringRagdoll(DeltaTime);
}

void AAnonCharacter::SetActorLocationDuringRagdoll(float DeltaTime)
{
	if (IsLocallyControlled())
	{
		// Set the pelvis as the target location.
		TargetRagdollLocation = GetMesh()->GetSocketLocation(NAME_Pelvis);
		if (!HasAuthority())
		{
			Server_SetMeshLocationDuringRagdoll(TargetRagdollLocation);
		}
	}

	// Determine whether the ragdoll is facing up or down and set the target rotation accordingly.
	const FRotator PelvisRot = GetMesh()->GetSocketRotation(NAME_Pelvis);

	if (bReversedPelvis) {
		bRagdollFaceUp = PelvisRot.Roll > 0.0f;
	} else
	{
		bRagdollFaceUp = PelvisRot.Roll < 0.0f;
	}


	const FRotator TargetRagdollRotation(0.0f, bRagdollFaceUp ? PelvisRot.Yaw - 180.0f : PelvisRot.Yaw, 0.0f);

	// Trace downward from the target location to offset the target location,
	// preventing the lower half of the capsule from going through the floor when the ragdoll is laying on the ground.
	const FVector TraceVect(TargetRagdollLocation.X, TargetRagdollLocation.Y,
	                        TargetRagdollLocation.Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult HitResult;
	World->LineTraceSingleByChannel(HitResult, TargetRagdollLocation, TraceVect,
	                                                  ECC_Visibility, Params);
	
	bRagdollOnGround = HitResult.IsValidBlockingHit();
	FVector NewRagdollLoc = TargetRagdollLocation;

	if (bRagdollOnGround)
	{
		const float ImpactDistZ = FMath::Abs(HitResult.ImpactPoint.Z - HitResult.TraceStart.Z);
		NewRagdollLoc.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - ImpactDistZ + 2.0f;
	}
	if (!IsLocallyControlled())
	{
		ServerRagdollPull = FMath::FInterpTo(ServerRagdollPull, 750.0f, DeltaTime, 0.6f);
		float RagdollSpeed = FVector(LastRagdollVelocity.X, LastRagdollVelocity.Y, 0).Size();
		FName RagdollSocketPullName = RagdollSpeed > 300 ? NAME_spine_03 : NAME_pelvis;
		GetMesh()->AddForce(
			(TargetRagdollLocation - GetMesh()->GetSocketLocation(RagdollSocketPullName)) * ServerRagdollPull,
			RagdollSocketPullName, true);
	}
	SetActorLocationAndTargetRotation(bRagdollOnGround ? NewRagdollLoc : TargetRagdollLocation, TargetRagdollRotation);
}

void AAnonCharacter::Server_RagdollStart_Implementation()
{
	Multicast_RagdollStart();
}

void AAnonCharacter::Multicast_RagdollStart_Implementation()
{
	RagdollStart();
}

void AAnonCharacter::Server_RagdollEnd_Implementation(FVector CharacterLocation)
{
	Multicast_RagdollEnd(CharacterLocation);
}

void AAnonCharacter::Multicast_RagdollEnd_Implementation(FVector CharacterLocation)
{
	RagdollEnd();
}

void AAnonCharacter::RagdollEnd()
{
	/** Re-enable Replicate Movement and if the host is a dedicated server set mesh visibility based anim
	tick option back to default*/

	if (UKismetSystemLibrary::IsDedicatedServer(GetWorld()))
	{
		GetMesh()->VisibilityBasedAnimTickOption = DefVisBasedTickOp;
	}

	GetMesh()->bEnableUpdateRateOptimizations = bPreRagdollURO;

	// Revert back to default settings
	AnonCharacterMovement->bIgnoreClientMovementErrorChecksAndCorrection = 0;
	GetMesh()->bOnlyAllowAutonomousTickPose = false;
	SetReplicateMovement(true);

	// Step 1: Save a snapshot of the current Ragdoll Pose for use in AnimGraph to blend out of the ragdoll
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->SavePoseSnapshot(NAME_RagdollPose);
	}

	// Step 2: If the ragdoll is on the ground, set the movement mode to walking and play a GetUp animation.
	// If not, set the movement mode to falling and update the character movement velocity to match the last ragdoll velocity.
	if (bRagdollOnGround)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		if (GetMesh()->GetAnimInstance())
		{
			GetMesh()->GetAnimInstance()->Montage_Play(GetGetUpAnimation(bRagdollFaceUp), 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		}
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		GetCharacterMovement()->Velocity = LastRagdollVelocity;
	}

	// Step 3: Re-Enable capsule collision, and disable physics simulation on the mesh.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetAllBodiesSimulatePhysics(false);

	if (RagdollStateChangedDelegate.IsBound())
	{
		RagdollStateChangedDelegate.Broadcast(false);
	}
}

void AAnonCharacter::ReplicatedRagdollStart()
{
	if (HasAuthority())
	{
		Multicast_RagdollStart();
	}
	else
	{
		Server_RagdollStart();
	}
}

void AAnonCharacter::ReplicatedRagdollEnd()
{
	if (HasAuthority())
	{
		Multicast_RagdollEnd(GetActorLocation());
	}
	else
	{
		Server_RagdollEnd(GetActorLocation());
	}
}

void AAnonCharacter::Server_SetMeshLocationDuringRagdoll_Implementation(FVector MeshLocation)
{
	TargetRagdollLocation = MeshLocation;
}

// ==================== Character States ==================== //

void AAnonCharacter::SetMovementState(const EMovementState NewState, bool bForce)
{
	if (bForce || MovementState != NewState)
	{
		PrevMovementState = MovementState;
		MovementState = NewState;
		OnMovementStateChanged(PrevMovementState);
	}
}

void AAnonCharacter::SetMovementAction(const EMovementAction NewAction, bool bForce)
{
	if (bForce || MovementAction != NewAction)
	{
		const EMovementAction Prev = MovementAction;
		MovementAction = NewAction;
		OnMovementActionChanged(Prev);
	}
}

void AAnonCharacter::SetStance(const EStance NewStance, bool bForce)
{
	if (bForce || Stance != NewStance)
	{
		const EStance Prev = Stance;
		Stance = NewStance;
		OnStanceChanged(Prev);
	}
}

void AAnonCharacter::SetDesiredStance(EStance NewStance)
{
	DesiredStance = NewStance;
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetDesiredStance(NewStance);
	}
}

void AAnonCharacter::Server_SetDesiredStance_Implementation(EStance NewStance)
{
	SetDesiredStance(NewStance);
}

void AAnonCharacter::SetOverlayOverrideState(int32 NewState)
{
	OverlayOverrideState = NewState;
}

void AAnonCharacter::SetGait(const EGait NewGait, bool bForce)
{
	if (bForce || Gait != NewGait)
	{
		const EGait Prev = Gait;
		Gait = NewGait;
		OnGaitChanged(Prev);
	}
}

void AAnonCharacter::SetDesiredGait(const EGait NewGait)
{
	DesiredGait = NewGait;
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetDesiredGait(NewGait);
	}
}

void AAnonCharacter::Server_SetDesiredGait_Implementation(EGait NewGait)
{
	SetDesiredGait(NewGait);
}

void AAnonCharacter::SetDesiredRotationMode(ERotationMode NewRotMode)
{
	DesiredRotationMode = NewRotMode;
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetDesiredRotationMode(NewRotMode);
	}
}

void AAnonCharacter::Server_SetDesiredRotationMode_Implementation(ERotationMode NewRotMode)
{
	SetDesiredRotationMode(NewRotMode);
}

void AAnonCharacter::SetRotationMode(const ERotationMode NewRotationMode, bool bForce)
{
	if (bForce || RotationMode != NewRotationMode)
	{
		const ERotationMode Prev = RotationMode;
		RotationMode = NewRotationMode;
		OnRotationModeChanged(Prev);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetRotationMode(NewRotationMode, bForce);
		}
	}
}

void AAnonCharacter::Server_SetRotationMode_Implementation(ERotationMode NewRotationMode, bool bForce)
{
	SetRotationMode(NewRotationMode, bForce);
}

void AAnonCharacter::SetViewMode(const EViewMode NewViewMode, bool bForce)
{
	if (bForce || ViewMode != NewViewMode)
	{
		const EViewMode Prev = ViewMode;
		ViewMode = NewViewMode;
		OnViewModeChanged(Prev);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetViewMode(NewViewMode, bForce);
		}
	}
}

void AAnonCharacter::Server_SetViewMode_Implementation(EViewMode NewViewMode, bool bForce)
{
	SetViewMode(NewViewMode, bForce);
}

void AAnonCharacter::SetOverlayState(const EOverlayState NewState, bool bForce)
{
	if (bForce || OverlayState != NewState)
	{
		const EOverlayState Prev = OverlayState;
		OverlayState = NewState;
		OnOverlayStateChanged(Prev);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetOverlayState(NewState, bForce);
		}
	}
}

void AAnonCharacter::SetGroundedEntryState(EGroundedEntryState NewState)
{
	GroundedEntryState = NewState;
}

void AAnonCharacter::Server_SetOverlayState_Implementation(EOverlayState NewState, bool bForce)
{
	SetOverlayState(NewState, bForce);
}

// ==================== Jumping ==================== //

void AAnonCharacter::EventOnLanded()
{
	const float VelZ = FMath::Abs(GetCharacterMovement()->Velocity.Z);

	if (bRagdollOnLand && VelZ > RagdollOnLandVelocity)
	{
		ReplicatedRagdollStart();
	}
	else if (bBreakfallOnLand && bHasMovementInput && VelZ >= BreakfallOnLandVelocity)
	{
		OnBreakfall();
	}
	else
	{
		GetCharacterMovement()->BrakingFrictionFactor = bHasMovementInput ? 0.5f : 3.0f;

		// After 0.5 secs, reset braking friction factor to zero
		GetWorldTimerManager().SetTimer(OnLandedFrictionResetTimer, this,
		                                &AAnonCharacter::OnLandFrictionReset, 0.5f, false);
	}
}

void AAnonCharacter::Multicast_OnLanded_Implementation()
{
	if (!IsLocallyControlled())
	{
		EventOnLanded();
	}
}

void AAnonCharacter::EventOnJumped()
{
	// Set the new In Air Rotation to the velocity rotation if speed is greater than 100.
	InAirRotation = Speed > 100.0f ? LastVelocityRotation : GetActorRotation();

	OnJumpedDelegate.Broadcast();
}

void AAnonCharacter::Multicast_OnJumped_Implementation()
{
	if (!IsLocallyControlled())
	{
		EventOnJumped();
	}
}

void AAnonCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	if (IsLocallyControlled())
	{
		EventOnJumped();
	}
	if (HasAuthority())
	{
		Multicast_OnJumped();
	}
}

void AAnonCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (IsLocallyControlled())
	{
		EventOnLanded();
	}
	if (HasAuthority())
	{
		Multicast_OnLanded();
	}
}

void AAnonCharacter::OnLandFrictionReset() const
{
	// Reset the braking friction
	GetCharacterMovement()->BrakingFrictionFactor = 0.0f;
}

// ==================== General Montage ==================== //

void AAnonCharacter::Server_PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(Montage, PlayRate);
	}

	ForceNetUpdate();
	Multicast_PlayMontage(Montage, PlayRate);
}

void AAnonCharacter::Multicast_PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	if (GetMesh()->GetAnimInstance() && !IsLocallyControlled())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(Montage, PlayRate);
	}
}

void AAnonCharacter::Replicated_PlayMontage(UAnimMontage* Montage, float PlayRate)
{
	// Roll: Simply play a Root Motion Montage.
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(Montage, PlayRate);
	}

	Server_PlayMontage(Montage, PlayRate);
}

// ==================== Rotation System ==================== //

void AAnonCharacter::SmoothCharacterRotation(const FRotator& Target, float TargetInterpSpeed, float ActorInterpSpeed,
											 float DeltaTime)
{
	// Interpolate the Target Rotation for extra smooth rotation behavior
	TargetRotation =
		FMath::RInterpConstantTo(TargetRotation, Target, DeltaTime, TargetInterpSpeed);
	SetActorRotation(
		FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, ActorInterpSpeed));
}

float AAnonCharacter::CalculateGroundedRotationRate() const
{
	// Calculate the rotation rate by using the current Rotation Rate Curve in the Movement Settings.
	// Using the curve in conjunction with the mapped speed gives you a high level of control over the rotation
	// rates for each speed. Increase the speed if the camera is rotating quickly for more responsive rotation.

	const float MappedSpeedVal = AnonCharacterMovement->GetMappedSpeed();
	const float CurveVal =
		AnonCharacterMovement->CurrentMovementSettings.RotationRateCurve->GetFloatValue(MappedSpeedVal);
	const float ClampedAimYawRate = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 300.0f}, {1.0f, 3.0f}, AimYawRate);
	return CurveVal * ClampedAimYawRate;
}

void AAnonCharacter::LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed, float DeltaTime)
{
	// Prevent the character from rotating past a certain angle.
	FRotator Delta = AimingRotation - GetActorRotation();
	Delta.Normalize();
	const float RangeVal = Delta.Yaw;

	if (RangeVal < AimYawMin || RangeVal > AimYawMax)
	{
		const float ControlRotYaw = AimingRotation.Yaw;
		const float TargetYaw = ControlRotYaw + (RangeVal > 0.0f ? AimYawMin : AimYawMax);
		SmoothCharacterRotation({0.0f, TargetYaw, 0.0f}, 0.0f, InterpSpeed, DeltaTime);
	}
}

void AAnonCharacter::SetActorLocationAndTargetRotation(const FVector& NewLocation, const FRotator& NewRotation)
{
	SetActorLocationAndRotation(NewLocation, NewRotation);
	TargetRotation = NewRotation;
}

// ==================== Breakfall System ==================== //

void AAnonCharacter::OnBreakfall()
{
	Replicated_PlayMontage(GetRollAnimation(), 1.35);
}

// ==================== Movement System ==================== //

FMovementSettings AAnonCharacter::GetTargetMovementSettings() const
{
	if (RotationMode == ERotationMode::VelocityDirection)
	{
		if (Stance == EStance::Standing)
		{
			return MovementData.VelocityDirection.Standing;
		}
		if (Stance == EStance::Crouching)
		{
			return MovementData.VelocityDirection.Crouching;
		}
	}
	else if (RotationMode == ERotationMode::LookingDirection)
	{
		if (Stance == EStance::Standing)
		{
			return MovementData.LookingDirection.Standing;
		}
		if (Stance == EStance::Crouching)
		{
			return MovementData.LookingDirection.Crouching;
		}
	}
	else if (RotationMode == ERotationMode::Aiming)
	{
		if (Stance == EStance::Standing)
		{
			return MovementData.Aiming.Standing;
		}
		if (Stance == EStance::Crouching)
		{
			return MovementData.Aiming.Crouching;
		}
	}

	// Default to velocity dir standing
	return MovementData.VelocityDirection.Standing;
}

EGait AAnonCharacter::GetAllowedGait() const
{
	// Calculate the Allowed Gait. This represents the maximum Gait the character is currently allowed to be in,
	// and can be determined by the desired gait, the rotation mode, the stance, etc. For example,
	// if you wanted to force the character into a walking state while indoors, this could be done here.

	if (Stance == EStance::Standing)
	{
		if (RotationMode != ERotationMode::Aiming)
		{
			if (DesiredGait == EGait::Sprinting)
			{
				return CanSprint() ? EGait::Sprinting : EGait::Running;
			}
			return DesiredGait;
		}
	}

	// Crouching stance & Aiming rot mode has same behaviour

	if (DesiredGait == EGait::Sprinting)
	{
		return EGait::Running;
	}

	return DesiredGait;
}

EGait AAnonCharacter::GetActualGait(EGait AllowedGait) const
{
	// Get the Actual Gait. This is calculated by the actual movement of the character,  and so it can be different
	// from the desired gait or allowed gait. For instance, if the Allowed Gait becomes walking,
	// the Actual gait will still be running until the character decelerates to the walking speed.

	const float LocWalkSpeed = AnonCharacterMovement->CurrentMovementSettings.WalkSpeed;
	const float LocRunSpeed = AnonCharacterMovement->CurrentMovementSettings.RunSpeed;

	if (Speed > LocRunSpeed + 10.0f)
	{
		if (AllowedGait == EGait::Sprinting)
		{
			return EGait::Sprinting;
		}
		return EGait::Running;
	}

	if (Speed >= LocWalkSpeed + 10.0f)
	{
		return EGait::Running;
	}

	return EGait::Walking;
}

bool AAnonCharacter::CanSprint() const
{
	// Determine if the character is currently able to sprint based on the Rotation mode and current acceleration
	// (input) rotation. If the character is in the Looking Rotation mode, only allow sprinting if there is full
	// movement input, and it is faced forward relative to the camera + or - 50 degrees.

	if (!bHasMovementInput || RotationMode == ERotationMode::Aiming)
	{
		return false;
	}

	const bool bValidInputAmount = MovementInputAmount > 0.9f;

	if (RotationMode == ERotationMode::VelocityDirection)
	{
		return bValidInputAmount;
	}

	if (RotationMode == ERotationMode::LookingDirection)
	{
		const FRotator AccRot = ReplicatedCurrentAcceleration.ToOrientationRotator();
		FRotator Delta = AccRot - AimingRotation;
		Delta.Normalize();

		return bValidInputAmount && FMath::Abs(Delta.Yaw) < 50.0f;
	}

	return false;
}

void AAnonCharacter::SetMovementModel()
{
	const FString ContextString = GetFullName();
	FMovementStateSettings* OutRow =
		MovementModel.DataTable->FindRow<FMovementStateSettings>(MovementModel.RowName, ContextString);
	check(OutRow);
	MovementData = *OutRow;
}

UAnimMontage* AAnonCharacter::GetRollAnimation()
{
	FName RollName;
	
	if (OverlayState == EOverlayState::Default || OverlayState == EOverlayState::Masculine || OverlayState == EOverlayState::Feminine)
	{
		RollName = "FrontDefault";
	}

	if (OverlayState == EOverlayState::Injured || OverlayState == EOverlayState::Bow || OverlayState == EOverlayState::Torch || OverlayState == EOverlayState::Barrel)
	{
		RollName = "FrontLH";
	}

	if (OverlayState == EOverlayState::HandsTied || OverlayState == EOverlayState::Box)
	{
		RollName = "Front2H";
	}

	if (OverlayState == EOverlayState::Rifle || OverlayState == EOverlayState::PistolOneHanded || OverlayState == EOverlayState::PistolTwoHanded || OverlayState == EOverlayState::Binoculars)
	{
		RollName = "FrontRH";
	}

	return RollMontage.Contains(RollName) ? RollMontage[RollName].LoadSynchronous() : nullptr;
}

// ==================== Utility ==================== //

float AAnonCharacter::GetAnimCurveValue(FName CurveName) const
{
	if (GetMesh()->GetAnimInstance())
	{
		return GetMesh()->GetAnimInstance()->GetCurveValue(CurveName);
	}

	return 0.0f;
}

void AAnonCharacter::SetVisibleMesh(USkeletalMesh* NewVisibleMesh)
{
	if (VisibleMesh != NewVisibleMesh)
	{
		const USkeletalMesh* Prev = VisibleMesh;
		VisibleMesh = NewVisibleMesh;
		OnVisibleMeshChanged(Prev);

		if (GetLocalRole() != ROLE_Authority)
		{
			Server_SetVisibleMesh(NewVisibleMesh);
		}
	}
}

void AAnonCharacter::Server_SetVisibleMesh_Implementation(USkeletalMesh* NewVisibleMesh)
{
	SetVisibleMesh(NewVisibleMesh);
}

// ==================== Camera System ==================== //

void AAnonCharacter::SetRightShoulder(bool bNewRightShoulder)
{
	bRightShoulder = bNewRightShoulder;
	if (CameraBehavior)
	{
		CameraBehavior->bRightShoulder = bRightShoulder;
	}
}

ECollisionChannel AAnonCharacter::GetThirdPersonTraceParams(FVector& TraceOrigin, float& TraceRadius)
{
	const FName CameraSocketName = bRightShoulder ? TEXT("TP_CameraTrace_R") : TEXT("TP_CameraTrace_L");
	TraceOrigin = GetMesh()->GetSocketLocation(CameraSocketName);
	TraceRadius = 15.0f;
	
	return ECC_Camera;
}

FTransform AAnonCharacter::GetThirdPersonPivotTarget()
{
	return FTransform(GetActorRotation(),
					  (GetMesh()->GetSocketLocation(TEXT("Head")) + GetMesh()->GetSocketLocation(TEXT("root"))) / 2.0f,
					  FVector::OneVector);
}

FVector AAnonCharacter::GetFirstPersonCameraTarget()
{
	return GetMesh()->GetSocketLocation(TEXT("FP_Camera"));
}

// ==================== Essential Information Getters/Setters ==================== //

void AAnonCharacter::SetEssentialValues(float DeltaTime)
{
	if (GetLocalRole() != ROLE_SimulatedProxy)
	{
		ReplicatedCurrentAcceleration = GetCharacterMovement()->GetCurrentAcceleration();
		ReplicatedControlRotation = GetControlRotation();
		EasedMaxAcceleration = GetCharacterMovement()->GetMaxAcceleration();
	}
	else
	{
		EasedMaxAcceleration = GetCharacterMovement()->GetMaxAcceleration() != 0
			                       ? GetCharacterMovement()->GetMaxAcceleration()
			                       : EasedMaxAcceleration / 2;
	}

	// Interp AimingRotation to current control rotation for smooth character rotation movement. Decrease InterpSpeed
	// for slower but smoother movement.
	AimingRotation = FMath::RInterpTo(AimingRotation, ReplicatedControlRotation, DeltaTime, 30);

	// These values represent how the capsule is moving as well as how it wants to move, and therefore are essential
	// for any data driven animation system. They are also used throughout the system for various functions,
	// so I found it is easiest to manage them all in one place.

	const FVector CurrentVel = GetVelocity();

	// Set the amount of Acceleration.
	const FVector NewAcceleration = (CurrentVel - PreviousVelocity) / DeltaTime;
	Acceleration = NewAcceleration.IsNearlyZero() || IsLocallyControlled() ? NewAcceleration : Acceleration / 2;

	// Determine if the character is moving by getting its speed. The Speed equals the length of the horizontal (x y)
	// velocity, so it does not take vertical movement into account. If the character is moving, update the last
	// velocity rotation. This value is saved because it might be useful to know the last orientation of movement
	// even after the character has stopped.
	Speed = CurrentVel.Size2D();
	bIsMoving = Speed > 1.0f;
	if (bIsMoving)
	{
		LastVelocityRotation = CurrentVel.ToOrientationRotator();
	}

	// Determine if the character has movement input by getting its movement input amount.
	// The Movement Input Amount is equal to the current acceleration divided by the max acceleration so that
	// it has a range of 0-1, 1 being the maximum possible amount of input, and 0 being none.
	// If the character has movement input, update the Last Movement Input Rotation.
	MovementInputAmount = ReplicatedCurrentAcceleration.Size() / EasedMaxAcceleration;
	bHasMovementInput = MovementInputAmount > 0.0f;
	if (bHasMovementInput)
	{
		LastMovementInputRotation = ReplicatedCurrentAcceleration.ToOrientationRotator();
	}

	// Set the Aim Yaw rate by comparing the current and previous Aim Yaw value, divided by Delta Seconds.
	// This represents the speed the camera is rotating left to right.
	AimYawRate = FMath::Abs((AimingRotation.Yaw - PreviousAimYaw) / DeltaTime);
}

void AAnonCharacter::UpdateCharacterMovement()
{
	// Set the Allowed Gait
	const EGait AllowedGait = GetAllowedGait();

	// Determine the Actual Gait. If it is different from the current Gait, Set the new Gait Event.
	const EGait ActualGait = GetActualGait(AllowedGait);

	if (ActualGait != Gait)
	{
		SetGait(ActualGait);
	}

	// Update the Character Max Walk Speed to the configured speeds based on the currently Allowed Gait.
	AnonCharacterMovement->SetAllowedGait(AllowedGait);
}

void AAnonCharacter::UpdateGroundedRotation(float DeltaTime)
{
	if (MovementAction == EMovementAction::None)
	{
		const bool bCanUpdateMovingRot = ((bIsMoving && bHasMovementInput) || Speed > 150.0f) && !HasAnyRootMotion();
		if (bCanUpdateMovingRot)
		{
			const float GroundedRotationRate = CalculateGroundedRotationRate();
			if (RotationMode == ERotationMode::VelocityDirection)
			{
				// Velocity Direction Rotation
				SmoothCharacterRotation({0.0f, LastVelocityRotation.Yaw, 0.0f}, 800.0f, GroundedRotationRate,
				                        DeltaTime);
			}
			else if (RotationMode == ERotationMode::LookingDirection)
			{
				// Looking Direction Rotation
				float YawValue;
				if (Gait == EGait::Sprinting)
				{
					YawValue = LastVelocityRotation.Yaw;
				}
				else
				{
					// Walking or Running...
					const float YawOffsetCurveVal = GetAnimCurveValue(NAME_YawOffset);
					YawValue = AimingRotation.Yaw + YawOffsetCurveVal;
				}
				SmoothCharacterRotation({0.0f, YawValue, 0.0f}, 500.0f, GroundedRotationRate, DeltaTime);
			}
			else if (RotationMode == ERotationMode::Aiming)
			{
				const float ControlYaw = AimingRotation.Yaw;
				SmoothCharacterRotation({0.0f, ControlYaw, 0.0f}, 1000.0f, 20.0f, DeltaTime);
			}
		}
		else
		{
			// Not Moving

			if ((ViewMode == EViewMode::ThirdPerson && RotationMode == ERotationMode::Aiming) ||
				ViewMode == EViewMode::FirstPerson)
			{
				LimitRotation(-100.0f, 100.0f, 20.0f, DeltaTime);
			}

			// Apply the RotationAmount curve from Turn In Place Animations.
			// The Rotation Amount curve defines how much rotation should be applied each frame,
			// and is calculated for animations that are animated at 30fps.

			const float RotAmountCurve = GetAnimCurveValue(NAME_RotationAmount);

			if (FMath::Abs(RotAmountCurve) > 0.001f)
			{
				if (GetLocalRole() == ROLE_AutonomousProxy)
				{
					TargetRotation.Yaw = UKismetMathLibrary::NormalizeAxis(
						TargetRotation.Yaw + (RotAmountCurve * (DeltaTime / (1.0f / 30.0f))));
					SetActorRotation(TargetRotation);
				}
				else
				{
					AddActorWorldRotation({0, RotAmountCurve * (DeltaTime / (1.0f / 30.0f)), 0});
				}
				TargetRotation = GetActorRotation();
			}
		}
	}
	else if (MovementAction == EMovementAction::Rolling)
	{
		// Rolling Rotation (Not allowed on networked games)
		if (!bEnableNetworkOptimizations && bHasMovementInput)
		{
			SmoothCharacterRotation({0.0f, LastMovementInputRotation.Yaw, 0.0f}, 0.0f, 2.0f, DeltaTime);
		}
	}

	// Other actions are ignored...
}

void AAnonCharacter::UpdateInAirRotation(float DeltaTime)
{
	if (RotationMode == ERotationMode::VelocityDirection || RotationMode == ERotationMode::LookingDirection)
	{
		// Velocity / Looking Direction Rotation
		SmoothCharacterRotation({0.0f, InAirRotation.Yaw, 0.0f}, 0.0f, 5.0f, DeltaTime);
	}
	else if (RotationMode == ERotationMode::Aiming)
	{
		// Aiming Rotation
		SmoothCharacterRotation({0.0f, AimingRotation.Yaw, 0.0f}, 0.0f, 15.0f, DeltaTime);
		InAirRotation = GetActorRotation();
	}
}

// ==================== Input ==================== //

void AAnonCharacter::MoveAction(const FInputActionValue& InputValue)
{
	if (MovementState != EMovementState::Grounded && MovementState != EMovementState::InAir) return;

	const FVector2D Value = InputValue.Get<FVector2D>();
	
	// Default camera relative movement behavior
	FRotator AimYawRotation = AimingRotation;
	AimYawRotation.Pitch = AimingRotation.Roll = 0.f;

	const FRotationMatrix AimMatrix = FRotationMatrix(AimYawRotation);
	const FVector ForwardAim = AimMatrix.GetUnitAxis(EAxis::X);
	const FVector RightAim = AimMatrix.GetUnitAxis(EAxis::Y);
	
	AddMovementInput(ForwardAim, Value.Y);
	AddMovementInput(RightAim, Value.X);
}

void AAnonCharacter::LookAction(const FInputActionValue& InputValue)
{
	const FVector2D Value = InputValue.Get<FVector2D>();
	
	AddControllerPitchInput(LookUpDownRate * Value.Y);
	AddControllerYawInput(LookLeftRightRate * Value.X);
}

void AAnonCharacter::JumpAction(const FInputActionValue& InputValue)
{
	const bool bValue = InputValue.Get<bool>();
	
	if (bValue)
	{
		// JumpAction: Press "Jump Action" to end the ragdoll if ragdolling, stand up if crouching, or jump if standing.
		if (JumpPressedDelegate.IsBound())
		{
			JumpPressedDelegate.Broadcast();
		}

		if (MovementAction == EMovementAction::None)
		{
			if (MovementState == EMovementState::Grounded)
			{
				if (Stance == EStance::Standing)
				{
					Jump();
				}
				else if (Stance == EStance::Crouching)
				{
					UnCrouch();
				}
			}
			else if (MovementState == EMovementState::Ragdoll)
			{
				ReplicatedRagdollEnd();
			}
		}
	}
	else
	{
		StopJumping();
	}
}

void AAnonCharacter::SprintAction(const FInputActionValue& InputValue)
{
	const bool bValue = InputValue.Get<bool>();
	
	if (bValue)
	{
		SetDesiredGait(EGait::Sprinting);
	}
	else
	{
		SetDesiredGait(EGait::Running);
	}
}

void AAnonCharacter::AimAction(const FInputActionValue& InputValue)
{
	const bool bValue = InputValue.Get<bool>();
	
	if (bValue)
	{
		// AimAction: Hold "AimAction" to enter the aiming mode, release to revert back the desired rotation mode.
		SetRotationMode(ERotationMode::Aiming);
	}
	else
	{
		if (ViewMode == EViewMode::ThirdPerson)
		{
			SetRotationMode(DesiredRotationMode);
		}
		else if (ViewMode == EViewMode::FirstPerson)
		{
			SetRotationMode(ERotationMode::LookingDirection);
		}
	}
}

void AAnonCharacter::CameraTapAction()
{
	if (ViewMode == EViewMode::FirstPerson)
	{
		// Don't swap shoulders on first person mode
		return;
	}

	// Switch shoulders
	SetRightShoulder(!bRightShoulder);
}

void AAnonCharacter::CameraHeldAction()
{
	// Switch camera mode
	if (ViewMode == EViewMode::FirstPerson)
	{
		SetViewMode(EViewMode::ThirdPerson);
	}
	else if (ViewMode == EViewMode::ThirdPerson)
	{
		SetViewMode(EViewMode::FirstPerson);
	}
}

void AAnonCharacter::StanceAction()
{
	// Stance Action: Press "Stance Action" to toggle Standing / Crouching, double tap to Roll.

	if (MovementAction != EMovementAction::None)
	{
		return;
	}

	UWorld* World = GetWorld();
	check(World);

	const float PrevStanceInputTime = LastStanceInputTime;
	LastStanceInputTime = World->GetTimeSeconds();

	if (LastStanceInputTime - PrevStanceInputTime <= RollDoubleTapTimeout)
	{
		// Roll
		Replicated_PlayMontage(GetRollAnimation(), 1.15f);

		if (Stance == EStance::Standing)
		{
			SetDesiredStance(EStance::Crouching);
		}
		else if (Stance == EStance::Crouching)
		{
			SetDesiredStance(EStance::Standing);
		}
		return;
	}

	if (MovementState == EMovementState::Grounded)
	{
		if (Stance == EStance::Standing)
		{
			SetDesiredStance(EStance::Crouching);
			Crouch();
		}
		else if (Stance == EStance::Crouching)
		{
			SetDesiredStance(EStance::Standing);
			UnCrouch();
		}
	}

	// Notice: MovementState == EMovementState::InAir case is removed
}

void AAnonCharacter::WalkAction()
{
	if (DesiredGait == EGait::Walking)
	{
		SetDesiredGait(EGait::Running);
	}
	else if (DesiredGait == EGait::Running)
	{
		SetDesiredGait(EGait::Walking);
	}
}

void AAnonCharacter::RagdollAction()
{
	// Ragdoll Action: Press "Ragdoll Action" to toggle the ragdoll state on or off.

	if (GetMovementState() == EMovementState::Ragdoll)
	{
		ReplicatedRagdollEnd();
	}
	else
	{
		ReplicatedRagdollStart();
	}
}

void AAnonCharacter::VelocityDirectionAction()
{
	// Select Rotation Mode: Switch the desired (default) rotation mode to Velocity or Looking Direction.
	// This will be the mode the character reverts back to when un-aiming
	SetDesiredRotationMode(ERotationMode::VelocityDirection);
	SetRotationMode(ERotationMode::VelocityDirection);
}

void AAnonCharacter::LookingDirectionAction()
{
	SetDesiredRotationMode(ERotationMode::LookingDirection);
	SetRotationMode(ERotationMode::LookingDirection);
}

void AAnonCharacter::ChangeOverlayAction()
{
	SetOverlayState(static_cast<EOverlayState>(static_cast<int>(OverlayState) + 1));
}

// ==================== State Changes ==================== //

void AAnonCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	// Use the Character Movement Mode changes to set the Movement States to the right values. This allows you to have
	// a custom set of movement states but still use the functionality of the default character movement component.

	if (GetCharacterMovement()->MovementMode == MOVE_Walking ||
		GetCharacterMovement()->MovementMode == MOVE_NavWalking)
	{
		SetMovementState(EMovementState::Grounded);
	}
	else if (GetCharacterMovement()->MovementMode == MOVE_Falling)
	{
		SetMovementState(EMovementState::InAir);
	}
}

void AAnonCharacter::OnMovementStateChanged(const EMovementState PreviousState)
{
	if (MovementState == EMovementState::InAir)
	{
		if (MovementAction == EMovementAction::None)
		{
			// If the character enters the air, set the In Air Rotation and uncrouch if crouched.
			InAirRotation = GetActorRotation();
			if (Stance == EStance::Crouching)
			{
				UnCrouch();
			}
		}
		else if (MovementAction == EMovementAction::Rolling)
		{
			// If the character is currently rolling, enable the ragdoll.
			ReplicatedRagdollStart();
		}
	}

	if (CameraBehavior)
	{
		CameraBehavior->MovementState = MovementState;
	}
}

void AAnonCharacter::OnMovementActionChanged(const EMovementAction PreviousAction)
{
	// Make the character crouch if performing a roll.
	if (MovementAction == EMovementAction::Rolling)
	{
		Crouch();
	}

	if (PreviousAction == EMovementAction::Rolling)
	{
		if (DesiredStance == EStance::Standing)
		{
			UnCrouch();
		}
		else if (DesiredStance == EStance::Crouching)
		{
			Crouch();
		}
	}

	if (CameraBehavior)
	{
		CameraBehavior->MovementAction = MovementAction;
	}
}

void AAnonCharacter::OnStanceChanged(const EStance PreviousStance)
{
	if (CameraBehavior)
	{
		CameraBehavior->Stance = Stance;
	}

	AnonCharacterMovement->SetMovementSettings(GetTargetMovementSettings());
}

void AAnonCharacter::OnRotationModeChanged(ERotationMode PreviousRotationMode)
{
	if (RotationMode == ERotationMode::VelocityDirection && ViewMode == EViewMode::FirstPerson)
	{
		// If the new rotation mode is Velocity Direction and the character is in First Person,
		// set the viewmode to Third Person.
		SetViewMode(EViewMode::ThirdPerson);
	}

	if (CameraBehavior)
	{
		CameraBehavior->SetRotationMode(RotationMode);
	}

	AnonCharacterMovement->SetMovementSettings(GetTargetMovementSettings());
}

void AAnonCharacter::OnGaitChanged(const EGait PreviousGait)
{
	if (CameraBehavior)
	{
		CameraBehavior->Gait = Gait;
	}
}

void AAnonCharacter::OnViewModeChanged(const EViewMode PreviousViewMode)
{
	if (ViewMode == EViewMode::ThirdPerson)
	{
		if (RotationMode == ERotationMode::VelocityDirection || RotationMode == ERotationMode::LookingDirection)
		{
			// If Third Person, set the rotation mode back to the desired mode.
			SetRotationMode(DesiredRotationMode);
		}
	}
	else if (ViewMode == EViewMode::FirstPerson && RotationMode == ERotationMode::VelocityDirection)
	{
		// If First Person, set the rotation mode to looking direction if currently in the velocity direction mode.
		SetRotationMode(ERotationMode::LookingDirection);
	}

	if (CameraBehavior)
	{
		CameraBehavior->ViewMode = ViewMode;
	}
}

void AAnonCharacter::OnOverlayStateChanged(const EOverlayState PreviousState)
{
}

void AAnonCharacter::OnVisibleMeshChanged(const USkeletalMesh* PrevVisibleMesh)
{
	// Update the Skeletal Mesh before we update materials and anim bp variables
	GetMesh()->SetSkeletalMesh(VisibleMesh);

	// Reset materials to their new mesh defaults
	if (GetMesh() != nullptr)
	{
		for (int32 MaterialIndex = 0; MaterialIndex < GetMesh()->GetNumMaterials(); ++MaterialIndex)
		{
			GetMesh()->SetMaterial(MaterialIndex, nullptr);
		}
	}

	// Force set variables. This ensures anim instance & character stay synchronized on mesh changes
	ForceUpdateCharacterState();
}

void AAnonCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	SetStance(EStance::Crouching);
}

void AAnonCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	SetStance(EStance::Standing);
}

void AAnonCharacter::ForceUpdateCharacterState()
{
	SetGait(DesiredGait, true);
	SetStance(DesiredStance, true);
	SetRotationMode(DesiredRotationMode, true);
	SetViewMode(ViewMode, true);
	SetOverlayState(OverlayState, true);
	SetMovementState(MovementState, true);
	SetMovementAction(MovementAction, true);
}

void AAnonCharacter::OnRep_RotationMode(ERotationMode PrevRotMode)
{
	OnRotationModeChanged(PrevRotMode);
}

void AAnonCharacter::OnRep_ViewMode(EViewMode PrevViewMode)
{
	OnViewModeChanged(PrevViewMode);
}

void AAnonCharacter::OnRep_OverlayState(EOverlayState PrevOverlayState)
{
	OnOverlayStateChanged(PrevOverlayState);
}

void AAnonCharacter::OnRep_VisibleMesh(const USkeletalMesh* PreviousSkeletalMesh)
{
	OnVisibleMeshChanged(PreviousSkeletalMesh);
}
