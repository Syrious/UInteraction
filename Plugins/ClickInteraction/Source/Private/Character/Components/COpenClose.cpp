#define TAG_KEY_OPENCLOSABLE "OpenCloseable"


// Fill out your copyright notice in the Description page of Project Settings.
#include "COpenClose.h"

#include "../Private/Character/CharacterController.h"
#include "Components/StaticMeshComponent.h"
#include "TagStatics.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"



// Sets default values for this component's properties
UCOpenClose::UCOpenClose()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;


	ForceToApply = 1500000.0f;
	bLocksComponent = true;
	// ...
}


// Called when the game starts
void UCOpenClose::BeginPlay()
{
	Super::BeginPlay();
	// ...

	SetOfOPenCloasableItems = FTagStatics::GetActorSetWithKeyValuePair(GetWorld(), "ClickInteraction", TAG_KEY_OPENCLOSABLE, "True");

}


// Called every frame
void UCOpenClose::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (PlayerCharacter == nullptr) return;

	if (PlayerCharacter->LockedByComponent != nullptr &&  PlayerCharacter->LockedByComponent != this) return; // Another component has exclusive rights for the controlls

	bool bRightHandEmpty = true;
	bool bLeftHandEmpty = true;

	// Check if hands are empty
	if (PlayerCharacter->PickupComponent != nullptr) {
		bRightHandEmpty = PlayerCharacter->PickupComponent->ItemInRightHand == nullptr;
		bLeftHandEmpty = PlayerCharacter->PickupComponent->ItemInLeftHand == nullptr;
	}

	bool bLeftMouseHold = GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::LeftMouseButton);
	bool bLeftMouseClicked = GetWorld()->GetFirstPlayerController()->WasInputKeyJustPressed(EKeys::LeftMouseButton);
	bool bLeftMouseReleased = GetWorld()->GetFirstPlayerController()->WasInputKeyJustReleased(EKeys::LeftMouseButton);

	bool bRightMouseHold = GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::RightMouseButton);
	bool bRightMouseClicked = GetWorld()->GetFirstPlayerController()->WasInputKeyJustPressed(EKeys::RightMouseButton);
	bool bRightMouseReleased = GetWorld()->GetFirstPlayerController()->WasInputKeyJustReleased(EKeys::RightMouseButton);

	if (PlayerCharacter->FocusedActor != nullptr && SetOfOPenCloasableItems.Contains(PlayerCharacter->FocusedActor))
	{
		if (bRightMouseClicked && bRightHandEmpty || bLeftMouseClicked && bLeftHandEmpty) {
			bLastUsedHandWasRightHand = bRightMouseClicked;

			ClickedActor = PlayerCharacter->FocusedActor;
			PlayerCharacter->MovementComponent->SetMovable(false);
			SetLockedByComponent(true);
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
			ClickedActor->GetStaticMeshComponent()->SetRenderCustomDepth(false);
			PlayerCharacter->MovementComponent->SetMovable(true);
			ClickedActor = nullptr;

			SetLockedByComponent(false);
		}
	}
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
	GetWorld()->GetFirstPlayerController()->GetInputMouseDelta(MouseX, MouseY);

	//UE_LOG(LogTemp, Warning, TEXT("MouseX %f MouseY %f"), MouseX, MouseY);
	AStaticMeshActor* Actor = Cast<AStaticMeshActor>(ClickedActor);

	if (Actor == nullptr) return;

	// Get Normal vector than add force along this vector
	UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent();
	//Mesh->AddForce(NormalOfFocusedObject * -MouseY * DeltaTime * ForceToApply, NAME_None, true);
	Mesh->AddForce(Actor->GetActorForwardVector() * -MouseY * DeltaTime * ForceToApply, NAME_None, true);
}

