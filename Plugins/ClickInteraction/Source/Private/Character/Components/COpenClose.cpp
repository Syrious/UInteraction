#define TAG_KEY_OPENCLOSABLE "OpenCloseable"
#define SEMLOG_TAG "SemLog"

// Fill out your copyright notice in the Description page of Project Settings.
#include "COpenClose.h"

#include "../Private/Character/CharacterController.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TagStatics.h"
#include "SLUtils.h"
#include "DrawDebugHelpers.h"



// Sets default values for this component's properties
UCOpenClose::UCOpenClose()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;


	ForceToApply = 400000.0f;
	//bLocksComponent = true;
	// ...
}


// Called when the game starts
void UCOpenClose::BeginPlay()
{
	Super::BeginPlay();
	// ...

	SetOfOPenCloasableItems = FTagStatics::GetActorSetWithKeyValuePair(GetWorld(), "ClickInteraction", TAG_KEY_OPENCLOSABLE, "True");

	// Get the semantic log runtime manager from the world
	for (TActorIterator<ASLRuntimeManager>RMItr(GetWorld()); RMItr; ++RMItr)
	{
		SemLogRuntimeManager = *RMItr;
		break;
	}
}


// Called every frame
void UCOpenClose::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (PlayerCharacter == nullptr) return;
	if (PlayerCharacter->LockedByComponent != nullptr &&  PlayerCharacter->LockedByComponent != this) return; // Another component has exclusive rights for the controlls

	if (bRightMouseHold) {
		OnInteractionKeyHold(true, DeltaTime);
	}
	else if (bLeftMouseHold) {
		OnInteractionKeyHold(false, DeltaTime);
	}
	/*
		bool bRightHandEmpty = true;
		bool bLeftHandEmpty = true;
		bool bTwoHandMode = true;

		// Check if hands are empty
		if (PlayerCharacter->PickupComponent != nullptr) {
			bRightHandEmpty = PlayerCharacter->PickupComponent->ItemInRightHand == nullptr;
			bLeftHandEmpty = PlayerCharacter->PickupComponent->ItemInLeftHand == nullptr;
			bTwoHandMode = PlayerCharacter->PickupComponent->bTwoHandMode;
		}

		if (PlayerCharacter->FocusedActor != nullptr && SetOfOPenCloasableItems.Contains(PlayerCharacter->FocusedActor))
		{

			if (bRightMouseClicked || bLeftMouseClicked) {
				// In TwoHandMode we can interact with both hands if they are empty. In OneHandMode we need all hands to be empty
				bool bCanInteractTwoHandMode = bTwoHandMode && (bRightMouseClicked && bRightHandEmpty || bLeftMouseClicked && bLeftHandEmpty);
				bool bCanInteractOneHandMode = !bTwoHandMode && bRightHandEmpty && bLeftHandEmpty;

				if (bCanInteractTwoHandMode || bCanInteractOneHandMode) {
					// Start interacting
					bLastUsedHandWasRightHand = bRightMouseClicked;

					ClickedActor = PlayerCharacter->FocusedActor;

					PlayerCharacter->MovementComponent->SetMovable(false);
					SetLockedByComponent(true);

					StartLogEvent(ClickedActor);
				}
				else {
					GEngine->AddOnScreenDebugMessage(1, 3.0f, FColor::Red, "Hand(s) not empty. Can't interact", false);
				}
			}
		}

		if (ClickedActor != nullptr)
		{
			if (bLastUsedHandWasRightHand && bRightMouseHold || bLastUsedHandWasRightHand == false && bLeftMouseHold) {
				AddForceToObject(DeltaTime);
				ClickedActor->GetStaticMeshComponent()->SetRenderCustomDepth(true); // Keep object highlighted
			}

			if (bLeftMouseReleased || bRightMouseReleased)
			{
				// Stop interacting
				ClickedActor->GetStaticMeshComponent()->SetRenderCustomDepth(false);
				PlayerCharacter->MovementComponent->SetMovable(true);

				SetLockedByComponent(false);
				FinishLogEvent(ClickedActor);

				ClickedActor = nullptr;
			}
		}

		ResetInput();
	*/
}

