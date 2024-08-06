// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/LocomotionEnum.h"

class ANONLOCOMOTION_API ULocomotionMathLibrary final
{
public:
	static bool AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool IncreaseBuffer);
	
	static EMovementDirection CalculateQuadrant(EMovementDirection Current, float FRThreshold, float FLThreshold,
												   float BRThreshold,
												   float BLThreshold, float Buffer, float Angle);
};
