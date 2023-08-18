// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MotionStructs.generated.h"
/**
 * 
 */

USTRUCT(BlueprintType)
struct MOTIONMATCHING_API FTrajectoryPoint
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
		FVector m_TM = FVector::ZeroVector;

	FTrajectoryPoint()
	{
		m_TM = FVector::ZeroVector;
	}
	
	FTrajectoryPoint(const FVector TMIn)
	{
		m_TM = TMIn;
	}

};

USTRUCT(BlueprintType)
struct MOTIONMATCHING_API FTrajectoryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrajectoryData")
	TArray <FTrajectoryPoint> TrajectoryPoints;
	
	FTrajectoryData()
	{
		TrajectoryPoints.Empty();
	}
	
};

