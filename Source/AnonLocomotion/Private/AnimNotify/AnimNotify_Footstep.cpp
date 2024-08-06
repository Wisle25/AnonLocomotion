// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotify/AnimNotify_Footstep.h"

#include "Components/AudioComponent.h"
#include "Data/LocomotionStruct.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

void UAnimNotify_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                  const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (!MeshComp) return;

	const AActor* MeshOwner = MeshComp->GetOwner();
	if (!MeshOwner) return;

	if (HitDataTable)
	{
		const UWorld* World = MeshComp->GetWorld();
		check(World);

		const FVector FootLocation = MeshComp->GetSocketLocation(FootSocketName);
		const FRotator FootRotation = MeshComp->GetSocketRotation(FootSocketName);
		const FVector TraceEnd = FootLocation - MeshOwner->GetActorUpVector() * TraceLength;

		FHitResult Hit;
		UKismetSystemLibrary::LineTraceSingle(MeshOwner, FootLocation, TraceEnd, TraceChannel, true, MeshOwner->Children,
        		                                          DrawDebugType, Hit, true);

		if (!Hit.bBlockingHit || !Hit.PhysMaterial.Get()) return;

		const EPhysicalSurface SurfaceType = Hit.PhysMaterial.Get()->SurfaceType;

		check(IsInGameThread());
		checkNoRecursion();
		static TArray<FHitFX*> HitFXRows;
		HitFXRows.Reset();

		HitDataTable->GetAllRows<FHitFX>("", HitFXRows);

		const FHitFX* HitFX;
		if (const auto FoundResult = HitFXRows.FindByPredicate([&](const FHitFX* Value)
			{
				return SurfaceType == Value->SurfaceType;
			})
		)
		{
			HitFX = *FoundResult;
		}
		else if (const auto DefaultResult = HitFXRows.FindByPredicate([&](const FHitFX* Value)
		{
			return EPhysicalSurface::SurfaceType_Default == Value->SurfaceType;
		}))
		{
			HitFX = *DefaultResult;
		}
		else
		{
			return;
		}
		
		if (bSpawnSound && HitFX->Sound.LoadSynchronous())
		{
			UAudioComponent* SpawnedSound = nullptr;
		
			const float MaskCurveValue = MeshComp->GetAnimInstance()->GetCurveValue(
				"Mask_FootstepSound");
			const float FinalVolMult = bOverrideMaskCurve
				                           ? VolumeMultiplier
				                           : VolumeMultiplier * (1.0f - MaskCurveValue);
		
			switch (HitFX->SoundSpawnType)
			{
			case ESpawnType::Location:
				SpawnedSound = UGameplayStatics::SpawnSoundAtLocation(
					World, HitFX->Sound.Get(), Hit.Location + HitFX->SoundLocationOffset,
					HitFX->SoundRotationOffset, FinalVolMult, PitchMultiplier);
				break;
		
			case ESpawnType::Attached:
				SpawnedSound = UGameplayStatics::SpawnSoundAttached(HitFX->Sound.Get(), MeshComp, FootSocketName,
				                                                    HitFX->SoundLocationOffset,
				                                                    HitFX->SoundRotationOffset,
				                                                    HitFX->SoundAttachmentType, true, FinalVolMult,
				                                                    PitchMultiplier);
		
				break;
			}
			if (SpawnedSound)
			{
				SpawnedSound->SetIntParameter(SoundParameterName, static_cast<int32>(FootstepType));
			}
		}
		
		if (bSpawnNiagara && HitFX->NiagaraSystem.LoadSynchronous())
		{
			const FVector Location = Hit.Location + MeshOwner->GetTransform().TransformVector(
				HitFX->DecalLocationOffset);
		
			switch (HitFX->NiagaraSpawnType)
			{
			case ESpawnType::Location:
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					World, HitFX->NiagaraSystem.Get(), Location, FootRotation + HitFX->NiagaraRotationOffset);
				break;
		
			case ESpawnType::Attached:
				UNiagaraFunctionLibrary::SpawnSystemAttached(
					HitFX->NiagaraSystem.Get(), MeshComp, FootSocketName, HitFX->NiagaraLocationOffset,
					HitFX->NiagaraRotationOffset, HitFX->NiagaraAttachmentType, true);
				break;
			}
		}
		
		if (bSpawnDecal && HitFX->DecalMaterial.LoadSynchronous())
		{
			const FVector Location = Hit.Location + MeshOwner->GetTransform().TransformVector(
				HitFX->DecalLocationOffset);
		
			const FVector DecalSize = FVector(bMirrorDecalX ? -HitFX->DecalSize.X : HitFX->DecalSize.X,
			                                  bMirrorDecalY ? -HitFX->DecalSize.Y : HitFX->DecalSize.Y,
			                                  bMirrorDecalZ ? -HitFX->DecalSize.Z : HitFX->DecalSize.Z);
		
			switch (HitFX->DecalSpawnType)
			{
			case ESpawnType::Location:
				UGameplayStatics::SpawnDecalAtLocation(
					World, HitFX->DecalMaterial.Get(), DecalSize, Location,
					FootRotation + HitFX->DecalRotationOffset, HitFX->DecalLifeSpan);
				break;
		
			case ESpawnType::Attached:
				UGameplayStatics::SpawnDecalAttached(HitFX->DecalMaterial.Get(), DecalSize,
				                                                    Hit.Component.Get(), NAME_None, Location,
				                                                    FootRotation + HitFX->DecalRotationOffset,
				                                                    HitFX->DecalAttachmentType,
				                                                    HitFX->DecalLifeSpan);
				break;
			}
		}
	}
}

FString UAnimNotify_Footstep::GetNotifyName_Implementation() const
{	FString Name(TEXT("Footstep Type: "));
	Name.Append(GetEnumerationToString(FootstepType));
	
	return Name;
}
