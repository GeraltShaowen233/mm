// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "MotionStructs.h"
#include "Animation/AnimSequence.h"

#include "MotionKeyUtils.h"

#include "MotionKey.generated.h"

/**
 * 
 */

class UAnimSequence;

USTRUCT(BlueprintType)
struct MOTIONMATCHING_API FMotionKey 
{
	GENERATED_BODY()
public:
	FMotionKey() = default;

	UPROPERTY()
	int SrcAnimIndex = 0;
	UPROPERTY()
	float StartTime = 0.0f;
	
	void ExtractDataFromAnimation(const UAnimSequence * InSequence, const int AtSrcAnimIndex, const float AtSrcStartTime, TArray <FName> InMotionBoneNames);

	FORCEINLINE bool operator==(const FMotionKey Other) const
	{
		return (SrcAnimIndex == Other.SrcAnimIndex) && (StartTime == Other.StartTime);
	}
};
