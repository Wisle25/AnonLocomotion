// Fill out your copyright notice in the Description page of Project Settings.

#include "Camera/AnonPlayerCameraManager.h"

#include "Camera/AnonPlayerCameraBehavior.h"
#include "Characters/AnonCharacter.h"
#include "Kismet/KismetMathLibrary.h"

const FName NAME_CameraBehavior(TEXT("CameraBehavior"));
const FName NAME_CameraOffset_X(TEXT("CameraOffset_X"));
const FName NAME_CameraOffset_Y(TEXT("CameraOffset_Y"));
const FName NAME_CameraOffset_Z(TEXT("CameraOffset_Z"));
const FName NAME_Override_Debug(TEXT("Override_Debug"));
const FName NAME_PivotLagSpeed_X(TEXT("PivotLagSpeed_X"));
const FName NAME_PivotLagSpeed_Y(TEXT("PivotLagSpeed_Y"));
const FName NAME_PivotLagSpeed_Z(TEXT("PivotLagSpeed_Z"));
const FName NAME_PivotOffset_X(TEXT("PivotOffset_X"));
const FName NAME_PivotOffset_Y(TEXT("PivotOffset_Y"));
const FName NAME_PivotOffset_Z(TEXT("PivotOffset_Z"));
const FName NAME_RotationLagSpeed(TEXT("RotationLagSpeed"));
const FName NAME_Weight_FirstPerson(TEXT("Weight_FirstPerson"));

AAnonPlayerCameraManager::AAnonPlayerCameraManager()
{
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	CameraBehavior = CreateDefaultSubobject<USkeletalMeshComponent>(NAME_CameraBehavior);
	CameraBehavior->SetupAttachment(GetRootComponent());
	CameraBehavior->bHiddenInGame = true;
}

void AAnonPlayerCameraManager::OnPossess(AAnonCharacter* NewCharacter)
{
	// Set "Controlled Pawn" when Player Controller Possesses new character. (called from Player Controller)
	check(NewCharacter);
	ControlledCharacter = NewCharacter;
	
	// Update references in the Camera Behavior AnimBP.
	if (UAnonPlayerCameraBehavior* CastedBehv = Cast<UAnonPlayerCameraBehavior>(CameraBehavior->GetAnimInstance()))
	{
		NewCharacter->SetCameraBehavior(CastedBehv);
		CastedBehv->MovementState = NewCharacter->GetMovementState();
		CastedBehv->MovementAction = NewCharacter->GetMovementAction();
		CastedBehv->bRightShoulder = NewCharacter->IsRightShoulder();
		CastedBehv->Gait = NewCharacter->GetGait();
		CastedBehv->SetRotationMode(NewCharacter->GetRotationMode());
		CastedBehv->Stance = NewCharacter->GetStance();
		CastedBehv->ViewMode = NewCharacter->GetViewMode();
	}
	
	// Initial position
	const FVector& TPSLoc = ControlledCharacter->GetThirdPersonPivotTarget().GetLocation();
	SetActorLocation(TPSLoc);
	SmoothedPivotTarget.SetLocation(TPSLoc);
}

float AAnonPlayerCameraManager::GetCameraBehaviorParam(FName CurveName) const
{
	UAnimInstance* Inst = CameraBehavior->GetAnimInstance();
	if (Inst)
	{
		return Inst->GetCurveValue(CurveName);
	}
	return 0.0f;
}

void AAnonPlayerCameraManager::UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime)
{
	// Partially taken from base class

	if (OutVT.Target)
	{
		FVector OutLocation;
		FRotator OutRotation;
		float OutFOV;

		if (OutVT.Target->IsA<AAnonCharacter>())
		{
			if (CustomCameraBehavior(DeltaTime, OutLocation, OutRotation, OutFOV))
			{
				OutVT.POV.Location = OutLocation;
				OutVT.POV.Rotation = OutRotation;
				OutVT.POV.FOV = OutFOV;
			}
			else
			{
				OutVT.Target->CalcCamera(DeltaTime, OutVT.POV);
			}
		}
		else
		{
			OutVT.Target->CalcCamera(DeltaTime, OutVT.POV);
		}
	}
}

FVector AAnonPlayerCameraManager::CalculateAxisIndependentLag(FVector CurrentLocation, FVector TargetLocation,
                                                             FRotator CameraRotation, FVector LagSpeeds,
                                                             float DeltaTime)
{
	CameraRotation.Roll = 0.0f;
	CameraRotation.Pitch = 0.0f;
	const FVector UnrotatedCurLoc = CameraRotation.UnrotateVector(CurrentLocation);
	const FVector UnrotatedTargetLoc = CameraRotation.UnrotateVector(TargetLocation);

	const FVector ResultVector(
		FMath::FInterpTo(UnrotatedCurLoc.X, UnrotatedTargetLoc.X, DeltaTime, LagSpeeds.X),
		FMath::FInterpTo(UnrotatedCurLoc.Y, UnrotatedTargetLoc.Y, DeltaTime, LagSpeeds.Y),
		FMath::FInterpTo(UnrotatedCurLoc.Z, UnrotatedTargetLoc.Z, DeltaTime, LagSpeeds.Z));

	return CameraRotation.RotateVector(ResultVector);
}

