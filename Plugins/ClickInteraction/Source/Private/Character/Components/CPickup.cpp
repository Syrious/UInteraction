#define TAG_PICKUP "Pickup"
#define PICKUP_ANIMATION_TIME 0.2f
#define LIN_DAMPING 20.0f
#define ANGULAR_DAMPING 20.0f

// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#include "CPickup.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "../Private/Character/CharacterController.h"
#include "Engine.h"

// Sets default values for this component's properties
UCPickup::UCPickup()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	bLocksComponent = false;
	// ...
}


// Called when the game starts
void UCPickup::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UCPickup::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PlayerCharacter == nullptr) return;

	bool bLeftMouseHold = GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::LeftMouseButton);
	bool bLeftMouseClicked = GetWorld()->GetFirstPlayerController()->WasInputKeyJustPressed(EKeys::LeftMouseButton);
	bool bLeftMouseReleased = GetWorld()->GetFirstPlayerController()->WasInputKeyJustReleased(EKeys::LeftMouseButton);

	bool bRightMouseHold = GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::RightMouseButton);
	bool bRightMouseClicked = GetWorld()->GetFirstPlayerController()->WasInputKeyJustPressed(EKeys::RightMouseButton);
	bool bRightMouseReleased = GetWorld()->GetFirstPlayerController()->WasInputKeyJustReleased(EKeys::RightMouseButton);

	bool bForceDrop = GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::LeftControl); // Enable ForceDrop

	if (bForceDrop) GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Green, FString("Force Drop Enabled"), true);

	if (PlayerCharacter->FocusedActor != nullptr)
	{
		AStaticMeshActor* FocusedItem = PlayerCharacter->FocusedActor;

		if (FocusedItem->Tags.Contains(TAG_PICKUP) && bForceDrop == false)
		{
			if (bRightMouseClicked || bLeftMouseClicked)
			{
				PickupItem(bRightMouseClicked);
			}
		}
	}


	if (PlayerCharacter->FocusedActor == nullptr || PlayerCharacter->FocusedActor != nullptr && bForceDrop) // Drop only if we haven't focused on an interacable or force drop mode is enabled
	{
		if (bRightMouseClicked || bLeftMouseClicked)
		{
			// We clicked onto something else
			DropItem(bRightMouseClicked);
		}
	}

	AnimatePickup(DeltaTime);
	MoveItemsRelativeToHands(DeltaTime);
}

void UCPickup::PickupItem(bool bIsRightHand)
{
	// TODO Check tag if this items needs both hands
	UStaticMeshComponent* Mesh = PlayerCharacter->FocusedActor->GetStaticMeshComponent();

	if (bIsRightHand)
	{
		if (ItemInRightHand != nullptr) return; // We already carry something

		ItemRightHandDefaultLinDamping = Mesh->GetLinearDamping();
		ItemRightHandDefaultAngDamping = Mesh->GetAngularDamping();

		FPickupAnimation Anim = FPickupAnimation(PlayerCharacter->FocusedActor, PlayerCharacter->RightHandPosition, PICKUP_ANIMATION_TIME);
		AnimationList.Add(Anim);
	}
	else
	{
		if (ItemInLeftHand != nullptr) return; // We already carry something

		ItemLeftHandDefaultLinDamping = Mesh->GetLinearDamping();
		ItemLeftHandDefaultAngDamping = Mesh->GetAngularDamping();

		FPickupAnimation Anim = FPickupAnimation(PlayerCharacter->FocusedActor, PlayerCharacter->LeftHandPosition, PICKUP_ANIMATION_TIME);
		AnimationList.Add(Anim);
	}

	// Set damping to a higher value
	Mesh->SetLinearDamping(LIN_DAMPING);
	Mesh->SetAngularDamping(ANGULAR_DAMPING);
}

void UCPickup::DropItem(bool bIsRightHand)
{
	FHitResult HitResult = PlayerCharacter->GetRaycastResult();
	AActor* Actor = HitResult.GetActor();

	if (Actor != nullptr)
	{
		if (bIsRightHand)
		{
			if (ItemInRightHand == nullptr) return;

			// Reset damping values to item's default
			UStaticMeshComponent* Mesh = ItemInRightHand->GetStaticMeshComponent();
			Mesh->SetLinearDamping(ItemRightHandDefaultLinDamping);
			Mesh->SetAngularDamping(ItemRightHandDefaultAngDamping);
			// ********************************
			FVector Origin;
			FVector Extend;
			ItemInRightHand->GetActorBounds(false, Origin, Extend);

			FVector DropPosition = HitResult.Location + FVector(0, 0, Extend.Z);

			FPickupAnimation Anim = FPickupAnimation(ItemInRightHand, DropPosition, PICKUP_ANIMATION_TIME, true);
			AnimationList.Add(Anim);

			ItemInRightHand = nullptr;
		}
		else
		{
			if (ItemInLeftHand == nullptr) return;
			// Reset damping values to item's default
			UStaticMeshComponent* Mesh = ItemInLeftHand->GetStaticMeshComponent();
			Mesh->SetLinearDamping(ItemLeftHandDefaultLinDamping);
			Mesh->SetAngularDamping(ItemLeftHandDefaultAngDamping);
			// ********************************

			FVector Origin;
			FVector Extend;
			ItemInLeftHand->GetActorBounds(false, Origin, Extend);

			FVector DropPosition = HitResult.Location + FVector(0, 0, Extend.Z);

			FPickupAnimation Anim = FPickupAnimation(ItemInLeftHand, DropPosition, PICKUP_ANIMATION_TIME, true);
			AnimationList.Add(Anim);

			ItemInLeftHand = nullptr;
		}
	}
}

void UCPickup::AnimatePickup(float DeltaTime)
{
	if (PlayerCharacter->MovementComponent == nullptr) return;
	if (AnimationList.Num() == 0)  return;

	PlayerCharacter->MovementComponent->SetMovable(false);

	TArray<FPickupAnimation> AnimsToKeep;

	for (FPickupAnimation & elem : AnimationList)
	{
		elem.AnimateTick(DeltaTime);

		if (elem.IsAnimationDone() == false)
		{
			AnimsToKeep.Add(elem);
		}
		else
		{//The object is in the right position.

			if (elem.bDropItem == false)
			{
				// We didn't drop the item
				if (elem.GetHandActor() == PlayerCharacter->RightHandPosition)
				{
					ItemInRightHand = elem.GetItem();
				}
				else if (elem.GetHandActor() == PlayerCharacter->LeftHandPosition)
				{
					ItemInLeftHand = elem.GetItem();
				}
			}

			if (PlayerCharacter->MovementComponent != nullptr) PlayerCharacter->MovementComponent->SetMovable(true);
		}
	}

	AnimationList = AnimsToKeep;
}

void UCPickup::MoveItemsRelativeToHands(float DeltaTime)
{
	if (ItemInRightHand != nullptr)
	{
		FVector HandPosition = PlayerCharacter->RightHandPosition->GetActorLocation();
		ItemInRightHand->SetActorLocation(HandPosition);
		ItemInRightHand->SetActorRotation(GetOwner()->GetActorRotation());
	}

	if (ItemInLeftHand != nullptr)
	{
		FVector HandPosition = PlayerCharacter->LeftHandPosition->GetActorLocation();
		ItemInLeftHand->SetActorLocation(HandPosition);
		ItemInLeftHand->SetActorRotation(GetOwner()->GetActorRotation());
	}
}

