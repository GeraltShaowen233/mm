// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionKey.h"

#include "AnimationRuntime.h"


void FMotionKey::ExtractDataFromAnimation(const UAnimSequence * InSequence, const int AtSrcAnimIndex, const float AtSrcStartTime, TArray <FName> InMotionBoneNames)
{
	if (InSequence)
	{
		SrcAnimIndex = AtSrcAnimIndex;
		
		StartTime = AtSrcStartTime;
	}

}

