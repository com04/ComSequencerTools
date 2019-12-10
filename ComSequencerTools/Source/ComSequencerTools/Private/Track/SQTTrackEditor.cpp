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
#include "Sections/MovieSceneEventTriggerSection.h"

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
				FExecuteAction::CreateRaw(this, &FSQTTrackEditor::HandleAddEventTrackMenuEntryExecute,  TArray<FGuid>(), UMovieSceneEventSection::StaticClass())
			)
		);
	}
}

void FSQTTrackEditor::BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const TArray<FGuid>& ObjectBindings, const UClass* ObjectClass)
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
				FExecuteAction::CreateRaw( this, &FSQTTrackEditor::HandleAddEventTrackMenuEntryExecute, ObjectBindings, UMovieSceneEventSection::StaticClass() )
			)
		);
	}
}

void FSQTTrackEditor::BuildTrackContextMenu(FMenuBuilder& MenuBuilder, UMovieSceneTrack* Track)
{
	UMovieSceneEventTrack* EventTrack = CastChecked<UMovieSceneEventTrack>(Track);
	const TArray<UMovieSceneSection*>& Sections = EventTrack->GetAllSections();
	for (auto Section : Sections)
	{
		if (Section->IsA<UMovieSceneEventTriggerSection>())
		{
			return;
		}
	}
	UProperty* EventPositionProperty = FindField<UProperty>(Track->GetClass(), GET_MEMBER_NAME_STRING_CHECKED(UMovieSceneEventTrack, EventPosition));

	/** Specific details customization for the event track */
	class FEventTrackCustomization : public IDetailCustomization
	{
	public:
		virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override
		{
			DetailBuilder.HideCategory("Track");
			DetailBuilder.HideCategory("General");

			IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("TrackEvent");
			Category.AddProperty("EventReceivers").ShouldAutoExpand(true);
		}
	};

	auto PopulateSubMenu = [this, EventTrack](FMenuBuilder& SubMenuBuilder)
	{
		FPropertyEditorModule& PropertyEditor = FModuleManager::Get().LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		// Create a details view for the track
		FDetailsViewArgs DetailsViewArgs(false,false,false,FDetailsViewArgs::HideNameArea,true);
		DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Automatic;
		DetailsViewArgs.bShowOptions = false;
		DetailsViewArgs.ColumnWidth = 0.55f;

		TSharedRef<IDetailsView> DetailsView = PropertyEditor.CreateDetailView(DetailsViewArgs);
		
		// Register the custom type layout for the class
		FOnGetDetailCustomizationInstance CreateInstance = FOnGetDetailCustomizationInstance::CreateLambda(&MakeShared<FEventTrackCustomization>);
		DetailsView->RegisterInstancedCustomPropertyLayout(UMovieSceneEventTrack::StaticClass(), CreateInstance);

		GetSequencer()->OnInitializeDetailsPanel().Broadcast(DetailsView, GetSequencer().ToSharedRef());

		// Assign the object
		DetailsView->SetObject(EventTrack, true);

		// Add it to the menu
		TSharedRef< SWidget > DetailsViewWidget =
			SNew(SBox)
			.MaxDesiredHeight(400.0f)
			.WidthOverride(450.0f)
		[
			DetailsView
		];

		SubMenuBuilder.AddWidget(DetailsViewWidget, FText(), true, false);
	};

	MenuBuilder.AddSubMenu(LOCTEXT("Properties_MenuText", "Properties"), FText(), FNewMenuDelegate::CreateLambda(PopulateSubMenu));
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

void FSQTTrackEditor::HandleAddEventTrackMenuEntryExecute(TArray<FGuid> InObjectBindingIDs, UClass* SectionType)
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

	
	TArray<UMovieSceneEventTrack*> NewTracks;
	for (FGuid InObjectBindingID : InObjectBindingIDs)
	{
		if (InObjectBindingID.IsValid())
		{
			UMovieSceneEventTrack* NewObjectTrack = FocusedMovieScene->AddTrack<UMovieSceneEventTrack>(InObjectBindingID);
			NewTracks.Add(NewObjectTrack);
		
			if (GetSequencer().IsValid())
			{
				GetSequencer()->OnAddTrack(NewObjectTrack, InObjectBindingID);
			}
		}
	}
	
	if (!NewTracks.Num())
	{
		UMovieSceneEventTrack* NewMasterTrack = FocusedMovieScene->AddMasterTrack<UMovieSceneEventTrack>();
		NewTracks.Add(NewMasterTrack);
		if (GetSequencer().IsValid())
		{
			GetSequencer()->OnAddTrack(NewMasterTrack, FGuid());
		}
	}
	
	check(NewTracks.Num() != 0);
	
	for (UMovieSceneEventTrack* NewTrack : NewTracks)
	{
		CreateNewSection(NewTrack, 0, SectionType, false);

		NewTrack->SetDisplayName(LOCTEXT("TrackName", "Events"));

		
	}
}

void FSQTTrackEditor::CreateNewSection(UMovieSceneTrack* Track, int32 RowIndex, UClass* SectionType, bool bSelect)
{
	TSharedPtr<ISequencer> SequencerPtr = GetSequencer();
	if (SequencerPtr.IsValid())
	{
		UMovieScene* FocusedMovieScene = GetFocusedMovieScene();
		FQualifiedFrameTime CurrentTime = SequencerPtr->GetLocalTime();

		FScopedTransaction Transaction(LOCTEXT("CreateNewSectionTransactionText", "Add Section"));

		UMovieSceneSection* NewSection = NewObject<UMovieSceneSection>(Track, SectionType);
		check(NewSection);

		int32 OverlapPriority = 0;
		for (UMovieSceneSection* Section : Track->GetAllSections())
		{
			if (Section->GetRowIndex() >= RowIndex)
			{
				Section->SetRowIndex(Section->GetRowIndex() + 1);
			}
			OverlapPriority = FMath::Max(Section->GetOverlapPriority() + 1, OverlapPriority);
		}

		Track->Modify();

		if (SectionType == UMovieSceneEventTriggerSection::StaticClass())
		{
			NewSection->SetRange(TRange<FFrameNumber>::All());
		}
		else
		{

			TRange<FFrameNumber> NewSectionRange;

			if (CurrentTime.Time.FrameNumber < FocusedMovieScene->GetPlaybackRange().GetUpperBoundValue())
			{
				NewSectionRange = TRange<FFrameNumber>(CurrentTime.Time.FrameNumber, FocusedMovieScene->GetPlaybackRange().GetUpperBoundValue());
			}
			else
			{
				const float DefaultLengthInSeconds = 5.f;
				NewSectionRange = TRange<FFrameNumber>(CurrentTime.Time.FrameNumber, CurrentTime.Time.FrameNumber + (DefaultLengthInSeconds * SequencerPtr->GetFocusedTickResolution()).FloorToFrame());
			}

			NewSection->SetRange(NewSectionRange);
		}

		NewSection->SetOverlapPriority(OverlapPriority);
		NewSection->SetRowIndex(RowIndex);

		Track->AddSection(*NewSection);
		Track->UpdateEasing();

		if (bSelect)
		{
			SequencerPtr->EmptySelection();
			SequencerPtr->SelectSection(NewSection);
			SequencerPtr->ThrobSectionSelection();
		}

		SequencerPtr->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
	}
}

#undef LOCTEXT_NAMESPACE
