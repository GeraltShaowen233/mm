// Fill out your copyright notice in the Description page of Project Settings.

#include "MyBlueprintFunctionLibrary.h"
#include "MotionField.h"
void UMyBlueprintFunctionLibrary::BuildGoal(FTrajectoryData & OutGoal, const FTransform DesiredTransform, const FTransform RootWorldTM)
{
	FMotionKeyUtils::MakeGoal(OutGoal, DesiredTransform, RootWorldTM);
}