void UCOpenClose::SetupKeyBindings(UInputComponent * PlayerInputComponent)
{
	PlayerInputComponent->BindAction("LeftHandAction", IE_Pressed, this, &UCOpenClose::InputLeftHandPressed);
	PlayerInputComponent->BindAction("LeftHandAction", IE_Released, this, &UCOpenClose::InputLeftHandReleased);

	PlayerInputComponent->BindAction("RightHandAction", IE_Pressed, this, &UCOpenClose::InputRightHandPressed);
	PlayerInputComponent->BindAction("RightHandAction", IE_Released, this, &UCOpenClose::InputRightHandReleased);
}

void UCOpenClose::SetLockedByComponent(bool bIsLocked)
{
	if (bIsLocked) {
		PlayerCharacter->LockedByComponent = this;
	}
	else {
		PlayerCharacter->LockedByComponent = nullptr;
	}
}

void UCOpenClose::AddForceToObject(float DeltaTime)
{
	float MouseX;
	float MouseY;
	GetWorld()
		->GetFirstPlayerController()
		->GetInputMouseDelta(MouseX, MouseY);

	AStaticMeshActor* Actor = Cast<AStaticMeshActor>(ClickedActor);

	if (Actor == nullptr) return;

	// Get forward vector than add force along this vector
	float ForceToAdd = -MouseY * DeltaTime * ForceToApply;
	UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent();
	Mesh->AddForce(Actor->GetActorForwardVector() * ForceToAdd, NAME_None, true);
}

void UCOpenClose::StartLogEvent(AActor * ActorToHandle)
{
	if (SemLogRuntimeManager == nullptr) return;
	FString HandUsed = bRightMouseHold ? "RightHand" : "LeftHand";

	// Log Pickup event
	const FString ItemClass = FTagStatics::GetKeyValue(ActorToHandle, SEMLOG_TAG, "Class");
	const FString ItemID = FTagStatics::GetKeyValue(ActorToHandle, SEMLOG_TAG, "Id");

	// Create contact event and other actor individual
	const FOwlIndividualName OtherIndividual("log", ItemClass, ItemID);
	const FOwlIndividualName ContactIndividual("log", "TouchingSituation", FSLUtils::GenerateRandomFString(4));

	// Owl prefixed names
	const FOwlPrefixName RdfType("rdf", "type");
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName RdfDatatype("rdf", "datatype");
	const FOwlPrefixName TaskContext("knowrob", "taskContext");
	const FOwlPrefixName InContact("knowrob_u", "inContact");
	const FOwlPrefixName OwlNamedIndividual("owl", "NamedIndividual");

	// Owl classes
	const FOwlClass XsdString("xsd", "string");
	const FOwlClass TouchingSituation("knowrob_u", "TouchingSituation");

	TArray <FOwlTriple> Properties;
	Properties.Add(FOwlTriple(RdfType, RdfResource, TouchingSituation));
	Properties.Add(FOwlTriple(TaskContext, RdfDatatype, XsdString, "Opening or Closing-" + HandUsed));
	Properties.Add(FOwlTriple(InContact, RdfResource, OtherIndividual));

	TSharedPtr<FOwlNode> ContactEvent = MakeShareable(new FOwlNode(OwlNamedIndividual, RdfAbout, ContactIndividual, Properties));

	// Start the event with the given properties
	if (SemLogRuntimeManager->StartEvent(ContactEvent))
	{
		// Add event to the local map of started events
		LogEvent = ContactEvent;
	}

	if (PlayerCharacter->LogComponent != nullptr) PlayerCharacter->LogComponent->StartEvent(ContactEvent);
}

void UCOpenClose::FinishLogEvent(AActor * ActorToHandle)
{
	if (LogEvent.IsValid()) {
		if (PlayerCharacter->LogComponent != nullptr) PlayerCharacter->LogComponent->FinishEvent(LogEvent);
		SemLogRuntimeManager->FinishEvent(LogEvent);
		LogEvent = nullptr;
	}
}

