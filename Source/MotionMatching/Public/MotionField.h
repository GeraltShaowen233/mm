

#pragma once

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"

#include "MotionKey.h"
#include<Eigen/Eigen>
#include "Animation/Skeleton.h"
#include <memory>
#include "MotionField.generated.h"



class USkeleton;
struct FMotionExtractionContext;
class MatrixCS_Manager;

UCLASS(BlueprintType)
class MOTIONMATCHING_API UMotionField : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
		float TimeStep;
	UPROPERTY()
		USkeleton* Skeleton;

public:

	    UMotionField(const FObjectInitializer& ObjectInitializer);
	
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionKeys")
		TArray <FMotionKey> MotionKeys;
	
		UPROPERTY(BlueprintReadOnly, Category = "BoneData")
		TArray <FName> MotionBones;
		
		UPROPERTY(BlueprintReadOnly, Category = "Animations")
		TArray <UAnimSequence*> SourceAnimations;

		Eigen::MatrixXf featureDataBase;
		std::shared_ptr<MatrixCS_Manager> CSM;

		TArray<int> CPUIndexArray;
		TArray<int> GPUIndexArray;
		TArray<float> Testvelocity;
		//UFUNCTION()
		int GetLowestCostMotionKey
		(
			const float Responsiveness, 
			const float VelocityStrength, 
			const float PoseStrength, 
			const Eigen::VectorXf& featureVector,
			float & OutLowestCost
		);
	
		void GetMotionFieldProperties(float & OutTimeStep, TArray<FString> & OutTags) const;
		void SetProperties(const float InTimeStep, const TArray <FString> InTags);
		
		float GetTimeStep() const
		{
			return TimeStep;
		}
			

		bool IsValidMotionKeyIndex(const int Index)
		{
			return MotionKeys.IsValidIndex(Index);
		}
		int GetNumMotionKey()
		{
			return MotionKeys.Num();
		}
		const FMotionKey& GetKeyFrameChecked(const int FrameIndex) const
		{
			return MotionKeys[FrameIndex];
		}
		
		UAnimSequence* GetMotionKeyAnimSequence(const int AtMotionKeyIndex)
		{
			return SourceAnimations[MotionKeys[AtMotionKeyIndex].SrcAnimIndex];
		}
	

		void ClearAnimMotionKeysAtAnimation(const int AtSourceAnimIndex);

		void ClearAllMotionKeys();
		
		void RebakeMotionKeysInAnim(const int FromSourceAnimation);
		void RebakeAllAnim();

		void BuildFeatureDataBase();
		Eigen::VectorXf ExtractFeatureVectoratAnimation(const UAnimSequence * InSequence, const int AtSrcAnimIndex, const float AtSrcStartTime, TArray <FName> InMotionBoneNames);


		int GetSrcAnimNum()
		{
			return SourceAnimations.Num();
		}

		void AddSrcAnim(UAnimSequence* NewAnim);
		

		void DeleteSrcAnim(const int AtIndex);

		bool IsValidSrcAnimIndex(const int AtIndex)
		{
			return SourceAnimations.IsValidIndex(AtIndex);
		}

		UAnimSequence* GetSrcAnimAtIndex(const int Index)
		{
				return SourceAnimations[Index];
		}

		bool IsExtractedFrame(const FName AnimName, const float Time);

		USkeleton* GetMotionFieldSkeleton()
		{
			return Skeleton;
		}

		void SetMotionFieldSkeleton(USkeleton* InSkeleton)
		{
			Skeleton = InSkeleton;
		}

		void PopulateFromSkeleton(USkeleton * SourceSkeleton, const TArray<FName> InMotionBones)
		{
			Skeleton = SourceSkeleton;
			MotionBones = InMotionBones;
		}
	
};
