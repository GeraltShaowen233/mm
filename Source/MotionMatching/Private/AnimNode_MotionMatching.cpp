// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNode_MotionMatching.h"

#include "MotionMatching.h"
#include "Engine/Engine.h"
#include "AnimationRuntime.h"
#include "MotCharacter.h"

#include "DualQuat.h"


bool FMotionAnimation::ApplyTime(const float DT, const float InBlendTime, const bool whetherStart)
{
	Position += DT;
	float x = 0;
	if((!whetherStart&&Maxed))
	{
		x = BlendTime -DT;
	}
	else
	{
		x = BlendTime + DT;
	}
	BlendTime = FMath::Clamp(x,0.0f,InBlendTime);
	if(BlendTime==0.0f)
		return true;
	
	if (BlendTime == InBlendTime)
	{
		Maxed = true;
	}
	
	Weight = FMath::Clamp((BlendTime / InBlendTime) , 0.f, 1.f);
	return false;
}


FAnimNode_MotionMatching::FAnimNode_MotionMatching()
	: MotionField(NULL)
	, DesiredTrajectory(FTrajectoryData())
	, Responsiveness(1.f)
	, VelocityStrength(1.f)
	, PoseStrength(1.f)
	, BlendTime(0.3f)
	, PlayRate(1.f)
	, bLoopAnimation(true)
{
	m_Anims.Empty();
	
}

float FAnimNode_MotionMatching::GetCurrentAssetTime()
{
	return InternalTimeAccumulator;
}

float FAnimNode_MotionMatching::GetCurrentAssetTimePlayRateAdjusted()
{
	//assume no negative play rate  
	return GetCurrentAssetTime();
}

float FAnimNode_MotionMatching::GetCurrentAssetLength()
{
	return GetCurrentAnim() ? GetCurrentAnim()->SequenceLength : 0.0f;
}

void FAnimNode_MotionMatching::Initialize_AnyThread(const FAnimationInitializeContext & Context)
{

	FAnimNode_Base::Initialize_AnyThread(Context);
	GetEvaluateGraphExposedInputs().Execute(Context);
	
	InternalTimeAccumulator = 0.f;
	if(MotionField)
	{
		UAnimSequence* Sequence = GetCurrentAnim();
		if(Sequence)
			InternalTimeAccumulator = Sequence->SequenceLength;
	}
}

void FAnimNode_MotionMatching::UpdateAssetPlayer(const FAnimationUpdateContext& Context)
{
	GetEvaluateGraphExposedInputs().Execute(Context);

	if (MotionField)
	{
			MotionUpdate(Context);
			Compute();
			
			if (GetCurrentAnim())
			{

				CreateTickRecordForNode(Context, GetCurrentAnim(), bLoopAnimation, PlayRate);
			}
			
			
	}
}


void FAnimNode_MotionMatching::Evaluate_AnyThread(FPoseContext & Output)
{
	if (m_Anims.Num() > 1)
	{
		FTransform RootMotion;

		GetBlendPose(Output.AnimInstanceProxy->GetDeltaSeconds(), RootMotion, Output.Pose, Output.Curve, Output.CustomAttributes);
		auto C = Cast<AMotCharacter>(Output.AnimInstanceProxy->GetSkelMeshComponent()->GetOwner());
		if(C)
			C->OverrideRootMotion(RootMotion);
	}
	else if(m_Anims.Num()==1)
	{

		FAnimationPoseData PoseData(Output.Pose, Output.Curve, Output.CustomAttributes);
		GetCurrentAnim()->GetAnimationPose(PoseData, FAnimExtractContext(m_Anims.Last().Position, true));
	}
	else
		Output.Pose.ResetToRefPose();
	

}

void FAnimNode_MotionMatching::PlayAnimStartingAtTime(const int32 AnimIndex, const float StartingTime)
{
	InternalTimeAccumulator = StartingTime;

	FMotionAnimation NewAnim = FMotionAnimation(AnimIndex, StartingTime);
	m_Anims.Add(NewAnim);
	
}

UAnimSequence * FAnimNode_MotionMatching::GetCurrentAnim()
{

	return m_Anims.Num() >0 ? MotionField->GetSrcAnimAtIndex(m_Anims.Last().AnimIndex) : nullptr;
}

UAnimSequence * FAnimNode_MotionMatching::AnimAtIndex(const int32 Index)
{
	return m_Anims.Num() > 0 ? MotionField->GetSrcAnimAtIndex(m_Anims[Index].AnimIndex): nullptr;
}



