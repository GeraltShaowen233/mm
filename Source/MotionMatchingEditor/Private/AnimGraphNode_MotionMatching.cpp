// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimGraphNode_MotionMatching.h"
#include "MotionMatchingEditor.h"


#define LOCTEXT_NAMESPACE "A3Nodes"

UAnimGraphNode_MotionMatching::UAnimGraphNode_MotionMatching(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	
}

FLinearColor UAnimGraphNode_MotionMatching::GetNodeTitleColor() const
{
	return FLinearColor::Gray;
}


#undef LOCTEXT_NAMESPACE
