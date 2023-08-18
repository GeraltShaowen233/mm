// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionFieldEditor.h"

#include "EditorViewportClient.h"
#include "UObject/Package.h"
#include "Modules/ModuleManager.h"
#include "EditorStyleSet.h"
#include "SSingleObjectDetailsPanel.h"


#include "SEditorViewport.h"
#include "ScopedTransaction.h"
#include "MotionFieldEditor/MotionFieldEditorViewportClient.h"
#include "MotionFieldEditor/MotionFieldEditorCommands.h"

#include "MotionMatchingEditor.h"

#include "SCommonEditorViewportToolbarBase.h"
#include "SScrubControlPanel.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Commands/GenericCommands.h"

#include "Animation/DebugSkelMeshComponent.h"

#include "Classes/AnimPreviewInstance.h"

#include "SContextList.h"

#define LOCTEXT_NAMESPACE "MotionFieldEditor"


class SMotionFieldEditorViewport : public SEditorViewport, public ICommonEditorViewportToolbarInfoProvider
{
public:
	SLATE_BEGIN_ARGS(SMotionFieldEditorViewport)
		: _MotionFieldBeingEdited((UMotionField*)nullptr)
	{}

	SLATE_ATTRIBUTE(UMotionField*, MotionFieldBeingEdited)

		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, TWeakPtr<FMotionFieldEditor> InMotionFieldEditorPtr);

	// SEditorViewport interface
	virtual void BindCommands() override;
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	// End of SEditorViewport interface

	// ICommonEditorViewportToolbarInfoProvider interface
	virtual TSharedRef<class SEditorViewport> GetViewportWidget() override;
	virtual TSharedPtr<FExtender> GetExtenders() const override;
	virtual void OnFloatingButtonClicked() override{}
	// End of ICommonEditorViewportToolbarInfoProvider interface

	UDebugSkelMeshComponent* GetPreviewComponent() const
	{
		return EditorViewportClient->GetPreviewComponent();
	}

private:
	TAttribute<UMotionField*> MotionFieldBeingEdited;
	TWeakPtr<FMotionFieldEditor> MotionFieldEditorPtr;
	// Viewport client
	TSharedPtr<FMotionFieldEditorViewportClient> EditorViewportClient;
};

void SMotionFieldEditorViewport::Construct(const FArguments& InArgs, TWeakPtr<FMotionFieldEditor> InMotionFieldEditorPtr)
{
	MotionFieldBeingEdited = InArgs._MotionFieldBeingEdited;
	MotionFieldEditorPtr = InMotionFieldEditorPtr;
	SEditorViewport::Construct(SEditorViewport::FArguments());
}

void SMotionFieldEditorViewport::BindCommands()
{
	SEditorViewport::BindCommands();

	const FMotionFieldEditorCommands& Commands = FMotionFieldEditorCommands::Get();

	TSharedRef<FMotionFieldEditorViewportClient> EditorViewportClientRef = EditorViewportClient.ToSharedRef();
	
}

TSharedRef<FEditorViewportClient> SMotionFieldEditorViewport::MakeEditorViewportClient()
{
	EditorViewportClient = MakeShareable(new FMotionFieldEditorViewportClient(MotionFieldBeingEdited, MotionFieldEditorPtr));

	return EditorViewportClient.ToSharedRef();
}


TSharedRef<class SEditorViewport> SMotionFieldEditorViewport::GetViewportWidget()
{
	return SharedThis(this);
}

TSharedPtr<FExtender> SMotionFieldEditorViewport::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}


/**
 //////////////////////////////////////////////////////////////////////////////////////////////////////////////
 */