bool AAnonPlayerCameraManager::CustomCameraBehavior(float DeltaTime, FVector& Location, FRotator& Rotation, float& FOV)
{
	if (!ControlledCharacter.IsValid())
	{
		return false;
	}

	// Step 1: Get Camera Parameters from CharacterBP via the Camera Interface
	const FTransform& PivotTarget = ControlledCharacter->GetThirdPersonPivotTarget();
	const FVector& FPTarget = ControlledCharacter->GetFirstPersonCameraTarget();
	float TPFOV = 90.0f;
	float FPFOV = 90.0f;
	bool bRightShoulder = false;
	ControlledCharacter->GetCameraParameters(TPFOV, FPFOV, bRightShoulder);

	// Step 2: Calculate Target Camera Rotation. Use the Control Rotation and interpolate for smooth camera rotation.
	TargetCameraRotation = FMath::RInterpTo(GetCameraRotation(),
	                                                GetOwningPlayerController()->GetControlRotation(), DeltaTime,
	                                                GetCameraBehaviorParam(NAME_RotationLagSpeed));

	// Step 3: Calculate the Smoothed Pivot Target (Orange Sphere).
	// Get the 3P Pivot Target (Green Sphere) and interpolate using axis independent lag for maximum control.
	const FVector LagSpd(GetCameraBehaviorParam(NAME_PivotLagSpeed_X),
	                     GetCameraBehaviorParam(NAME_PivotLagSpeed_Y),
	                     GetCameraBehaviorParam(NAME_PivotLagSpeed_Z));

	const FVector& AxisIndpLag = CalculateAxisIndependentLag(SmoothedPivotTarget.GetLocation(),
	                                                         PivotTarget.GetLocation(), TargetCameraRotation, LagSpd,
	                                                         DeltaTime);

	SmoothedPivotTarget.SetRotation(PivotTarget.GetRotation());
	SmoothedPivotTarget.SetLocation(AxisIndpLag);
	SmoothedPivotTarget.SetScale3D(FVector::OneVector);

	// Step 4: Calculate Pivot Location (BlueSphere). Get the Smoothed
	// Pivot Target and apply local offsets for further camera control.
	PivotLocation =
		SmoothedPivotTarget.GetLocation() +
		UKismetMathLibrary::GetForwardVector(SmoothedPivotTarget.Rotator()) * GetCameraBehaviorParam(
			NAME_PivotOffset_X) +
		UKismetMathLibrary::GetRightVector(SmoothedPivotTarget.Rotator()) * GetCameraBehaviorParam(
			NAME_PivotOffset_Y) +
		UKismetMathLibrary::GetUpVector(SmoothedPivotTarget.Rotator()) * GetCameraBehaviorParam(
			NAME_PivotOffset_Z);

	// Step 5: Calculate Target Camera Location. Get the Pivot location and apply camera relative offsets.
	TargetCameraLocation = UKismetMathLibrary::VLerp(
		PivotLocation +
		UKismetMathLibrary::GetForwardVector(TargetCameraRotation) * GetCameraBehaviorParam(
			NAME_CameraOffset_X) +
		UKismetMathLibrary::GetRightVector(TargetCameraRotation) * GetCameraBehaviorParam(NAME_CameraOffset_Y)
		+
		UKismetMathLibrary::GetUpVector(TargetCameraRotation) * GetCameraBehaviorParam(NAME_CameraOffset_Z),
		PivotTarget.GetLocation(),
		GetCameraBehaviorParam(NAME_Override_Debug));

	// Step 6: Trace for an object between the camera and character to apply a corrective offset.
	// Trace origins are set within the Character BP via the Camera Interface.
	// Functions like the normal spring arm, but can allow for different trace origins regardless of the pivot
	FVector TraceOrigin;
	float TraceRadius;
	ECollisionChannel TraceChannel = ControlledCharacter->GetThirdPersonTraceParams(TraceOrigin, TraceRadius);

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(ControlledCharacter.Get());

	FHitResult HitResult;
	const FCollisionShape SphereCollisionShape = FCollisionShape::MakeSphere(TraceRadius);
	World->SweepSingleByChannel(HitResult, TraceOrigin, TargetCameraLocation, FQuat::Identity,
	                                              TraceChannel, SphereCollisionShape, Params);

	if (HitResult.IsValidBlockingHit())
	{
		TargetCameraLocation += HitResult.Location - HitResult.TraceEnd;
	}

	// Step 8: Lerp First Person Override and return target camera parameters.
	FTransform TargetCameraTransform(TargetCameraRotation, TargetCameraLocation, FVector::OneVector);
	FTransform FPTargetCameraTransform(TargetCameraRotation, FPTarget, FVector::OneVector);

	const FTransform& MixedTransform = UKismetMathLibrary::TLerp(TargetCameraTransform, FPTargetCameraTransform,
	                                                             GetCameraBehaviorParam(
		                                                             NAME_Weight_FirstPerson));

	Location = MixedTransform.GetLocation();
	Rotation = MixedTransform.Rotator();
	FOV = FMath::Lerp(TPFOV, FPFOV, GetCameraBehaviorParam(NAME_Weight_FirstPerson));

	return true;
}
