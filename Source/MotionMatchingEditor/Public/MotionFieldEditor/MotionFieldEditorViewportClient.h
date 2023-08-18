// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "InputCoreTypes.h"
#include "PreviewScene.h"
#include "MotionField.h"

#include "EditorViewportClient.h"
#include "SEditorViewport.h"

class FCanvas;
class UDebugSkelMeshComponent;
class UPoseableMeshComponent;

class UAssetViewerSettings;
class UMaterialInstanceConstant;
class UPostProcessComponent;
class USkyLightComponent;
class UStaticMeshComponent;
class USphereReflectionCaptureComponent;
struct FPreviewSceneProfile;
struct FTrajectoryData;

struct FMotionKey;


class FMotionFieldEditorViewportClient : public FEditorViewportClient
{
public:
	/** Constructor */
	FMotionFieldEditorViewportClient(const TAttribute<class UMotionField*>& InMotionFieldBeingEdited, TWeakPtr<class FMotionFieldEditor> InMotionFieldEditorPtr);

	// FViewportClient interface
	virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas) override;
	virtual void Tick(float DeltaSeconds) override;

	// FEditorViewportClient interface
	virtual bool InputKey(FViewport* Viewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed, bool bGamepad) override;
	virtual FLinearColor GetBackgroundColor() const override;
	

	UDebugSkelMeshComponent* GetPreviewComponent() const
	{
		return AnimatedObject.Get();
	}
	virtual void RequestFocusOnSelection(bool bInstant);


protected:
	// FPaperEditorViewportClient interface
	virtual FBox GetDesiredFocusBounds() const;
	// End of FPaperEditorViewportClient interface
	UDirectionalLightComponent* DirectionalLight;
	USkyLightComponent* SkyLightComponent;
	UStaticMeshComponent* SkyComponent;
	USphereReflectionCaptureComponent* SphereReflectionComponent;
	UMaterialInstanceConstant* InstancedSkyMaterial;
	UPostProcessComponent* PostProcessComponent;
	UStaticMeshComponent* FloorMeshComponent;
	UAssetViewerSettings* DefaultSettings;
private:

	TWeakPtr<class FMotionFieldEditor> MotionFieldEditorPtr;
	
	// The preview scene
	FPreviewScene OwnedPreviewScene;

	// The MotionField being displayed in this client
	TAttribute<class UMotionField*> MotionFieldBeingEdited;

	// Render component for the sprite being edited
	TWeakObjectPtr<UDebugSkelMeshComponent> AnimatedObject;
	
	
};
