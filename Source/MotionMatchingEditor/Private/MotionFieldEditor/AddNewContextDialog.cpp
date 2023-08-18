
#include "AddNewContextDialog.h"
#include "MotionMatchingEditor.h"
#include "MotionFieldEditor/MotionFieldEditor.h"


#include "Widgets/SBoxPanel.h"
#include "Widgets/SWindow.h"
#include "Widgets/SViewport.h"
#include "Misc/FeedbackContext.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Misc/PackageName.h"
#include "Layout/WidgetPath.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SButton.h"
#include "Framework/Docking/TabManager.h"
#include "EditorStyleSet.h"
#include "CanvasItem.h"

#include "PropertyEditorModule.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "CanvasTypes.h"

#include "Factories/DataAssetFactory.h"

#include "MotionKey.h"

#include "SScrollBox.h"

#include "Animation/AnimSequence.h"
//////////////////////////////////////////

#include "AssetRegistryModule.h"

#include "AssetThumbnail.h"
#include "NetworkMessage.h"

#include "STextBlock.h"
#include "SBox.h"

#define LOCTEXT_NAMESPACE "MotionMatchingEditor"


void SAddNewContextDialog::Construct(const FArguments & InArgs, TSharedPtr<FMotionFieldEditor> InMotionFieldEditorPtr)
{
	MotionFieldEditorPtr = InMotionFieldEditorPtr;
	SkeletonName = MotionFieldEditorPtr.Get()->GetMotionFieldBeingEdited()->GetMotionFieldSkeleton()->GetName();

	MainBox = SNew(SVerticalBox);

	FAssetPickerConfig PickerConfig;
	PickerConfig.Filter.ClassNames.Add(UAnimSequence::StaticClass()->GetFName());
	PickerConfig.bAutohideSearchBar = true;
	PickerConfig.ThumbnailLabel = EThumbnailLabel::ClassName;
	PickerConfig.GetCurrentSelectionDelegates.Add(&GetCurrentSelectionDelegate);
	PickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &SAddNewContextDialog::FilterAnim);
	PickerConfig.InitialAssetViewType = EAssetViewType::List;
	PickerConfig.ThumbnailScale = 64.f;
	PickerConfig.bShowPathInColumnView = true;
	
	MainBox->AddSlot()
		[
			FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser")).Get().CreateAssetPicker(PickerConfig)
		];
	
	ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
					[
						MainBox.ToSharedRef()
					]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
					.ForegroundColor(FLinearColor::White)
					.Text(LOCTEXT("AddButton", "Add"))
					.OnClicked(this, &SAddNewContextDialog::AddClicked)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				.AutoWidth()
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton")
					.ForegroundColor(FLinearColor::White)
					.Text(LOCTEXT("CancelButton", "Cancel"))
					.OnClicked(this, &SAddNewContextDialog::CancelClicked)
				]
					
			]
		];
		
}

SAddNewContextDialog::~SAddNewContextDialog()
{
}

void SAddNewContextDialog::ShowWindow(TSharedPtr<FMotionFieldEditor> InMotionFieldEditorPtr)
{
	// Create the window to pick the class
	TSharedRef<SWindow> AddNewContextWindow = SNew(SWindow)
		.Title(NSLOCTEXT("MotionMatchingEditor", "MotionMatchingEditor_AddNewContext", "Add New Context"))
		.ClientSize(FVector2D(550.f, 1000.f))
		.SupportsMinimize(true);

	TSharedRef<SAddNewContextDialog> AddNewContextDialog = SNew(SAddNewContextDialog, InMotionFieldEditorPtr);

	AddNewContextWindow->SetContent(AddNewContextDialog);
	
	FSlateApplication::Get().AddWindow(AddNewContextWindow);
}

bool SAddNewContextDialog::FilterAnim(const FAssetData & AssetData)
{
	if (!AssetData.IsAssetLoaded())
	{
		AssetData.GetPackage()->FullyLoad();
	}

	for(int i = 0;i< MotionFieldEditorPtr.Get()->GetMotionFieldBeingEdited()->SourceAnimations.Num();i++)
	{
		if(MotionFieldEditorPtr.Get()->GetMotionFieldBeingEdited()->GetSrcAnimAtIndex(i)->GetFName()==AssetData.AssetName)
			return true;
	}

	if(SkeletonName != Cast<UAnimSequence>(AssetData.GetAsset())->GetSkeleton()->GetName())
		return true;

	return false;
}

FReply SAddNewContextDialog::AddClicked()
{
	
	TArray<FAssetData> SelectionArray = GetCurrentSelectionDelegate.Execute();

	if(SelectionArray.Num())
	{
		for(int i=0;i<SelectionArray.Num();i++)
		{
			UAnimSequence* tempAnim = Cast <UAnimSequence>(SelectionArray[i].GetAsset());
			if(tempAnim)
			{
				MotionFieldEditorPtr.Get()->GetMotionFieldBeingEdited()->AddSrcAnim(tempAnim);
			}
		}
		MotionFieldEditorPtr.Get()->rebuildContext();

		CloseContainingWindow();
	}
	
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoAnimationsSelected", "try to reSelect"));
	}
	
	return FReply::Handled();
}

FReply SAddNewContextDialog::CancelClicked()
{
	CloseContainingWindow();
	return FReply::Handled();
}

void SAddNewContextDialog::CloseContainingWindow()
{
	FWidgetPath Path;
	auto ContainingWindow = FSlateApplication::Get().FindWidgetWindow(AsShared(), Path);
	if (ContainingWindow.IsValid())
	{
		ContainingWindow->RequestDestroyWindow();
	}
}

#undef LOCTEXT_NAMESPACE