void FAnimNode_MotionMatching::MotionUpdate(const FAnimationUpdateContext& Context)
{

	const float DT = Context.GetDeltaTime();

	if(m_Anims.Num()>1)
	{
		for(int i = m_Anims.Num()-1;i>=0;i--)
		{
			if (m_Anims[i].ApplyTime(DT, BlendTime, i == m_Anims.Num() - 1))
			{
				m_Anims.RemoveAt(i);
			}
			/*
			else
			{
				const float FinalBlendWeight = Context.GetFinalBlendWeight() * m_Anims[i].GlobalWeight;

				FAnimGroupInstance* SyncGroup = nullptr;
				const FName GroupNameToUse = ((GroupRole < EAnimGroupRole::TransitionLeader) || bHasBeenFullWeight) ? GroupName : NAME_None;
				
				FAnimTickRecord& TickRecord = Context.AnimInstanceProxy->CreateUninitializedTickRecordInScope(SyncGroup, GroupNameToUse, GroupScope);
				Context.AnimInstanceProxy->MakeSequenceTickRecord(TickRecord, MotionField->SourceAnimations[m_Anims[i].AnimIndex], true, 1.0f, FinalBlendWeight, m_Anims[i].Position, m_Anims[i].MarkerTickRecord);
				TickRecord.RootMotionWeightModifier = Context.GetRootMotionWeightModifier();

				// Update the sync group if it exists
				if (SyncGroup != NULL)
				{
					SyncGroup->TestTickRecordForLeadership(GroupRole);
				}
				
				TRACE_ANIM_TICK_RECORD(Context, TickRecord);
			}
			*/
			
			
		}
	}

}

void FAnimNode_MotionMatching::Compute()
{
	if (MotionField)
	{
		if (m_Anims.Num())
		{
				GetMotionData(featureVector);

				float OutCost;

				int Winner = MotionField->GetLowestCostMotionKey(Responsiveness, VelocityStrength, PoseStrength,featureVector, OutCost);

				if (Winner >= 0)
				{
					if((MotionField->MotionKeys[Winner].SrcAnimIndex == m_Anims.Last().AnimIndex)&&
						(fabs(MotionField->MotionKeys[Winner].StartTime - m_Anims.Last().Position) < .2f))
					{
						return;
					}
					else
					{
						CurrentMotionKeyIndex = Winner;

						PlayAnimStartingAtTime(MotionField->MotionKeys[Winner].SrcAnimIndex, MotionField->MotionKeys[Winner].StartTime);
					}
				}
		}
		else
		{
			PlayAnimStartingAtTime(0, 0.f);
		}

	}
}


void FAnimNode_MotionMatching::GetMotionData(Eigen::VectorXf& vector)
{
	featureVector.resize(MotionField->MotionBones.Num()*2*3 + 3 + 3*3);

	for (int32 i = 0; i < MotionField->MotionBones.Num(); i++)
	{
		FMotionKeyUtils::GetAnimJointData(GetCurrentAnim(),m_Anims.Last().Position,MotionField->MotionBones[i],vector,i*6);
	}
	FMotionKeyUtils::SetVector2FeatureatIndex(FMotionKeyUtils::GetAnimVelocityAtTime(GetCurrentAnim(), m_Anims.Last().Position),vector,MotionField->MotionBones.Num()*2*3);
	if(DesiredTrajectory.TrajectoryPoints.Num()<1)
	{
		for(int i=0;i<3;i++)
		{
			FMotionKeyUtils::SetVector2FeatureatIndex(FVector(0,0,0),vector,MotionField->MotionBones.Num()*2*3 + 3 + i*3);
		}
		
	}
	else
	{
		for(int i=1;i<=3;i++)
			FMotionKeyUtils::SetVector2FeatureatIndex(DesiredTrajectory.TrajectoryPoints[i*3].m_TM,vector,MotionField->MotionBones.Num()*2*3 + 3 + (i-1)*3);
	}
		
}



