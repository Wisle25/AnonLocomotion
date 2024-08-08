// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/TraversalComponent.h"

#include "Characters/AnonCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionWarpingComponent.h"
#include "Characters/AnonAnimInstance.h"
#include "Kismet/KismetSystemLibrary.h"

UTraversalComponent::UTraversalComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

// ==================== Lifecycles ==================== //

void UTraversalComponent::BeginPlay()
{
	Super::BeginPlay();

	InitReferences();
}

void UTraversalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

// ==================== References ==================== //

void UTraversalComponent::InitReferences()
{
	Character = Cast<AAnonCharacter>(GetOwner());

	Movement = Character->GetCharacterMovement();
	Mesh = Character->GetMesh();
	Capsule = Character->GetCapsuleComponent();
	MotionWarping = Character->GetComponentByClass<UMotionWarpingComponent>();
	AnimInstance = Cast<UAnonAnimInstance>(Mesh->GetAnimInstance());

	// Debugging Arrow
	Arrow = GetWorld()->SpawnActor(ArrowClass, &Character->GetTransform());
	Arrow->AttachToComponent(Mesh.Get(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	Arrow->SetActorRelativeLocation({0.f, 0.f, 195.f});
}

// ==================== States ==================== //

void UTraversalComponent::TraversalStateSettings(const ECollisionEnabled::Type CollisionType,
	const EMovementMode NewMovementMode, const bool bStopMovementImmediately) const
{
	Capsule->SetCollisionEnabled(CollisionType);
	Movement->SetMovementMode(NewMovementMode);

	if (bStopMovementImmediately)
	{
		Movement->StopMovementImmediately();
	}
}

void UTraversalComponent::SetTraversalState(const ETraversalState NewState)
{
	if (TraversalState == NewState) return;
	
	TraversalState = NewState;

	AnimInstance->SetTraversalState(TraversalState);

	switch (TraversalState) {
	case ETraversalState::FreeRoam:
		TraversalStateSettings(ECollisionEnabled::QueryAndPhysics, MOVE_Walking);
		break;
	case ETraversalState::Climb:
		TraversalStateSettings(ECollisionEnabled::NoCollision, MOVE_Flying, true);
		break;
	default:
		TraversalStateSettings(ECollisionEnabled::NoCollision, MOVE_Flying);
		break;
	}
}

void UTraversalComponent::SetTraversalDirection(const ETraversalDirection NewDirection)
{
	if (TraversalDirection == NewDirection) return;

	TraversalDirection = NewDirection;

	AnimInstance->SetTraversalDirection(TraversalDirection);
}

void UTraversalComponent::SetClimbStyle(const EClimbStyle NewStyle)
{
	if (ClimbStyle == NewStyle) return;

	ClimbStyle = NewStyle;

	AnimInstance->SetClimbStyle(ClimbStyle);
}

// ==================== Wall Detection ==================== //

FHitResult UTraversalComponent::DetectWall() const
{
	const int8 LastLoop = Movement->IsFalling() ? 8 : 15;

	// Determine trace point
	const FVector CharLocation = Character->GetActorLocation();
	
	FVector EntryStart = CharLocation;
	EntryStart.Z = CharLocation.Z - LowDetectPoint;
	
	FHitResult WallResult;

	// Detect
	for (int8 I = 0; I <= LastLoop; ++I)
	{
		const FVector Start = EntryStart + Character->GetActorUpVector() * /* Gap */ 20.f * I;
		const FVector End = Start + Character->GetActorForwardVector() * FrontDetectLength;
		
		UKismetSystemLibrary::SphereTraceSingle(Character.Get(), Start, End, 8.f,TraceTypeQuery1,
			false, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, WallResult, true
		);

		if (WallResult.bBlockingHit && !WallResult.bStartPenetrating) break;
	}

	return WallResult;
}

void UTraversalComponent::GatherScanVertically(TArray<FHitResult>& LineHitTraces, const FVector& WallForward, const FVector& Width) const
{
	for (int32 J = 0; J <= ScanHeight; ++J)
	{
		const FVector Height = Width + Character->GetActorUpVector() * J * 8;

		const FVector TraceStart = Height - WallForward * 60.f;
		const FVector TraceEnd = Height + WallForward * 30.f;

		FHitResult LineResult;
			
		UKismetSystemLibrary::LineTraceSingle(Character.Get(), TraceStart, TraceEnd, TraceTypeQuery1,
			false, TArray<AActor*>(), EDrawDebugTrace::None, LineResult, true
		);

		LineHitTraces.Add(LineResult);
	}
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void UTraversalComponent::PrepareClosestPoint(TArray<FHitResult>& WallHitTraces, const TArray<FHitResult>& LineHitTraces) const
{
	for (int32 J = 1; J < LineHitTraces.Num(); ++J)
	{
		const float CurrentDistance = LineHitTraces[J].bBlockingHit ?
							          LineHitTraces[J].Distance :
									  (LineHitTraces[J].TraceEnd - LineHitTraces[J].TraceStart).Size();
		const float LastDistance = LineHitTraces[J - 1].bBlockingHit ?
								   LineHitTraces[J - 1].Distance :
								   (LineHitTraces[J - 1].TraceEnd - LineHitTraces[J - 1].TraceStart).Size();
		const float DeltaDistance = FMath::Abs(LastDistance - CurrentDistance);

		if (DeltaDistance > 5.f)
		{
			WallHitTraces.Add(LineHitTraces[J - 1]);
				
			break;
		}
	}
}

void UTraversalComponent::CalculateClosestPoint(const TArray<FHitResult>& WallHitTraces)
{
	WallHitResult = WallHitTraces[0];

	for (int32 I = 1; I < WallHitTraces.Num(); ++I)
	{
		const float DistanceCharaToWallCurrent = (WallHitTraces[I].ImpactPoint - Character->GetActorLocation()).Size();
		const float DistanceCharaToWallResult = (WallHitResult.ImpactPoint - Character->GetActorLocation()).Size();
		
		if (DistanceCharaToWallCurrent <= DistanceCharaToWallResult)
			WallHitResult = WallHitTraces[I];
	}
}

void UTraversalComponent::GatherWallTop(FHitResult& LastTopHit)
{
	FHitResult TopResult;

	if (TraversalState != ETraversalState::Climb)
	{
		WallRotation = ReverseNormal(WallHitResult.ImpactNormal);
	}
	
	for (int32 I = 0; I <= 8; ++I)
	{
		const FVector PivotPoint = WallHitResult.ImpactPoint + FRotationMatrix(WallRotation).GetUnitAxis(EAxis::X) * I * 30;
		const FVector StartTrace = PivotPoint + FVector(0.f, 0.f, 25.f);
		const FVector EndTrace = PivotPoint  - FVector(0.f, 0.f, 25.f); 
		
		UKismetSystemLibrary::SphereTraceSingle(Character.Get(), StartTrace, EndTrace, 2.5f, TraceTypeQuery1,
			false, TArray<AActor*>(), EDrawDebugTrace::None, TopResult, true
		);

		if (I == 0 && TopResult.bBlockingHit)
		{
			WallTopResult = TopResult;

			DrawDebugSphere(GetWorld(), WallTopResult.ImpactPoint, 10.f, 8, FColor::Green, false, 2.f);
		}

		if (TopResult.bBlockingHit)
		{
			LastTopHit = TopResult;
		}
		else
		{
			break;
		}
	}
}

void UTraversalComponent::GetVaultLanding(const FVector& WallForward, const FVector& WallUp)
{
	if (WallDepthResult.bBlockingHit)
	{
		DrawDebugSphere(GetWorld(), WallDepthResult.ImpactPoint, 10.f, 8, FColor::Green, false, 2.f);
		
		const FVector VaultStart = WallDepthResult.ImpactPoint + WallForward * 70.f;
		const FVector VaultEnd = VaultStart - WallUp * 200.f;
		
		UKismetSystemLibrary::SphereTraceSingle(Character.Get(), VaultStart, VaultEnd, 10.f, TraceTypeQuery1,
			false, TArray<AActor*>(), EDrawDebugTrace::None, WallVaultResult, true
		);

		DrawDebugSphere(GetWorld(), WallVaultResult.ImpactPoint, 10.f, 8, FColor::Blue, false, 2.f);
	}
}

void UTraversalComponent::GetWallDepth(const FVector& WallForward, const FHitResult& LastTopHit)
{
	const FVector StartTrace = LastTopHit.ImpactPoint + WallForward * 50.f; 
	const FVector EndTrace = LastTopHit.ImpactPoint;
	
	UKismetSystemLibrary::SphereTraceSingle(Character.Get(), StartTrace, EndTrace, 10.f, TraceTypeQuery1,
	                                        false, TArray<AActor*>(), EDrawDebugTrace::None, WallDepthResult, true
	);
}

void UTraversalComponent::WallScan(const FVector& BaseLocation, const FRotator& BaseRotation)
{
	TArray<FHitResult> WallHitTraces;
	TArray<FHitResult> LineHitTraces;

	const FRotationMatrix BaseMatrix = FRotationMatrix(BaseRotation);
	const FVector WallForward = BaseMatrix.GetUnitAxis(EAxis::X);
	const FVector WallRight = BaseMatrix.GetUnitAxis(EAxis::Y);
	const FVector WallUp = BaseMatrix.GetUnitAxis(EAxis::Z);

	for (int32 I = 0; I <= ScanWidth; ++I)
	{
		const float CurrentWidth = I * 20 - ScanWidth * 10;
		const FVector Width = BaseLocation + WallRight * CurrentWidth;
		
		LineHitTraces.Empty();

		// Gather the wall/obstacle points
		GatherScanVertically(LineHitTraces, WallForward, Width);

		// Gather the line traces to later can get calculated for closest point to character
		PrepareClosestPoint(WallHitTraces, LineHitTraces);
	}

	// Calculate which is the closest point to the character
	if (WallHitTraces.IsEmpty()) return;
	CalculateClosestPoint(WallHitTraces);
	
	if (!WallHitResult.bBlockingHit || WallHitResult.bStartPenetrating) return;
	
	// Now, gather for the Wall Top
	FHitResult LastTopHit;
	GatherWallTop(LastTopHit);

	if (TraversalState != ETraversalState::FreeRoam) return;
	
	// Now check for wall depth
	GetWallDepth(WallForward, LastTopHit);

	// Check for Vault Result
	GetVaultLanding(WallForward, WallUp);
}

// ==================== Wall Measurement ==================== //

void UTraversalComponent::MeasureWall()
{
	if (!WallHitResult.bBlockingHit || !WallTopResult.bBlockingHit)
	{
		WallHeight = WallDepth = VaultHeight = 0.f;
		
		return;
	}

	// Calcluate Wall Height
	WallHeight = FMath::Abs(WallTopResult.ImpactPoint.Z - Mesh->GetSocketLocation("root").Z);
	WallDepth = WallDepthResult.bBlockingHit ? (WallDepthResult.ImpactPoint - WallTopResult.ImpactPoint).Size() : 0.f;
	VaultHeight = WallVaultResult.bBlockingHit ? FMath::Abs(WallDepthResult.ImpactPoint.Z - WallVaultResult.ImpactPoint.Z) : 0.f;

	UE_LOG(LogTemp, Warning, TEXT("Height: %f, Depth: %f, Vault: %f"), WallHeight, WallDepth, VaultHeight);
}

// ==================== Mechanics ==================== //

void UTraversalComponent::DecideTraversalType(bool bJumpAction)
{
	if (!WallTopResult.bBlockingHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Action"));
		
		if (bJumpAction)
		{
		    Character->Jump();
		}
		
		return;
	}

	if (TraversalState == ETraversalState::FreeRoam)
	{
		if (45.f <= WallHeight && WallHeight <= 176.f)
		{
			if ((0.f <= WallDepth && WallDepth <= 120.f) && (60.f <= VaultHeight && VaultHeight <= 120.f) && Character->GetSpeed() > 20.f)
			{
				UE_LOG(LogTemp, Warning, TEXT("VAULT"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("MANTLE"));
			}
		}
		else if (176.f <= WallHeight && WallHeight < 250.f)
		{
			UE_LOG(LogTemp, Warning, TEXT("CLIMB"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No Action"));
		}
	}
	else if (TraversalState ==  ETraversalState::ReadyToClimb)
	{
		UE_LOG(LogTemp, Warning, TEXT("Climb Up or Hop"));
	}
}

void UTraversalComponent::ResetResult()
{
	WallHitResult.Reset();
	WallTopResult.Reset();
	WallDepthResult.Reset();
	WallVaultResult.Reset();

	WallHeight = WallDepth = VaultHeight = 0.f;
}

void UTraversalComponent::TriggerTraversalAction(const bool bJumpAction)
{
	if (TraversalAction != ETraversalAction::NoAction) return;

	const FHitResult WallResult = DetectWall();

	// If no wall infront of the character, just jump
	if (!WallResult.bBlockingHit)
	{
		if (bJumpAction)
		{
			Character->Jump();
		}

		return;
	}

	// Next if wall is found
	WallScan(WallResult.ImpactPoint, ReverseNormal(WallResult.ImpactNormal));
	MeasureWall();
	DecideTraversalType(bJumpAction);

	// Reset
	ResetResult();
}
