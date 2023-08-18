// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

class FMotionFieldEditorCommands : public TCommands<FMotionFieldEditorCommands>
{
public:
	FMotionFieldEditorCommands();
	
	virtual void RegisterCommands() final;
	
	TSharedPtr<FUICommandInfo> ProcessAll;

	TSharedPtr<FUICommandInfo> ClearAll;

	TSharedPtr<FUICommandInfo> SetProperties;
};
