// Copyright 2018-2019 com04 All Rights Reserved.


#include "SQTTrackEditor.h"

#include "ComSequencerToolsStyle.h"
#include "ComSequencerTools.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "GameFramework/Actor.h"
#include "EditorStyleSet.h"
#include "UObject/Package.h"
#include "Tracks/MovieSceneEventTrack.h"
#include "ISequencerSection.h"
#include "MovieSceneObjectBindingIDPicker.h"
#include "IDetailCustomization.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"
#include "DetailLayoutBuilder.h"
#include "MovieSceneObjectBindingIDCustomization.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "Widgets/Layout/SBox.h"
//#include "Sections/EventSection.h"
#include "Sections/MovieSceneEventSection.h"
#include "SequencerUtilities.h"
#include "MovieSceneSequenceEditor.h"

#define LOCTEXT_NAMESPACE "FSQTTrackEditor"


/* FSQTTrackEditor static functions
 *****************************************************************************/

TSharedRef<ISequencerTrackEditor> FSQTTrackEditor::CreateTrackEditor(TSharedRef<ISequencer> InSequencer)
{
	return MakeShareable(new FSQTTrackEditor(InSequencer));
}


/* FSQTTrackEditor structors
 *****************************************************************************/

FSQTTrackEditor::FSQTTrackEditor(TSharedRef<ISequencer> InSequencer)
	: FMovieSceneTrackEditor(InSequencer)
{ }


/* ISequencerTrackEditor interface
 *****************************************************************************/

void FSQTTrackEditor::BuildAddTrackMenu(FMenuBuilder& MenuBuilder)
{
	UMovieSceneSequence*       RootMovieSceneSequence = GetSequencer()->GetRootMovieSceneSequence();
	FMovieSceneSequenceEditor* SequenceEditor         = FMovieSceneSequenceEditor::Find(RootMovieSceneSequence);

	if (SequenceEditor && SequenceEditor->SupportsEvents(RootMovieSceneSequence))
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("AddEventTrack", "Event Track(String)"),
			LOCTEXT("AddEventTooltip", "Adds a new event track that can trigger events on the timeline."),
			FSlateIcon(FEditorStyle::GetStyleSetName(), "Sequencer.Tracks.Event"),
			FUIAction(
				FExecuteAction::CreateRaw(this, &FSQTTrackEditor::HandleAddEventTrackMenuEntryExecute, FGuid())
			)
		);
	}
}

void FSQTTrackEditor::BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const FGuid& ObjectBinding, const UClass* ObjectClass)
{
	UMovieSceneSequence*       RootMovieSceneSequence = GetSequencer()->GetRootMovieSceneSequence();
	FMovieSceneSequenceEditor* SequenceEditor         = FMovieSceneSequenceEditor::Find(RootMovieSceneSequence);

	if (SequenceEditor && SequenceEditor->SupportsEvents(RootMovieSceneSequence))
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("AddEventTrack_ObjectBinding", "Event"),
			LOCTEXT("AddEventTooltip_ObjectBinding", "Adds a new event track that will trigger events on this object binding."),
			FSlateIcon(),
			FUIAction( 
				FExecuteAction::CreateSP( this, &FSQTTrackEditor::HandleAddEventTrackMenuEntryExecute, ObjectBinding )
			)
		);
	}
}




bool FSQTTrackEditor::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	return (Type == UMovieSceneEventTrack::StaticClass());
}

bool  FSQTTrackEditor::SupportsSequence(UMovieSceneSequence* InSequence) const
{
	static UClass* LevelSequenceClass = FindObject<UClass>(ANY_PACKAGE, TEXT("LevelSequence"), true);
	static UClass* WidgetAnimationClass = FindObject<UClass>(ANY_PACKAGE, TEXT("WidgetAnimation"), true);
	return InSequence != nullptr &&
		((LevelSequenceClass != nullptr && InSequence->GetClass()->IsChildOf(LevelSequenceClass)) ||
		(WidgetAnimationClass != nullptr && InSequence->GetClass()->IsChildOf(WidgetAnimationClass)));
}

const FSlateBrush* FSQTTrackEditor::GetIconBrush() const
{
	return FEditorStyle::GetBrush("Sequencer.Tracks.Event");
}

/* FSQTTrackEditor callbacks
 *****************************************************************************/

void FSQTTrackEditor::HandleAddEventTrackMenuEntryExecute(FGuid InObjectBindingID)
{
	UMovieScene* FocusedMovieScene = GetFocusedMovieScene();

	if (FocusedMovieScene == nullptr)
	{
		return;
	}

	if (FocusedMovieScene->IsReadOnly())
	{
		return;
	}

	const FScopedTransaction Transaction(NSLOCTEXT("Sequencer", "AddEventTrack_Transaction", "Add Event Track"));
	FocusedMovieScene->Modify();

	UMovieSceneEventTrack* NewTrack = nullptr;
	if (InObjectBindingID.IsValid())
	{
		NewTrack = FocusedMovieScene->AddTrack<UMovieSceneEventTrack>(InObjectBindingID);
	}
	else
	{
		NewTrack = FocusedMovieScene->AddMasterTrack<UMovieSceneEventTrack>();
	}

	check(NewTrack);

//	UMovieSceneSection* NewSection = NewTrack->CreateNewSection();
	UMovieSceneSection* NewSection = NewObject<UMovieSceneEventSection>(NewTrack, NAME_None, RF_Transactional);
	check(NewSection);

	NewTrack->AddSection(*NewSection);
	NewTrack->SetDisplayName(LOCTEXT("TrackName", "Events"));

	if (GetSequencer().IsValid())
	{
		GetSequencer()->OnAddTrack(NewTrack);
	}

	GetSequencer()->NotifyMovieSceneDataChanged( EMovieSceneDataChangeType::MovieSceneStructureItemAdded );
}

#undef LOCTEXT_NAMESPACE
