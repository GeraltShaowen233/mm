// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionKeyUtils.h"
#include "MotionField.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine.h"
#include "AnimationRuntime.h"


const float DeltaTime = 0.1f;

FVector FMotionKeyUtils::GetAnimVelocityAtTime(const UAnimSequence * InSequence, const float AtTime)
{
	FVector TmpVelo = InSequence->ExtractRootMotion(AtTime, DeltaTime, true).GetTranslation();
	
	return TmpVelo.GetSafeNormal() * (TmpVelo.Size() / DeltaTime);
	
}

void FMotionKeyUtils::ExtractAnimTrajectory(FTrajectoryData& OutTrajectoryData, const UAnimSequence * InSequence, const float KeyTime)
{
	if (!InSequence)
	{
		return;
	}

	OutTrajectoryData = FTrajectoryData();

	FTransform KeyTM;
	InSequence->GetBoneTransform(KeyTM, 0, KeyTime, false);

	TArray <FTrajectoryPoint> TrajectoryPoints;

	for (int i = 0; i < 10; i++)
	{

		float TimeDelay = (0.1 * i);
		
		FTransform TrajectoryPointTM = InSequence->ExtractRootMotion(KeyTime, TimeDelay, true);

		OutTrajectoryData.TrajectoryPoints.Add(FTrajectoryPoint(TrajectoryPointTM, TimeDelay));

	}
	
}

void FMotionKeyUtils::GetAnimBoneWorldTM(const UAnimSequence * InSequence, const float AtTime, const int BoneIndex, FTransform & OutTM)
{

	OutTM = FTransform::Identity;

	bool bUseRawData = false;

	if (InSequence && (BoneIndex != INDEX_NONE))
	{

		InSequence->GetBoneTransform(OutTM, BoneIndex, AtTime, bUseRawData);

		USkeleton* SourceSkeleton = InSequence->GetSkeleton();

		FReferenceSkeleton RefSkel = SourceSkeleton->GetReferenceSkeleton();

		if (RefSkel.IsValidIndex(BoneIndex))
		{
			int CurrentIndex = BoneIndex;
			//(RefSkel.GetParentIndex(CurrentIndex) != 0) && (
			if (CurrentIndex == 0)
			{
				InSequence->GetBoneTransform(OutTM, CurrentIndex, AtTime, bUseRawData);
				return;
			}
			while (RefSkel.GetParentIndex(CurrentIndex) != INDEX_NONE)
			{
				int ParentIndex = RefSkel.GetParentIndex(CurrentIndex);
				FTransform ParentTM;
				InSequence->GetBoneTransform(ParentTM, ParentIndex, AtTime, bUseRawData);

				OutTM = OutTM * ParentTM;
				CurrentIndex = ParentIndex;

			}
		}
		else
		{
			OutTM = FTransform::Identity;
		}
	}
}

void FMotionKeyUtils::GetAnimJointData(const UAnimSequence * InSequence, const float AtTime, const FName BoneName, Eigen::VectorXf& result,int index)
{
	if (InSequence && (BoneName != NAME_None))
	{

		USkeleton* SourceSkeleton = InSequence->GetSkeleton();

		FReferenceSkeleton RefSkel = SourceSkeleton->GetReferenceSkeleton();

		const int BoneIndex = RefSkel.FindBoneIndex(BoneName);

		//const float DeltaTime = 0.03f;
		
		FTransform RootTM;
		InSequence->GetBoneTransform(RootTM, 0, AtTime, false);

		FTransform BoneTM = FTransform::Identity;
		GetAnimBoneWorldTM(InSequence, AtTime, BoneIndex, BoneTM);

		FTransform PastBoneTM = FTransform::Identity;
		GetAnimBoneWorldTM(InSequence, AtTime - DeltaTime, BoneIndex, PastBoneTM);

		////Velocity
		FVector TmpVelo = BoneTM.GetLocation() - PastBoneTM.GetLocation();
		float UUS = TmpVelo.Size() / DeltaTime;

		SetVector2FeatureatIndex(RootTM.InverseTransformPositionNoScale(BoneTM.GetLocation()),result,index);

		SetVector2FeatureatIndex(RootTM.InverseTransformVectorNoScale(TmpVelo.GetSafeNormal()) * UUS,result,index+3);
		
	}
}

void FMotionKeyUtils::MakeGoal(FTrajectoryData & OutGoal, const FTransform DesiredTransform, const float TargetUUS, const FTransform RootWorldTM)
{
	OutGoal = FTrajectoryData();

	FTransform CurrentTM; 
	
	for (int i = 0; i < 10; i++)
	{
		float Alpho = ((float)(i + 1)) / ((float)10);

		CurrentTM.Blend(RootWorldTM, DesiredTransform, Alpho);

		FTransform PointTM = CurrentTM;
		//PointTM.SetTranslation(RootWorldTM.InverseTransformPosition(RootWorldTM.GetLocation() + (Dir * (TargetUUS * Alpho))));

		
		OutGoal.TrajectoryPoints.Add(FTrajectoryPoint(PointTM.GetRelativeTransform(RootWorldTM), 0.1f * i));

	}

	const FVector TrueDesDir = RootWorldTM.InverseTransformVectorNoScale(CurrentTM.GetRotation().GetForwardVector());

	OutGoal.EndVel = TrueDesDir * TargetUUS;


}

void FMotionKeyUtils::SetVector2FeatureatIndex(const FVector& a, Eigen::VectorXf& b, int index)
{
	b(index) = a.X;
	b(index + 1) = a.Y;
	b(index + 2) = a.Z;
}
