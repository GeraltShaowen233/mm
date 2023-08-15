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
		FTransform m_TM = FTransform::Identity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
		float m_TimeDelay = 0.f;

	FTrajectoryPoint()
	{
		m_TM = FTransform::Identity;
		m_TimeDelay = 0.f;
	}

	FTrajectoryPoint(const FTransform TMIn, const float TimeDelayIn)
	{
		m_TM = TMIn;
		m_TimeDelay = TimeDelayIn;
	}

};

USTRUCT(BlueprintType)
struct MOTIONMATCHING_API FTrajectoryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrajectoryData")
		TArray <FTrajectoryPoint> TrajectoryPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrajectoryData")
		FVector EndVel = FVector::ZeroVector;

	FTrajectoryData()
	{
		TrajectoryPoints.Empty();
		EndVel = FVector::ZeroVector;
	}

	float CompareTo(const FTrajectoryData Other, const int Per) const;
};

