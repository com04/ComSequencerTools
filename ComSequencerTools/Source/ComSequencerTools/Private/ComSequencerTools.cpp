// Copyright 2018-2019 com04, Inc. All Rights Reserved.

#include "ComSequencerTools.h"
#include "ComSequencerToolsStyle.h"

#include "ISequencerModule.h"
#include "Track/SQTTrackEditor.h"

#define LOCTEXT_NAMESPACE "FComSequencerToolsModule"


void FComSequencerToolsModule::StartupModule()
{
	FComSequencerToolsStyle::Initialize();

	ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
	TrackEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FSQTTrackEditor::CreateTrackEditor));
}

void FComSequencerToolsModule::ShutdownModule()
{
	ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
	SequencerModule.UnRegisterTrackEditor(TrackEditorHandle);

	FComSequencerToolsStyle::Shutdown();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FComSequencerToolsModule, ComSequencerTools)
