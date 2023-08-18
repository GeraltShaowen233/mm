// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "UObject/GCObject.h"
#include "Framework/Docking/TabManager.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "MotionField.h"
#include "SContextList.h"
#include "Editor/EditorWidgets/Public/ITransportControl.h"

class SMotionFieldEditorViewport;
class UDebugSkelMeshComponent;

class USkeletalMesh;
class UAnimSequence;
class SContextList;


class FMotionFieldEditor : public FAssetEditorToolkit, public FGCObject
{
public:
	FMotionFieldEditor();

	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;

	// FAssetEditorToolkit
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FString GetDocumentationLink() const override;

	// FSerializableObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

public:
	void InitMotionFieldEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UMotionField* InitMotionField);

	UMotionField* GetMotionFieldBeingEdited() const { return MotionFieldBeingEdited; }
	UAnimSequence* GetSourceAnimation() const;
	void SetExtractionAnimSequence(UAnimSequence* AnimSequence) const;
	
	
	void DeleteExtractionContext(const int AtIndex);
	void SetCurrentExtractionContext(const int AtIndex);

	
	int CurrentExtractionContextIndex;

	void ProcessAllClicked();
	void ClearAllClicked();
	void SetPropertiesClicked();
	
	void rebuildContext();


protected:
	UMotionField* MotionFieldBeingEdited;


	TSharedPtr <SMotionFieldEditorViewport> ViewportPtr;
	TSharedPtr <SContextList> ContextList;
	TSharedPtr <SVerticalBox> TagTimelineBoxPtr;


	

protected:

	bool PendingTimelineRebuild = false;

	float GetFramesPerSecond() const
	{
		return 30.f;
	}

protected:
	void BindCommands();
	void ExtendToolbar();
	
};
