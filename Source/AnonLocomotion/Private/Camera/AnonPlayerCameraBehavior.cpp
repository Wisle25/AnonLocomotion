// Fill out your copyright notice in the Description page of Project Settings.

#include "Camera/AnonPlayerCameraBehavior.h"

void UAnonPlayerCameraBehavior::SetRotationMode(ERotationMode RotationMode)
{
	bVelocityDirection = RotationMode == ERotationMode::VelocityDirection;
	bLookingDirection = RotationMode == ERotationMode::LookingDirection;
	bAiming = RotationMode == ERotationMode::Aiming;
}

