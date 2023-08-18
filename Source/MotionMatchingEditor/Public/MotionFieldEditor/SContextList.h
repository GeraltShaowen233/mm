// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "MotionFieldEditor.h"
#include "ContentBrowserDelegates.h"
#include "PropertyEditor/Public/PropertyCustomizationHelpers.h"



//////////////////////////////////////////////////////////////////////////
// SSpriteList

class SBorder;
class SScrollBox;
class SBox;
class SButton;
class STextBlock;
class SObjectPropertyEntryBox;



class SSingleContextWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSingleContextWidget) {}
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, int32 InFrameIndex, TWeakPtr<class FMotionFieldEditor> InMotionFieldEditor);

	FReply Selected();
	
protected:
	int32 ContextIndex;
	
private:
	TWeakPtr<class FMotionFieldEditor> MotionFieldEditorPtr;
	UPROPERTY(EditAnywhere)
	UAnimSequence* SelectedAnimationSequence;
	void RemoveContext();
};

class SAnimationWidget : public SObjectPropertyEntryBox
{
public:
	SLATE_BEGIN_ARGS(SAnimationWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, int32 InFrameIndex, TWeakPtr<class FMotionFieldEditor> InMotionFieldEditor);
	
protected:

protected:
	int32 ContextIndex;
	
private:
	TWeakPtr<class FMotionFieldEditor> MotionFieldEditorPtr;
	
};

class SContextList : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SContextList) {}
	SLATE_END_ARGS()
public:
	void Construct(const FArguments& InArgs, TWeakPtr<class FMotionFieldEditor> InMotionFieldEditor);
	
	
	FReply AddNewContextClicked();

	void Rebuild();
	
protected:
	TWeakPtr<class FMotionFieldEditor> MotionFieldEditorPtr;

	TSharedPtr<SVerticalBox> MainBox;
	
	FSyncToAssetsDelegate SyncToAssetsDelegate;
};
