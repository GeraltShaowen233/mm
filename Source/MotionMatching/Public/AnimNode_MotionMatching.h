// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#include "Animation/AnimNode_AssetPlayerBase.h"
#include "Animation/AnimInstanceProxy.h"
#include "MotionField.h"
#include "AnimNode_MotionMatching.generated.h"

USTRUCT(BlueprintType)
struct MOTIONMATCHING_API FMotionAnimation
{
	GENERATED_BODY()
	
	int AnimIndex;
	float Position = 0.f;
	float BlendTime = 0.f;
	float Weight = 0.f;
	bool Maxed;
	float LocalWeight = 0.0f;
	float GlobalWeight = 0.0f;
	FMarkerTickRecord MarkerTickRecord;

	FMotionAnimation()
	{
		AnimIndex = 0;
		Position = 0.f;
		BlendTime = 0.f;
		Weight = 0.f;
		Maxed = false;
	}

	FMotionAnimation(const int32 InAnimIndex, const float StartTime)
	{
		AnimIndex = InAnimIndex;
		Position = StartTime;
		BlendTime = 0.f;
		Weight = 0.f;
		Maxed = false;
	}


	bool ApplyTime(const float DT, const float InBlendTime, const bool Main);

};
USTRUCT(BlueprintInternalUseOnly)
struct MOTIONMATCHING_API FAnimNode_MotionMatching : public FAnimNode_AssetPlayerBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData)
	UMotionField* MotionField;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
	FTrajectoryData DesiredTrajectory;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		float Responsiveness;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		float VelocityStrength;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		float PoseStrength;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		float BlendTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MotionData, meta = (PinShownByDefault))
		float PlayRate;

public:

	FAnimNode_MotionMatching();


	// FAnimNode_AssetPlayerBase interface
	virtual float GetCurrentAssetTime();
	virtual float GetCurrentAssetTimePlayRateAdjusted();
	virtual float GetCurrentAssetLength();
	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void UpdateAssetPlayer(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;

	

protected:
	int CurrentMotionKeyIndex;
	bool bLoopAnimation;

private:

	UPROPERTY()
	TArray<FMotionAnimation> m_Anims;
	
	Eigen::VectorXf featureVector;
	
	void PlayAnimStartingAtTime(const int32 AnimIndex, const float StartingTime);
	UAnimSequence* GetCurrentAnim();
	UAnimSequence* AnimAtIndex(const int32 Index);

	void MotionUpdate(const FAnimationUpdateContext& Context);
	void GetMotionData(Eigen::VectorXf& vector);
	
	void Compute();

	void GetBlendPose(const float DT, FTransform& OutRootMotion, FCompactPose& OutPose, FBlendedCurve& OutCurve, FStackCustomAttributes& CustomAttributes);
};
