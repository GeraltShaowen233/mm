// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionField_Factory.h"

#include "InputCoreTypes.h"
#include "UObject/Interface.h"
#include "Layout/Visibility.h"
#include "Input/Reply.h"
#include "Widgets/SWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimInstance.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SButton.h"
#include "EditorStyleSet.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "AssetData.h"
#include "Editor.h"
#include "Kismet2/KismetEditorUtilities.h"


#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"

#include "IAssetTools.h"
#include "AssetToolsModule.h"

#include "SScrollBox.h"
#include "SCheckBox.h"
#include "SColorBlock.h"

#define LOCTEXT_NAMESPACE "MotionFieldFactory"


class SSingleBoneItem : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSingleBoneItem) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const int32 InBoneIndex, const bool DefaultSelected, const FName InBoneName)//, TWeakPtr<SMotionFieldCreateDialog> InMotionFieldEditorPtr)
	{
		BoneName = InBoneName;
		bIsMotion = false;
		ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock).Text(FText::FromName(BoneName))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.HAlign(HAlign_Center)
					[
						SNew(SCheckBox)
						.OnCheckStateChanged(this, &SSingleBoneItem::OnCheckIsMotion)
						.IsChecked(ECheckBoxState::Unchecked)
						.Padding(10.f)
						.ForegroundColor(this, &SSingleBoneItem::GetIsMotionColor)
					]
				]
			]
		];

			
	}

	void OnCheckIsMotion(ECheckBoxState NewState)
	{
		bIsMotion = NewState == ECheckBoxState::Checked ? true : NewState == ECheckBoxState::Unchecked ? false : bIsMotion;
	}
	

	FSlateColor GetIsMotionColor() const
	{
		return bIsMotion ?  FSlateColor(FLinearColor::Green) : FSlateColor(FLinearColor::Red);
	}

	FName BoneName;
	
	bool bIsMotion;
};



class SMotionFieldCreateDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMotionFieldCreateDialog) {}

	SLATE_END_ARGS()

		// Constructs this widget with InArgs 
		void Construct(const FArguments& InArgs, const bool Ow)
	{
		bOkClicked = false;

		ChildSlot
		[
		    SNew(SBorder)
		    .BorderImage(FEditorStyle::GetBrush("Menu.Background"))
		    .Padding(8.0f)
		    [
		        SNew(SBox)
		        .WidthOverride(500.0f)
		        [
		            SNew(SVerticalBox)
		            + SVerticalBox::Slot()
		            .FillHeight(1)
		            .Padding(0.0f, 10.0f)
		            [
		                SNew(SBorder)
		                .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		                [
		                    SAssignNew(SkeletonContainer, SVerticalBox)
		                ]
		            ]
		            + SVerticalBox::Slot()
		            .FillHeight(0.9f)
		            [
		                SNew(SScrollBox)
		                + SScrollBox::Slot()
		                [
		                    SAssignNew(SkeletonBoneContainer, SVerticalBox)
		                ]
		            ]
		            + SVerticalBox::Slot()
		            .AutoHeight()
		            .HAlign(HAlign_Center)
		            [
		                SNew(SUniformGridPanel)
		                .SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
		                .MinDesiredSlotWidth(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
		                .MinDesiredSlotHeight(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
		
		                + SUniformGridPanel::Slot(0, 0)
		                [
		                    SNew(SButton)
		                    .ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
		                    .OnClicked(this, &SMotionFieldCreateDialog::OkClicked)
		                    .Text(LOCTEXT("CreateMotionFieldOk", "OK"))
		                ]
		
		                + SUniformGridPanel::Slot(1, 0)
		                [
		                    SNew(SButton)
		                    .ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
		                    .OnClicked(this, &SMotionFieldCreateDialog::CancelClicked)
		                    .Text(LOCTEXT("CreateMotionFieldCancel", "Cancel"))
		                ]
		            ]
		        ]
		    ]
		];
		MakeSkeletonPicker();
		RebuildBonePicker();
	}
	
	bool ConfigureProperties(TWeakObjectPtr<UMotionFieldFactory> InMotionFieldFactory)
	{
		MotionFieldFactory = InMotionFieldFactory;

		TSharedRef<SWindow> Window = SNew(SWindow)
			.Title(LOCTEXT("CreateMotionField", "CreateAnimationBlueprint"))
			.ClientSize(FVector2D(400, 700))
			.SupportsMinimize(false).SupportsMaximize(false)
			[
				AsShared()
			];
		PickerWindow = Window;
		GEditor->EditorAddModalWindow(Window);
		MotionFieldFactory.Reset();
		return bOkClicked;
	}

private:
	class FMotionFieldParentFilter : public IClassViewerFilter
	{
	public:
		TSet< const UClass* > AllowedChildrenOfClasses;
		const FAssetData& ShouldBeCompatibleWithSkeleton;

		FMotionFieldParentFilter(const FAssetData& Skeleton) : ShouldBeCompatibleWithSkeleton(Skeleton) {}

		virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
		{
			return InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) != EFilterReturn::Failed;
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InitOptions, const TSharedRef< const IUnloadedBlueprintData > UnloadedClassData, TSharedRef< FClassViewerFilterFuncs > FilterFuncs) override
		{
			return FilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, UnloadedClassData) != EFilterReturn::Failed;
		}
	};
	
	
	void MakeSkeletonPicker()
	{
		FContentBrowserModule* CBModule = &FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

		FAssetPickerConfig Config;
		Config.Filter.ClassNames.Emplace(USkeleton::StaticClass()->GetFName());
		Config.bAllowNullSelection = true;
		Config.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SMotionFieldCreateDialog::OnSkeletonSelected);
		Config.InitialAssetViewType = EAssetViewType::Column;
		Config.InitialAssetSelection = TargetSkeleton;

		SkeletonContainer->ClearChildren();

		SkeletonContainer->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SkeletonChoice", "Chosen Skeleton:"))
			.ShadowOffset(FVector2D(1, 1))
		];

		TSharedRef<SWidget> AssetPicker = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser")).Get().CreateAssetPicker(Config);
		SkeletonContainer->AddSlot()[AssetPicker];
	}

	void RebuildBonePicker()
	{
		BoneItems.Empty();
		SkeletonBoneContainer->ClearChildren();
		if (TargetSkeleton.IsValid())
		{
			if (TargetSkeleton.IsAssetLoaded())
			{
				USkeleton* SelectedSkeleton = Cast<USkeleton>(TargetSkeleton.GetAsset());
				if (SelectedSkeleton)
				{
					const FReferenceSkeleton& ReferenceSkeleton = SelectedSkeleton->GetReferenceSkeleton();
					for (int32 BoneIndex = 1; BoneIndex < ReferenceSkeleton.GetNum(); BoneIndex++)
					{
						FName BoneName = ReferenceSkeleton.GetBoneName(BoneIndex);
						TSharedRef<SSingleBoneItem> BoneItemWidget = SNew(SSingleBoneItem, BoneIndex, false, BoneName);
        
						BoneItems.Add(BoneItemWidget);
						SkeletonBoneContainer->AddSlot()
						[
							BoneItemWidget
						];
					}
				}
			}
			else
			{
				TargetSkeleton.GetPackage()->FullyLoad();
			}
			
		}
	}
	
	void OnSkeletonSelected(const FAssetData& AssetData)
	{
		TargetSkeleton = AssetData;
		RebuildBonePicker();
	}
	
	FReply OkClicked()
	{
		MotionFieldFactory->TargetMotionBones.Empty();

		for (const auto& BoneItem : BoneItems)
		{
			if (BoneItem.IsValid() && BoneItem.Get()->bIsMotion)
			{
				MotionFieldFactory->TargetMotionBones.Add(BoneItem.Get()->BoneName);
			}
		}

		if (MotionFieldFactory.IsValid())
		{
			MotionFieldFactory->TargetSkeleton = Cast<USkeleton>(TargetSkeleton.GetAsset());
		}

		CloseDialog(true);
		return FReply::Handled();
	}

	void CloseDialog(bool bWasPicked = false)
	{
		bOkClicked = bWasPicked;
		if (PickerWindow.IsValid())
		{
			PickerWindow.Pin()->RequestDestroyWindow();
		}
	}
	
	FReply CancelClicked()
	{
		CloseDialog();
		return FReply::Handled();
	}

private:
	TWeakObjectPtr<UMotionFieldFactory> MotionFieldFactory;
	
	TWeakPtr<SWindow> PickerWindow;
	
	TSharedPtr<SVerticalBox> SkeletonContainer;
	TSharedPtr<SVerticalBox> SkeletonBoneContainer;
	TArray <TSharedPtr <SSingleBoneItem>> BoneItems;
	
	FAssetData TargetSkeleton;
	TArray <FName> TargetMotionBones;
	bool bOkClicked;
};



//UMotionFieldFactory implementation.

UMotionFieldFactory::UMotionFieldFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UMotionField::StaticClass();
}

bool UMotionFieldFactory::ConfigureProperties()
{
	
	TSharedRef<SMotionFieldCreateDialog> Dialog = SNew(SMotionFieldCreateDialog, true);
	return Dialog->ConfigureProperties(this);
};

UObject* UMotionFieldFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	
	auto CreatedMotionField = Cast<UMotionField>(StaticConstructObject_Internal(Class, InParent, Name, Flags | RF_Transactional));

	if (CreatedMotionField && TargetSkeleton)
	{
		CreatedMotionField->PopulateFromSkeleton(TargetSkeleton, TargetMotionBones);
		return CreatedMotionField;
	}

	return nullptr;
}

UObject* UMotionFieldFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FactoryCreateNew(Class, InParent, Name, Flags, Context, Warn, NAME_None);
}

#undef LOCTEXT_NAMESPACE
