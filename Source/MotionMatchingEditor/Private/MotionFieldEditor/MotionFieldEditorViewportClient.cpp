// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionFieldEditorViewportClient.h"



#include "CanvasItem.h"
#include "Engine/Engine.h"
#include "EngineGlobals.h"
#include "Engine/CollisionProfile.h"
#include "Utils.h"

#include "Engine.h"

#include "CanvasTypes.h"

#include "AssetEditorModeManager.h"


#include "Classes/AnimPreviewInstance.h"

#include "Animation/DebugSkelMeshComponent.h"
#include "Components/PoseableMeshComponent.h"

#include "Components/PostProcessComponent.h"
#include "Engine/Texture.h"
#include "Engine/TextureCube.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"


#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereReflectionCaptureComponent.h"

#include "Editor/AdvancedPreviewScene/Public/AssetViewerSettings.h"

#include "Engine/StaticMesh.h"

#include "MotionFieldEditor.h"

#include "MotionKeyUtils.h"


#define LOCTEXT_NAMESPACE "MotionFieldEditor"

void FMotionFieldEditorViewportClient::RequestFocusOnSelection(bool bInstant)
{
	FocusViewportOnBox(GetDesiredFocusBounds(), true);
}


FMotionFieldEditorViewportClient::FMotionFieldEditorViewportClient(const TAttribute<UMotionField*>& InMotionFieldBeingEdited, TWeakPtr<FMotionFieldEditor> InMotionFieldEditorPtr)
	:MotionFieldEditorPtr(InMotionFieldEditorPtr),
		FEditorViewportClient(nullptr)
{
	SetInitialViewTransform(LVT_Perspective, FVector(-200, 200, 150), FRotator(0, -60.f, 0), 0.0f);
	MotionFieldBeingEdited = InMotionFieldBeingEdited;
	PreviewScene = &OwnedPreviewScene;
	SetRealtime(true);

	// Create a render component for the sprite being edited
	AnimatedObject = NewObject<UDebugSkelMeshComponent>();

	USkeletalMesh* SkelMesh = MotionFieldBeingEdited.Get()->GetMotionFieldSkeleton()->GetPreviewMesh();

	if(SkelMesh)
	{
		AnimatedObject->SetSkeletalMesh(SkelMesh);
	}
	
	AnimatedObject->UpdateBounds();

	PreviewScene->AddComponent(AnimatedObject.Get(), FTransform::Identity);

	// Add a directional light
	DirectionalLight = NewObject<UDirectionalLightComponent>();
	DirectionalLight->Intensity = 3.0f;
	PreviewScene->AddComponent(DirectionalLight, FTransform(FRotator(-45, 45, 0)));

	// Add a sky light
	SkyLightComponent = NewObject<USkyLightComponent>();
	SkyLightComponent->Intensity = 1.0f;
	PreviewScene->AddComponent(SkyLightComponent, FTransform());

	// Add a floor
	UStaticMesh* FloorMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/EditorMeshes/AssetViewer/Floor_Mesh.Floor_Mesh"));
	FloorMeshComponent = NewObject<UStaticMeshComponent>();
	FloorMeshComponent->SetStaticMesh(FloorMesh);
	FloorMeshComponent->SetRelativeScale3D(FVector(300.0f, 300.0f, 2.0f));
	FloorMeshComponent->SetVisibleFlag(true);
	PreviewScene->AddComponent(FloorMeshComponent, FTransform(FRotator(0, 0, 0), FVector(0, 0, 0)));
}


void FMotionFieldEditorViewportClient::DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas)
{
	FEditorViewportClient::DrawCanvas(InViewport, View, Canvas);

}


void FMotionFieldEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FEditorViewportClient::Draw(View, PDI);
	FUnrealEdUtils::DrawWidget(View, PDI, AnimatedObject->GetComponentTransform().ToMatrixWithScale(), 0, 0, EAxisList::Screen, EWidgetMovementMode::WMM_Translate);

}

FBox FMotionFieldEditorViewportClient::GetDesiredFocusBounds() const
{

	return AnimatedObject->Bounds.GetBox();
	
}

void FMotionFieldEditorViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);
	
	if (AnimatedObject.IsValid())
	{
		
			AnimatedObject->UpdateBounds();

			FTransform ComponentTransform; 
			auto CurrentAnim = MotionFieldEditorPtr.Pin().Get()->GetSourceAnimation();
			if (CurrentAnim)
			{
				if (CurrentAnim->bEnableRootMotion)
				{
					CurrentAnim->GetBoneTransform(ComponentTransform, 0, AnimatedObject->GetPosition(), false);
				}
				else
					ComponentTransform = FTransform::Identity;
			}
			AnimatedObject->SetWorldTransform(ComponentTransform, false);
			OwnedPreviewScene.GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
		
	}
}

bool FMotionFieldEditorViewportClient::InputKey(FViewport* InViewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed, bool bGamepad)
{
	return FEditorViewportClient::InputKey(InViewport, ControllerId, Key, Event, AmountDepressed, bGamepad);
}

FLinearColor FMotionFieldEditorViewportClient::GetBackgroundColor() const
{
	return FColor(55, 55, 55);
}

#undef LOCTEXT_NAMESPACE
