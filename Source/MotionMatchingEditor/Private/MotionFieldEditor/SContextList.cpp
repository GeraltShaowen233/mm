// Fill out your copyright notice in the Description page of Project Settings.

#include "SContextList.h"

#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "SScrubControlPanel.h"
#include "EditorStyleSet.h"
#include "SBorder.h"
#include "SScrollBox.h"
#include "SBox.h"
#include "SButton.h"
#include "STextBlock.h"
//#include "PaperSprite.h"
#include "AddNewContextDialog.h"



#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "ContextList"

void SSingleContextWidget::Construct(const FArguments& InArgs, int32 InIndex, TWeakPtr<FMotionFieldEditor> InMotionFieldEditor)
{
	ContextIndex = InIndex;
	MotionFieldEditorPtr = InMotionFieldEditor;
	this->SelectedAnimationSequence = MotionFieldEditorPtr.Pin().Get()->GetMotionFieldBeingEdited()->GetSrcAnimAtIndex(ContextIndex);
	
	TSharedRef<SWidget> removeButton = PropertyCustomizationHelpers::MakeDeleteButton(FSimpleDelegate::CreateSP(this, &SSingleContextWidget::RemoveContext),
		 LOCTEXT("RemoveContextToolTip", "Remove Context."), true);
	
	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SBox)
			.HeightOverride(50.0f)
			[
				SNew(SButton)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.OnClicked(this, &SSingleContextWidget::Selected)
					[
						SNew(SObjectPropertyEntryBox)
						.AllowedClass(UAnimSequence::StaticClass())
						.DisplayBrowse(false)
						.ObjectPath(SelectedAnimationSequence->GetPathName())
						.DisplayThumbnail(true)
						.AllowClear(true)
					]
			]
		]
	    + SHorizontalBox::Slot()
		.Padding(2.0f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			removeButton
		]
	];
}

FReply SSingleContextWidget::Selected()
{
	MotionFieldEditorPtr.Pin().Get()->SetCurrentExtractionContext(ContextIndex);

	return FReply::Handled();
}

void SSingleContextWidget::RemoveContext()
{
	MotionFieldEditorPtr.Pin().Get()->DeleteExtractionContext(ContextIndex);
}

void SAnimationWidget::Construct(const FArguments& InArgs, int32 InFrameIndex,
	TWeakPtr<FMotionFieldEditor> InMotionFieldEditor)
{
	ContextIndex = InFrameIndex;
	MotionFieldEditorPtr = InMotionFieldEditor;
	
}


void SContextList::Construct(const FArguments& InArgs, TWeakPtr<class FMotionFieldEditor> InMotionFieldEditor)
{
	MotionFieldEditorPtr = InMotionFieldEditor;

	MainBox = SNew(SVerticalBox);

	this->ChildSlot
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(SScrollBox)
						.Orientation(Orient_Vertical)
						+ SScrollBox::Slot()
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2, 0, 2, 2)
							[
								MainBox.ToSharedRef()
							]
						]
				]
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				[
						SNew(SButton)
						.Text(LOCTEXT("Add animation", "Add animation sequence"))
						.HAlign(HAlign_Center)
						.ToolTipText(LOCTEXT("AddContextToolTip", "Adds a new Sequence"))
						.OnClicked(FOnClicked::CreateSP(this, &SContextList::AddNewContextClicked))
				]
			]
		];

	Rebuild();
}


FReply SContextList::AddNewContextClicked()
{
	SAddNewContextDialog::ShowWindow(MotionFieldEditorPtr.Pin());
	return FReply::Handled();
}

void SContextList::Rebuild()
{
	MainBox->ClearChildren();
	int Num = MotionFieldEditorPtr.Pin().Get()->GetMotionFieldBeingEdited()->SourceAnimations.Num();
	if(!Num)
		return;
	for(int i = 0;i<Num;i++)
	{
		MainBox->AddSlot()
		.AutoHeight()
		[
			SNew(SSingleContextWidget, i, MotionFieldEditorPtr)
		];
	}
	
}


#undef LOCTEXT_NAMESPACE