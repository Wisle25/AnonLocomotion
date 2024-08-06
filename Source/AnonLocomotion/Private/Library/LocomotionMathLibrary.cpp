// Fill out your copyright notice in the Description page of Project Settings.

#include "LocomotionMathLibrary.h"

bool ULocomotionMathLibrary::AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer,
	bool IncreaseBuffer)
{
	if (IncreaseBuffer)
	{
		return Angle >= MinAngle - Buffer && Angle <= MaxAngle + Buffer;
	}
	
	return Angle >= MinAngle + Buffer && Angle <= MaxAngle - Buffer;
}

EMovementDirection ULocomotionMathLibrary::CalculateQuadrant(EMovementDirection Current, float FRThreshold,
	float FLThreshold, float BRThreshold, float BLThreshold, float Buffer, float Angle)
{
	// Take the input angle and determine its quadrant (direction). Use the current Movement Direction to increase or
	// decrease the buffers on the angle ranges for each quadrant.
	if (AngleInRange(Angle, FLThreshold, FRThreshold, Buffer,
					 Current != EMovementDirection::Forward && Current != EMovementDirection::Backward))
	{
		return EMovementDirection::Forward;
	}

	if (AngleInRange(Angle, FRThreshold, BRThreshold, Buffer,
					 Current != EMovementDirection::Right && Current != EMovementDirection::Left))
	{
		return EMovementDirection::Right;
	}

	if (AngleInRange(Angle, BLThreshold, FLThreshold, Buffer,
					 Current != EMovementDirection::Right && Current != EMovementDirection::Left))
	{
		return EMovementDirection::Left;
	}

	return EMovementDirection::Backward;
}
