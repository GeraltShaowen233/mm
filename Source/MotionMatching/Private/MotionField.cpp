
#include "MotionField.h"

#include <chrono>
#include <thread>

#include "MotionMatching.h"
#include "AnimationRuntime.h"
#include "MatrixCS.h"
#include "BehaviorTree/BTCompositeNode.h"


UMotionField::UMotionField(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	TimeStep = 0.1f;
	Skeleton = NULL;
	
	MotionKeys.Empty();

	MotionBones.Empty();
	CSM = std::make_shared<MatrixCS_Manager>();;
}

int UMotionField::GetLowestCostMotionKey
(
	const float Responsiveness, 
	const float VelocityStrength, 
	const float PoseStrength, 
	const Eigen::VectorXf& featureVector,
	float & OutLowestCost
)
{
	int WinnerIndex = INDEX_NONE;
	float LowestCost = 1000000000.f;

	auto GetFvectoratIndex = [&](const Eigen::VectorXf& a,int index)->FVector
	{
		return FVector(a(index),a(index+1),a(index+2));
	};

	auto ComputeCost = [&](const Eigen::VectorXf& a, const Eigen::VectorXf& b,
							const float Responsiveness, 
							const float VelocityStrength, 
							const float PoseStrength)->float
	{
		float scale = 0;
		float Cost = 0;
		for(int i =0; i<a.rows();i+=3)
		{
			if(i<a.rows()-12)
			{
				scale = PoseStrength;
			}
			else if(i<a.rows()-9)
			{
				scale = VelocityStrength;
			}
			else
			{
				scale = Responsiveness/3.0f;
			}

			Cost += /*scale **/FVector::Dist(GetFvectoratIndex(a,i),GetFvectoratIndex(b,i));
		}
		return Cost;
	};

	TArray<float> CostrenferenceArray;
	CostrenferenceArray.Init(0,featureDataBase.rows());
	if(featureDataBase.rows()>1&&featureDataBase.cols()>1)
	{
		
		auto t1 = std::chrono::system_clock::now();
		for(int i=0;i<featureDataBase.rows();i++)
		{
			float CurrentCost = ComputeCost(featureDataBase.row(i),featureVector,Responsiveness, VelocityStrength, PoseStrength);
			CostrenferenceArray[i] = CurrentCost;
			if (CurrentCost < LowestCost)
			{
				WinnerIndex = i;
				LowestCost = CurrentCost;
			}
			
		}
		auto t2 = std::chrono::system_clock::now();

		auto t3 = t2 - t1;

		
		ENQUEUE_RENDER_COMMAND(FComputeShaderRunner)(
	[this, &featureVector,&WinnerIndex,&Responsiveness,&VelocityStrength,&PoseStrength](FRHICommandListImmediate& RHICommands)
		{
			CSM->Compute(RHICommands,featureVector,Responsiveness,VelocityStrength,PoseStrength);
			this->GPUIndexArray.Add(CSM->WinIndex);
		
		});
		
		
	}
	else
	{
		RebakeAllAnim();
	}
	
	OutLowestCost = LowestCost;

	CPUIndexArray.Add(WinnerIndex);
	FlushRenderingCommands(true);
	
	return CSM->WinIndex;
	//return WinnerIndex;

	
}



void UMotionField::GetMotionFieldProperties(float & OutTimeStep, TArray<FString> & OutTags ) const
{
	OutTimeStep = TimeStep;
}

void UMotionField::SetProperties(const float InTimeStep, const TArray<FString> InTags)
{
	Modify();
	TimeStep = InTimeStep;

	RebakeAllAnim();

	MarkPackageDirty();
}

void UMotionField::ClearAnimMotionKeysAtAnimation(const int AtSourceAnimIndex)
{
	if(MotionKeys.Num())
	{
		for (int i = 0; i<MotionKeys.Num(); i++)
		{
			if (MotionKeys[i].SrcAnimIndex == AtSourceAnimIndex)
			{
				MotionKeys.RemoveAt(i);
			}
		}
		
	}
}
void UMotionField::ClearAllMotionKeys()
{
	Modify();
	
	MotionKeys.Empty();

	MarkPackageDirty();
}