FMotionFieldEditor::FMotionFieldEditor()
	: MotionFieldBeingEdited(nullptr)
{
	
}
void FMotionFieldEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_MotionFieldEditor", "MotionField Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	auto SpawnTabViewport = [this](const FSpawnTabArgs& Args)->TSharedRef<SDockTab>
	{
		return SNew(SDockTab)
		.Label(LOCTEXT("ViewportTab_Title", "Viewport"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			[
				ViewportPtr.ToSharedRef()
			]
		];
	};

	auto SpawnTabDetails = [this](const FSpawnTabArgs& Args)->TSharedRef<SDockTab>
	{
		TSharedPtr<FMotionFieldEditor> MotionFieldEditorPtr = SharedThis(this);
	
		ContextList = SNew(SContextList, MotionFieldEditorPtr);

		return SNew(SDockTab)
			.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
			.Label(LOCTEXT("DetailsTab_Title", "Details"))
			[
				  SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.FillHeight(0.5f)
				[
					SNew(SBorder)
					[
						ContextList.ToSharedRef()
					]
				]
			];
	};

	InTabManager->RegisterTabSpawner(TEXT("Viewport"), FOnSpawnTab::CreateLambda(SpawnTabViewport))
		.SetDisplayName(LOCTEXT("ViewportTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));

	
	InTabManager->RegisterTabSpawner(TEXT("Details"), FOnSpawnTab::CreateLambda(SpawnTabDetails))
		.SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.ContentBrowser"));
}

void FMotionFieldEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	InTabManager->UnregisterTabSpawner(TEXT("Viewport"));
	InTabManager->UnregisterTabSpawner(TEXT("Details"));
}

void FMotionFieldEditor::InitMotionFieldEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, class UMotionField* InitMotionField)
{
	MotionFieldBeingEdited = InitMotionField;

	BindCommands();
	TSharedPtr<FMotionFieldEditor> MotionFieldEditorPtr = SharedThis(this);

	ViewportPtr = SNew(SMotionFieldEditorViewport, MotionFieldEditorPtr)
		.MotionFieldBeingEdited(this, &FMotionFieldEditor::GetMotionFieldBeingEdited);
	
	// Default layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("New_Standalone_MotionFieldEditor_Layout_v1")
	->AddArea
	(
		FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewStack()
			->SetSizeCoefficient(0.1f)
			->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
		)
		->Split
		(
			FTabManager::NewStack()
			->SetSizeCoefficient(0.6f)
			->AddTab(TEXT("Viewport"), ETabState::OpenedTab)
		)
		->Split
		(
			FTabManager::NewStack()
			->SetSizeCoefficient(0.3f)
			->AddTab(TEXT("Details"), ETabState::OpenedTab)
		)
	);

	// Initialize the asset editor and spawn nothing (dummy layout)
	InitAssetEditor(Mode, InitToolkitHost, FName("MotionFieldEditorAppId"), StandaloneDefaultLayout, /*bCreateDefaultStandaloneMenu=*/ true, /*bCreateDefaultToolbar=*/ true, InitMotionField);

	// Extend things
	ExtendToolbar();
	RegenerateMenusAndToolbars();

	
}

UAnimSequence * FMotionFieldEditor::GetSourceAnimation() const
{
	if (!GetMotionFieldBeingEdited()->SourceAnimations.IsValidIndex(CurrentExtractionContextIndex))
		return nullptr;
	UAnimSequence* SourceAnimation = GetMotionFieldBeingEdited()->GetSrcAnimAtIndex(CurrentExtractionContextIndex);
	if (SourceAnimation)
	{
		return SourceAnimation;
	}
	
	return nullptr;
}

void FMotionFieldEditor::SetExtractionAnimSequence(UAnimSequence * AnimSequence) const
{
	if (!ViewportPtr->GetPreviewComponent()->SkeletalMesh)
	{
		return;
	}
	if(AnimSequence && AnimSequence->GetSkeleton() == ViewportPtr->GetPreviewComponent()->SkeletalMesh->Skeleton)
	{
		ViewportPtr->GetPreviewComponent()->EnablePreview(true, AnimSequence);
		return;
	}
	ViewportPtr->GetPreviewComponent()->EnablePreview(true, NULL);
}


void FMotionFieldEditor::DeleteExtractionContext(const int AtIndex)
{
	if (AtIndex == CurrentExtractionContextIndex)
	{
		CurrentExtractionContextIndex = INDEX_NONE;
		SetExtractionAnimSequence(nullptr);
	}
	GetMotionFieldBeingEdited()->DeleteSrcAnim(AtIndex);
	ContextList.Get()->Rebuild();
}

