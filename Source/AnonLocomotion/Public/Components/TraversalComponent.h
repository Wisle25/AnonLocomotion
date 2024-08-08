// Fill out your copyright notice in the Description page of Project Settings.

// ReSharper disable CppRedundantAccessSpecifier
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/TraversalEnum.h"
#include "TraversalComponent.generated.h"

class AAnonCharacter;
class UAnonAnimInstance;
class UCapsuleComponent;
class UCharacterMovementComponent;
class UMotionWarpingComponent;

UCLASS( ClassGroup=(Anon), meta=(BlueprintSpawnableComponent) )
class ANONLOCOMOTION_API UTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTraversalComponent();
	
	// ==================== Lifecycles ==================== //	

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// ==================== References ==================== //

	TWeakObjectPtr<AAnonCharacter> Character;
	
	TWeakObjectPtr<UCharacterMovementComponent> Movement;
	TWeakObjectPtr<USkeletalMeshComponent> Mesh;
	TWeakObjectPtr<UCapsuleComponent> Capsule;
	TWeakObjectPtr<UMotionWarpingComponent> MotionWarping;
	TWeakObjectPtr<UAnonAnimInstance> AnimInstance;

	/** For debugging, we can remove later */
	UPROPERTY(EditAnywhere, Category="Traversal|References")
	TSubclassOf<AActor> ArrowClass;

	TWeakObjectPtr<AActor> Arrow;

	void InitReferences();

private:
	// ==================== States ==================== //

	ETraversalAction TraversalAction = ETraversalAction::NoAction;
	ETraversalState TraversalState = ETraversalState::FreeRoam;
	ETraversalDirection TraversalDirection = ETraversalDirection::Forward;
	EClimbStyle ClimbStyle = EClimbStyle::BracedClimb;
	
	void TraversalStateSettings(const ECollisionEnabled::Type CollisionType, const EMovementMode NewMovementMode, const bool bStopMovementImmediately = false) const;

	FORCEINLINE static float ClimbStyleValues(const EClimbStyle ClimbStyle, float Braced, float FreeHang)
	{
		return ClimbStyle == EClimbStyle::BracedClimb ? Braced : FreeHang;
	}
	
public:
	void SetTraversalState(const ETraversalState NewState);
	void SetTraversalDirection(const ETraversalDirection NewDirection);
	void SetClimbStyle(const EClimbStyle NewStyle);

private:
	// ==================== Utility ==================== //

	FORCEINLINE static FRotator ReverseNormal(const FVector& NormalVector)
	{
		const FRotator DeltaRot = (FRotationMatrix::MakeFromX(NormalVector).Rotator() - FRotator(0.f, 180.f, 0.f).GetNormalized());

		return { 0.f, DeltaRot.Yaw, 0.f };
	}
	
	// ==================== Wall Detection ==================== //
	
	/** The lowest point where the system can detect the wall */
	UPROPERTY(EditAnywhere, Category="Traversal|Wall Detection")
	float LowDetectPoint = 62.f;

	/** Its like, maximum distance between character's front side with the wall that can be detected  */
	UPROPERTY(EditAnywhere, Category="Traversal|Wall Detection")
	float FrontDetectLength = 200.f;

	UPROPERTY(EditAnywhere, Category="Traversal|Wall Detection")
	int32 ScanWidth = 4;
	
	UPROPERTY(EditAnywhere, Category="Traversal|Wall Detection")
	int32 ScanHeight = 30;

	FHitResult WallHitResult;
	FHitResult WallTopResult;
	FHitResult WallDepthResult;
	FHitResult WallVaultResult;
	
	FRotator WallRotation;
	
	FHitResult DetectWall() const;

	void GatherScanVertically(TArray<FHitResult>& LineHitTraces, const FVector& WallForward, const FVector& Width) const;
	void PrepareClosestPoint(TArray<FHitResult>& WallHitTraces, const TArray<FHitResult>& LineHitTraces) const;
	void CalculateClosestPoint(const TArray<FHitResult>& WallHitTraces);
	void GatherWallTop(FHitResult& LastTopHit);
	void GetVaultLanding(const FVector& WallForward, const FVector& WallUp);
	void GetWallDepth(const FVector& WallForward, const FHitResult& LastTopHit);
	void WallScan(const FVector& BaseLocation, const FRotator& BaseRotation);

	// ==================== Wall Measurement ==================== //

	float WallHeight = 0.f;
	float WallDepth = 0.f;
	float VaultHeight = 0.f;
	
	void MeasureWall();

private:
	// ==================== Mechanics ==================== //

	void DecideTraversalType(bool bJumpAction);
	void ResetResult();

public:
	void TriggerTraversalAction(const bool bJumpAction = false);
	
};
