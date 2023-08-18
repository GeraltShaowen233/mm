
#pragma once
#include "AnimGraphNode_Base.h"
#include "AnimNode_MotionMatching.h"
#include "AnimGraphNode_MotionMatching.generated.h"
UCLASS()
class MOTIONMATCHINGEDITOR_API UAnimGraphNode_MotionMatching : public UAnimGraphNode_Base
{
	GENERATED_BODY()
		UPROPERTY(EditAnywhere, Category = Settings)
		FAnimNode_MotionMatching Node;
	
	virtual FLinearColor GetNodeTitleColor() const override;
	UAnimGraphNode_MotionMatching(const FObjectInitializer& ObjectInitializer);
};