void UMotionField::RebakeMotionKeysInAnim(const int  FromSourceAnimation)
{

	if (SourceAnimations[FromSourceAnimation])
	{
		Modify();

		ClearAnimMotionKeysAtAnimation(FromSourceAnimation);
		
		const float AnimLength = SourceAnimations[FromSourceAnimation]->GetPlayLength();

		for(float Time = 1.0f;Time<=AnimLength - 1.0f;Time+=TimeStep)
		{
			FMotionKey NewMotionKey = FMotionKey();

			NewMotionKey.ExtractDataFromAnimation(SourceAnimations[FromSourceAnimation], FromSourceAnimation, Time, MotionBones);

			MotionKeys.Add(NewMotionKey);
		}

		MarkPackageDirty();
	}
}

void UMotionField::RebakeAllAnim()
{
	ClearAllMotionKeys();
	if (SourceAnimations.Num() > 0)
	{
		for (int32 i = 0; i < SourceAnimations.Num(); i++)
		{
			RebakeMotionKeysInAnim(i);
		}

	}
	int dataBaseRows = 0;
	int dataBaseCols = 0;
	for(int i = 0;i<SourceAnimations.Num();i++)
	{
		dataBaseRows+= (SourceAnimations[i]->GetPlayLength() - 2)/TimeStep + 1;
	}
	dataBaseCols = MotionBones.Num()*3*2 + 3 + 3*3;

	featureDataBase.resize(dataBaseRows,dataBaseCols);
	BuildFeatureDataBase();
	CSM->Init(featureDataBase);
}

void UMotionField::BuildFeatureDataBase()
{
	int rowIndex = 0;
	for(int i = 0;i<SourceAnimations.Num();i++)
	{
		const float AnimLength = SourceAnimations[i]->GetPlayLength();
		float AccumulatedTime = 0.f;

		while (AccumulatedTime <= AnimLength)
		{
			if((AccumulatedTime >= 1.f) && (AccumulatedTime <= (AnimLength - 1.f)))
			{
				featureDataBase.row(rowIndex) = ExtractFeatureVectoratAnimation(SourceAnimations[i], i, AccumulatedTime, MotionBones);
				rowIndex += 1;
			}
			AccumulatedTime += TimeStep;

		}
	}
}

Eigen::VectorXf UMotionField::ExtractFeatureVectoratAnimation(const UAnimSequence* InSequence, const int AtSrcAnimIndex,
	const float AtSrcStartTime, TArray<FName> InMotionBoneNames)
{
	Eigen::VectorXf returnValue;
	returnValue.resize(InMotionBoneNames.Num()*2*3 + 3 + 3*3);
	for (int i = 0; i < InMotionBoneNames.Num(); i++)
	{
		FMotionKeyUtils::GetAnimJointData(InSequence, AtSrcStartTime, InMotionBoneNames[i],returnValue,i*6);
	}
	FVector velocity = FMotionKeyUtils::GetAnimVelocityAtTime(InSequence,AtSrcStartTime);
	Testvelocity.Add(velocity.Size());
	FMotionKeyUtils::SetVector2FeatureatIndex(velocity,returnValue,InMotionBoneNames.Num()*2*3);
	FTrajectoryData TF = FTrajectoryData();
	FMotionKeyUtils::ExtractAnimTrajectory(TF, InSequence, AtSrcStartTime);
	//FVector tra = TF.TrajectoryPoints.Last().m_TM.GetTranslation();
	
	for(int i=1;i<=3;i++)
	{
		FMotionKeyUtils::SetVector2FeatureatIndex(TF.TrajectoryPoints[i*3].m_TM,returnValue,InMotionBoneNames.Num()*2*3 + 3 + (i-1)*3);
	}

	return returnValue;
}

void UMotionField::AddSrcAnim(UAnimSequence * NewAnim)
{
	Modify();
	SourceAnimations.Add(NewAnim);
	
	RebakeAllAnim();
	MarkPackageDirty();
}

void UMotionField::DeleteSrcAnim(const int AtIndex)
{
	Modify();

	SourceAnimations.RemoveAt(AtIndex);
	
	RebakeAllAnim();

	MarkPackageDirty();
}


//#endif //WITH_EDITOR