void FMotionFieldEditor::SetCurrentExtractionContext(const int AtIndex)
{
	if((!GetMotionFieldBeingEdited()->IsValidSrcAnimIndex(AtIndex))||(!GetMotionFieldBeingEdited()->GetSrcAnimAtIndex(AtIndex)))
	{
		return;
	}
	if (AtIndex != CurrentExtractionContextIndex)
	{
		CurrentExtractionContextIndex = AtIndex;
		SetExtractionAnimSequence(GetMotionFieldBeingEdited()->GetSrcAnimAtIndex(AtIndex));
		return;
	}
		
	CurrentExtractionContextIndex = INDEX_NONE;
	SetExtractionAnimSequence(nullptr);
	

}


void FMotionFieldEditor::rebuildContext()
{
	this->ContextList->Rebuild();
}


void FMotionFieldEditor::BindCommands()
{
	const FMotionFieldEditorCommands& Commands = FMotionFieldEditorCommands::Get();

	const TSharedRef<FUICommandList>& UICommandList = GetToolkitCommands();
	
	UICommandList->MapAction(Commands.ProcessAll,
		FExecuteAction::CreateSP(this, &FMotionFieldEditor::ProcessAllClicked));
	UICommandList->MapAction(Commands.ClearAll,
		FExecuteAction::CreateSP(this, &FMotionFieldEditor::ClearAllClicked));
	UICommandList->MapAction(Commands.SetProperties,
		FExecuteAction::CreateSP(this, &FMotionFieldEditor::SetPropertiesClicked));
}

FName FMotionFieldEditor::GetToolkitFName() const
{
	return FName("MotionFieldEditor");
}

FText FMotionFieldEditor::GetBaseToolkitName() const
{
	return LOCTEXT("MotionFieldEditorAppLabel", "MotionField Editor");
}

FText FMotionFieldEditor::GetToolkitName() const
{
	bool bDirtyState = MotionFieldBeingEdited->GetOutermost()->IsDirty();

	FString MotionFieldName = MotionFieldBeingEdited->GetFName().ToString();

	FString DisplayLabel = bDirtyState 
						   ? FString::Printf(TEXT("%s*"), *MotionFieldName) 
						   : MotionFieldName;

	return FText::FromString(DisplayLabel);
}

FText FMotionFieldEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(MotionFieldBeingEdited);
}

FString FMotionFieldEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("MotionFieldEditor");
}

FString FMotionFieldEditor::GetDocumentationLink() const
{
	return TEXT("Engine/Paper2D/MotionFieldEditor");
}

FLinearColor FMotionFieldEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

void FMotionFieldEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(MotionFieldBeingEdited);
}

void FMotionFieldEditor::ExtendToolbar()
{
	auto FillToolbar = [](FToolBarBuilder& ToolbarBuilder) 
	{
		ToolbarBuilder.BeginSection("Command");
		{
			ToolbarBuilder.AddToolBarButton(FMotionFieldEditorCommands::Get().ProcessAll);
			ToolbarBuilder.AddToolBarButton(FMotionFieldEditorCommands::Get().ClearAll);
			ToolbarBuilder.AddToolBarButton(FMotionFieldEditorCommands::Get().SetProperties);
		}
		ToolbarBuilder.EndSection();
	};

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateLambda(FillToolbar)
	);

	AddToolbarExtender(ToolbarExtender);

	FMotionMatchingEditorModule* MotionMatchingEditorModule = &FModuleManager::LoadModuleChecked<FMotionMatchingEditorModule>("MotionMatchingEditor");
	AddToolbarExtender(MotionMatchingEditorModule->GetMotionFieldEditorToolBarExtensibilityManager()->GetAllExtenders());
}

void FMotionFieldEditor::ProcessAllClicked()
{
	if (MotionFieldBeingEdited)
	{
		MotionFieldBeingEdited->Modify();

		MotionFieldBeingEdited->RebakeAllAnim();

		MotionFieldBeingEdited->MarkPackageDirty();
	}
}

void FMotionFieldEditor::ClearAllClicked()
{
	MotionFieldBeingEdited->ClearAllMotionKeys();
}

void FMotionFieldEditor::SetPropertiesClicked()
{
	
}

#undef LOCTEXT_NAMESPACE
