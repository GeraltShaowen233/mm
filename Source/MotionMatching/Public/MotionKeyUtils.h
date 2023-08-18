// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <Eigen/Eigen>

#include "CoreMinimal.h"

#include "Animation/AnimSequence.h"

#include "MotionStructs.h"
#include <Eigen/Eigen>

#include "Math/UnrealMathUtility.h"
/**
*
*/

class UAnimSequence;
class FPrimitiveDrawInterface;
class UMotionField;
class USkeletalMeshComponent;
struct FMotionKey;

/**
 * 
 */
class MOTIONMATCHING_API FMotionKeyUtils
{
public:
	////////////////////////////////EXTRACTION AND BAKE FUNCTIONALITIES
	static FVector GetAnimVelocityAtTime(const UAnimSequence* InSequence, const float AtTime);
	
	static void ExtractAnimTrajectory(FTrajectoryData& OutVelocityData, const UAnimSequence* InSequence, const float KeyTime);
	
	static void GetAnimBoneWorldTM(const UAnimSequence* InSequence, const float AtTime, const int BoneIndex, FTransform& OutTM);

	static void GetAnimJointData(const UAnimSequence* InSequence, const float AtTime, const FName BoneName, Eigen::VectorXf& result,int index);

	static void MakeGoal(FTrajectoryData& OutGoal, const FTransform DesiredTransform,const FTransform RootWorldTM);

	static void SetVector2FeatureatIndex(const FVector& a, Eigen::VectorXf&b,int index);

};
