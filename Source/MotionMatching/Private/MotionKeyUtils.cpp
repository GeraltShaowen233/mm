// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionKeyUtils.h"
#include "MotionField.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine.h"
#include "AnimationRuntime.h"
#include <memory>


const float DeltaTime = 0.1f;

FVector FMotionKeyUtils::GetAnimVelocityAtTime(const UAnimSequence * InSequence, const float AtTime)
{
	FVector TmpVelo = InSequence->ExtractRootMotion(AtTime, DeltaTime, true).GetTranslation();

	return TmpVelo.Size()<1.e-8f ? FVector::ZeroVector : TmpVelo/DeltaTime;
}

void FMotionKeyUtils::ExtractAnimTrajectory(FTrajectoryData& OutTrajectoryData, const UAnimSequence * InSequence, const float KeyTime)
{
	if (InSequence)
	{
		OutTrajectoryData = FTrajectoryData();
		for (int i = 0; i < 10; i++)
		{
			FVector tp = InSequence->ExtractRootMotion(KeyTime, (0.1 * i), true).GetTranslation();
			OutTrajectoryData.TrajectoryPoints.Add(FTrajectoryPoint(tp));
		}
	}
	
}

void FMotionKeyUtils::GetAnimBoneWorldTM(const UAnimSequence * InSequence, const float AtTime, const int BoneIndex, FTransform & OutTM)
{

	if(!InSequence)
		return;
	USkeleton* SourceSkeleton = InSequence->GetSkeleton();
	FReferenceSkeleton RefSkel = SourceSkeleton->GetReferenceSkeleton();
	OutTM = FTransform::Identity;
	if(!RefSkel.IsValidIndex(BoneIndex))
		return;
	
	InSequence->GetBoneTransform(OutTM,BoneIndex,AtTime,false);
	int cbone = BoneIndex;
	while(RefSkel.IsValidIndex(RefSkel.GetParentIndex(cbone)))
	{
		cbone = RefSkel.GetParentIndex(cbone);
		FTransform parentTrans;
		InSequence->GetBoneTransform(parentTrans,cbone,AtTime,false);
		OutTM = OutTM*parentTrans;
	}
}

void FMotionKeyUtils::GetAnimJointData(const UAnimSequence * InSequence, const float AtTime, const FName BoneName, Eigen::VectorXf& result,int index)
{
	if (InSequence)
	{

		USkeleton* SourceSkeleton = InSequence->GetSkeleton();
		FReferenceSkeleton RefSkel = SourceSkeleton->GetReferenceSkeleton();
		const int BoneIndex = RefSkel.FindBoneIndex(BoneName);
		if(RefSkel.IsValidIndex(BoneIndex))
		{
			FTransform RootTrans;
			InSequence->GetBoneTransform(RootTrans, 0, AtTime, false);

			FTransform BoneTransNow;
			GetAnimBoneWorldTM(InSequence, AtTime, BoneIndex, BoneTransNow);
			FTransform futureTrans;
			GetAnimBoneWorldTM(InSequence, AtTime + DeltaTime, BoneIndex, futureTrans);

			FVector BoneVector = BoneTransNow.GetTranslation();
			FVector FutreVector = futureTrans.GetTranslation();
			
			FVector Velo = (FutreVector - BoneVector)/DeltaTime;
			
			SetVector2FeatureatIndex(RootTrans.InverseTransformPositionNoScale(BoneVector),result,index);

			SetVector2FeatureatIndex(RootTrans.InverseTransformVectorNoScale(Velo),result,index+3);
		}
		
	}
}

void FMotionKeyUtils::MakeGoal(FTrajectoryData & OutGoal, const FTransform DesiredTransform, const FTransform RootWorldTrans)
{
	OutGoal = FTrajectoryData();
	
	FTransform CurrentTrans = RootWorldTrans;
	
	for (int i = 0; i < 10; i++)
	{
		CurrentTrans.BlendWith(DesiredTransform,float(i)/10.0f);
		OutGoal.TrajectoryPoints.Add(FTrajectoryPoint(CurrentTrans.GetRelativeTransform(RootWorldTrans).GetTranslation()));
	}
	
}

void FMotionKeyUtils::SetVector2FeatureatIndex(const FVector& a, Eigen::VectorXf& b, int index)
{
	b(index) = a.X;
	b(index + 1) = a.Y;
	b(index + 2) = a.Z;
}