void FAnimNode_MotionMatching::GetBlendPose(const float DT, FTransform & OutRootMotion, FCompactPose & OutPose, FBlendedCurve & OutCurve, FStackCustomAttributes& CustomAttributes)
{
	
	const int32 NumPoses = m_Anims.Num();
	OutRootMotion = FTransform::Identity;
	/*
	if (NumPoses > 1)
	{
		TArray<FCompactPose, TInlineAllocator<8>> ChildrenPoses;
		ChildrenPoses.AddZeroed(NumPoses);

		TArray<FBlendedCurve, TInlineAllocator<8>> ChildrenCurves;
		ChildrenCurves.AddZeroed(NumPoses);

		TArray<FStackCustomAttributes, TInlineAllocator<8>> ChildrenAttributes;
		ChildrenAttributes.AddZeroed(NumPoses);

		TArray<float, TInlineAllocator<8>> ChildrenWeights;
		ChildrenWeights.AddZeroed(NumPoses);


		for (int32 ChildrenIdx = 0; ChildrenIdx < ChildrenPoses.Num(); ++ChildrenIdx)
		{
			ChildrenPoses[ChildrenIdx].SetBoneContainer(&(OutPose.GetBoneContainer()));
			ChildrenCurves[ChildrenIdx].InitFrom(OutCurve);
			ChildrenAttributes[ChildrenIdx].CopyFrom(CustomAttributes);
		}

		float RestAlpha = 1.0f;
		float SumAlpha = 0.0f;
		for (int32 i = NumPoses - 1; i >= 0; i--)
		{
			FCompactPose& Pose = ChildrenPoses[i];
			FBlendedCurve& Curve = ChildrenCurves[i];
			FStackCustomAttributes& Attributes = ChildrenAttributes[i];
			FAnimationPoseData AnimationPoseData(Pose, Curve, Attributes);
			if (i > 0)
			{
				m_Anims[i].GlobalWeight = FMath::Clamp(m_Anims[i].LocalWeight * RestAlpha, 0.0f, 1.0f);
				ChildrenWeights[i] = m_Anims[i].GlobalWeight;
				SumAlpha += ChildrenWeights[i];
				RestAlpha *= 1.0 - m_Anims[i].LocalWeight;
			}
			else
			{
				m_Anims[i].GlobalWeight = 1 - SumAlpha;
				ChildrenWeights[i] = m_Anims[i].GlobalWeight;
			}
			const float Time = m_Anims[i].Position;
			MotionField->SourceAnimations[m_Anims[i].AnimIndex]->GetAnimationPose(AnimationPoseData, FAnimExtractContext(Time, true));
			ChildrenRootMotions[i] = AnimAtIndex(i)->ExtractRootMotion(Time - DT, DT, true);
		}
		TArrayView<FCompactPose> ChildrenPosesView(ChildrenPoses);
		FAnimationPoseData AnimationPoseData(OutPose, OutCurve, CustomAttributes);
		FAnimationRuntime::BlendPosesTogether(ChildrenPosesView, ChildrenCurves, ChildrenAttributes, ChildrenWeights, AnimationPoseData);
	}
	else
	{
		FAnimationPoseData AnimationPoseData(OutPose, OutCurve, CustomAttributes);
		GetCurrentAnim()->GetAnimationPose(AnimationPoseData, FAnimExtractContext(m_Anims.Last().Position, true));
	}
	*/
	
	if (NumPoses)
	{
		TArray<FCompactPose, TInlineAllocator<8>> ChildrenPoses;
		ChildrenPoses.SetNumZeroed(NumPoses);

		TArray<FBlendedCurve, TInlineAllocator<8>> ChildrenCurves;
		ChildrenCurves.SetNumZeroed(NumPoses);

		TArray<float, TInlineAllocator<8>> ChildrenWeights;
		ChildrenWeights.SetNumZeroed(NumPoses);

		TArray<FStackCustomAttributes, TInlineAllocator<8>> ChildrenAttributes;
		ChildrenAttributes.SetNumZeroed(NumPoses);

		TArray<FTransform> ChildrenRootMotions;
		ChildrenRootMotions.SetNumZeroed(NumPoses);

		for (int32 ChildrenIdx = 0; ChildrenIdx<ChildrenPoses.Num(); ++ChildrenIdx)
		{
			ChildrenPoses[ChildrenIdx].SetBoneContainer(&(OutPose.GetBoneContainer()));
			ChildrenCurves[ChildrenIdx].InitFrom(OutCurve);
			ChildrenAttributes[ChildrenIdx].CopyFrom(CustomAttributes);
		}
		
		float SumAlpha = 0.0f;
		
		for (int32 I = 0; I < NumPoses; I++)
		{
			ChildrenWeights[I] = m_Anims[I].Weight * (static_cast<float>(I + 1) / static_cast<float>(NumPoses));
			SumAlpha += ChildrenWeights[I];
			const float Time = FMath::Clamp<float>(m_Anims[I].Position, 0.f, AnimAtIndex(I)->SequenceLength);
			FAnimationPoseData PoseDataInfo(ChildrenPoses[I], ChildrenCurves[I], CustomAttributes);
			AnimAtIndex(I)->GetAnimationPose(PoseDataInfo, FAnimExtractContext(Time, true));
			ChildrenRootMotions[I] = AnimAtIndex(I)->ExtractRootMotion(Time - DT, DT, true);
		}
		
		for(int32 i = 0; i < ChildrenWeights.Num(); i++)
		{
			ChildrenWeights[i] = ChildrenWeights[i] / (SumAlpha + 0.0001);
		}
		TArrayView<FCompactPose> ChildrenPosesView(ChildrenPoses);
		FAnimationRuntime::BlendPosesTogether(ChildrenPosesView, ChildrenCurves, ChildrenWeights, OutPose, OutCurve);

		FRootMotionMovementParams MotMotion;
			
		for (int j = 0; j < NumPoses; j++)
		{
			MotMotion.AccumulateWithBlend(ChildrenRootMotions[j], ChildrenWeights[j]);
		}

		OutRootMotion = MotMotion.GetRootMotionTransform();
		OutRootMotion.NormalizeRotation();

	}
	else
	{
		FAnimationPoseData PoseInfo(OutPose, OutCurve, CustomAttributes);
		GetCurrentAnim()->GetAnimationPose(PoseInfo, FAnimExtractContext(m_Anims.Last().Position, true));
	}
	
}

