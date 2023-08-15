// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "Animation/AnimSequence.h"
#include "Public/BonePose.h"

#include "MotionKeyUtils.h"

#include "MyBlueprintFunctionLibrary.generated.h"

/**
 * 
 */

struct FBlendedCurve;
struct FCompactPose;
class UAnimSequence;
class UMotionField;

/*Blueprint Function Library to Test things in the editor*/
UCLASS()
class MOTIONMATCHING_API UMyBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	UFUNCTION(BlueprintCallable, Category = "shit")
	static void BuildGoal(FTrajectoryData& OutGoal, const FTransform DesiredTransform, const float TargetUUS, const FTransform RootWorldTM);
	
};