void UCOpenClose::InputLeftHandPressed()
{
	if (bLeftMouseHold) return; // We already clicked that key
	if (bRightMouseHold) return; // Ignore this call if the other key has been pressed

	bLeftMouseHold = true;
	OnInteractionKeyPressed(false);
}

void UCOpenClose::InputLeftHandReleased()
{
	if (bLeftMouseHold == false) return; // We can't release if we didn't clicked first
	if (bRightMouseHold) return; // Ignore this call if the other key has been pressed

	bLeftMouseHold = false;
	OnInteractionKeyReleased(false);
}

void UCOpenClose::InputRightHandPressed()
{
	if (bRightMouseHold) return; // We already clicked that key
	if (bLeftMouseHold) return; // Ignore this call if the other key has been pressed

	bRightMouseHold = true;
	OnInteractionKeyPressed(true);
}

void UCOpenClose::InputRightHandReleased()
{
	if (bRightMouseHold == false) return;  // We can't release if we didn't clicked first
	if (bLeftMouseHold) return; // Ignore this call if the other key has been pressed

	bRightMouseHold = false;
	OnInteractionKeyReleased(true);
}

void UCOpenClose::OnInteractionKeyPressed(bool bIsRightKey)
{
	if (PlayerCharacter->FocusedActor != nullptr && SetOfOPenCloasableItems.Contains(PlayerCharacter->FocusedActor))
	{
		bool bRightHandEmpty = true;
		bool bLeftHandEmpty = true;
		bool bTwoHandMode = true;

		// Check if hands are empty
		if (PlayerCharacter->PickupComponent != nullptr) {
			bRightHandEmpty = PlayerCharacter->PickupComponent->ItemInRightHand == nullptr;
			bLeftHandEmpty = PlayerCharacter->PickupComponent->ItemInLeftHand == nullptr;
			bTwoHandMode = PlayerCharacter->PickupComponent->bTwoHandMode;
		}

		// In TwoHandMode we can interact with both hands if they are empty. In OneHandMode we need all hands to be empty
		bool bCanInteractTwoHandMode = bTwoHandMode && (bIsRightKey && bRightHandEmpty || bIsRightKey == false && bLeftHandEmpty);
		bool bCanInteractOneHandMode = !bTwoHandMode && bRightHandEmpty && bLeftHandEmpty;

		if (bCanInteractTwoHandMode || bCanInteractOneHandMode) {
			// Start interacting
			// bLastUsedHandWasRightHand = bRightMouseClicked;

			ClickedActor = PlayerCharacter->FocusedActor;

			PlayerCharacter->MovementComponent->SetMovable(false);
			SetLockedByComponent(true);

			StartLogEvent(ClickedActor);
		}
		else {
			GEngine->AddOnScreenDebugMessage(1, 3.0f, FColor::Red, "Hand(s) not empty. Can't interact", false);
		}

	}
}

void UCOpenClose::OnInteractionKeyHold(bool bIsRightKey, float DeltaTime)
{
	if (ClickedActor != nullptr)
	{
		AddForceToObject(DeltaTime);
		ClickedActor->GetStaticMeshComponent()->SetRenderCustomDepth(true); // Keep object highlighted
	}
}

void UCOpenClose::OnInteractionKeyReleased(bool bIsRightKey)
{
	if (ClickedActor != nullptr)
	{
		// Stop interacting
		ClickedActor->GetStaticMeshComponent()->SetRenderCustomDepth(false);
		PlayerCharacter->MovementComponent->SetMovable(true);

		SetLockedByComponent(false);
		FinishLogEvent(ClickedActor);

		ClickedActor = nullptr;
	}
}

//void UCOpenClose::ResetInput()
//{
//	bLeftMouseClicked = false;
//	bLeftMouseReleased = false;
//
//	bRightMouseClicked = false;
//	bRightMouseReleased = false;
//}

