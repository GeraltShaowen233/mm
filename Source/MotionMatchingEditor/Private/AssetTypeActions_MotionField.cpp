

#include "AssetTypeActions_MotionField.h"
#include "MotionMatchingEditor.h"

#include "MotionFieldEditor.h"

#include "MotionField.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_MotionField::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetActions_MotionField", "Motion Field");
}

FColor FAssetTypeActions_MotionField::GetTypeColor() const
{
	return FColor::Blue;
}

UClass* FAssetTypeActions_MotionField::GetSupportedClass() const
{
	return UMotionField::StaticClass();
}

void FAssetTypeActions_MotionField::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	for (UObject* CurrentObject : InObjects)
	{
		if (UMotionField* MotionField = Cast<UMotionField>(CurrentObject))
		{
			TSharedRef<FMotionFieldEditor> NewMotionFieldEditor = MakeShared<FMotionFieldEditor>();
			NewMotionFieldEditor->InitMotionFieldEditor(EToolkitMode::Standalone, EditWithinLevelEditor, MotionField);
		}
	}
}

uint32 FAssetTypeActions_MotionField::GetCategories()
{
	return EAssetTypeCategories::Animation;
}

#undef LOCTEXT_NAMESPACE