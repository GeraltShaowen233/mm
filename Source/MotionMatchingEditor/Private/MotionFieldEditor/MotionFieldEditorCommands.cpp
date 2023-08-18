// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionFieldEditorCommands.h"

//////////////////////////////////////////////////////////////////////////
// FMotionFieldEditorCommands

#define LOCTEXT_NAMESPACE "MotionFieldEditorCommands"

FMotionFieldEditorCommands::FMotionFieldEditorCommands(): TCommands<FMotionFieldEditorCommands>(
			TEXT("MotionFieldEditor"), 
			NSLOCTEXT("MotionMatchingDataBase", "MotionFieldEditor", "MotionField Editor"),
			NAME_None, 
			FName("Motion Field")
			)
{
	
}

void FMotionFieldEditorCommands::RegisterCommands()
{
	UI_COMMAND(ProcessAll, "ProcessAll", "Processes all Motion Keys in all Animations", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ClearAll, "ClearAll", "Eliminates all Motion Keys", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(SetProperties, "SetProperties", "WARNING This will reset Tag Time Ranges", EUserInterfaceActionType::Button, FInputChord());
	
	
}

#undef LOCTEXT_NAMESPACE
