#define TAG_KEY_PICKUP "Pickup"
#define PICKUP_ANIMATION_TIME 0.2f
#define LIN_DAMPING 20.0f
#define ANGULAR_DAMPING 20.0f
#define RAYCAST_RANGE 200.0f
#define PICKUP_SHADOW_SCALE_FACTOR 0.2f // The scale factor when testing collisions on pickup

// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#include "CPickup.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "../Private/Character/CharacterController.h"
#include "TagStatics.h"
#include "Engine.h"

// Sets default values for this component's properties
UCPickup::UCPickup()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	bLocksComponent = false;
	bDoStackCheck = true;

	bUseBothHands = true;
	bBothHandsDependOnMass = true;
	MassThresholdTouseBothHands = 5.0f;

	bBothHandsDependOnVolume = true;
	VolumeThresholdTouseBothHands = 3000.0f;

	bCheckForCollisionsOnPickup = true;
	// ...
}


// Called when the game starts
void UCPickup::BeginPlay()
{
	Super::BeginPlay();

	// ...

	SetOfPickupItems = FTagStatics::GetActorSetWithKeyValuePair(GetWorld(), "ClickInteraction", TAG_KEY_PICKUP, "True");

	//UInputComponent* PlayerInputComponent = 

	//PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &UCPlaceItem::OnFirePressed);
	if (PlayerCharacter != nullptr) {
		UInputComponent* PlayerInputComponent = PlayerCharacter->InputComponent;

		PlayerInputComponent->BindAction("CancelAction", IE_Pressed, this, &UCPickup::CancelActions);
		PlayerInputComponent->BindAction("CancelAction", IE_Released, this, &UCPickup::StopCancelActions);
		PlayerInputComponent->BindAction("ResetRotation", IE_Pressed, this, &UCPickup::ResetRotationOfItemsInHand);

		// Create Static mesh actors for hands to weld items we pickup into this position
		LeftHandActor = GetWorld()->SpawnActor<AStaticMeshActor>();
		RightHandActor = GetWorld()->SpawnActor<AStaticMeshActor>();
		BothHandActor = GetWorld()->SpawnActor<AStaticMeshActor>();

		LeftHandActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
		RightHandActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
		BothHandActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);

		LeftHandActor->SetActorLocation(PlayerCharacter->LeftHandPosition->GetActorLocation());
		RightHandActor->SetActorLocation(PlayerCharacter->RightHandPosition->GetActorLocation());
		BothHandActor->SetActorLocation(PlayerCharacter->BothHandPosition->GetActorLocation());

		LeftHandActor->AttachToActor(PlayerCharacter, FAttachmentTransformRules::KeepWorldTransform);
		RightHandActor->AttachToActor(PlayerCharacter, FAttachmentTransformRules::KeepWorldTransform);
		BothHandActor->AttachToActor(PlayerCharacter, FAttachmentTransformRules::KeepWorldTransform);

		LeftHandActor->GetStaticMeshComponent()->SetSimulatePhysics(true);
		RightHandActor->GetStaticMeshComponent()->SetSimulatePhysics(true);
		BothHandActor->GetStaticMeshComponent()->SetSimulatePhysics(true);
	}

	// Bind Delegate to StackChecker
	if (StackChecker != nullptr) {
		StackChecker->OnStackCheckDone.AddDynamic(this, &UCPickup::OnStackCheckIsDone);
		UE_LOG(LogTemp, Warning, TEXT("Bound OnStackCheckIsDone delegate"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Stack checker assigned"));
	}
}


// Called every frame
void UCPickup::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PlayerCharacter == nullptr) return;
	if (bIsStackChecking == true) return; // Bail out if a stack check is running

	bool bLockedByOtherComponent = PlayerCharacter->LockedByComponent != nullptr &&  PlayerCharacter->LockedByComponent != this;

	bool bLeftMouseHold = GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::LeftMouseButton);
	bool bLeftMouseClicked = GetWorld()->GetFirstPlayerController()->WasInputKeyJustPressed(EKeys::LeftMouseButton);
	bool bLeftMouseReleased = GetWorld()->GetFirstPlayerController()->WasInputKeyJustReleased(EKeys::LeftMouseButton);

	bool bRightMouseHold = GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::RightMouseButton);
	bool bRightMouseClicked = GetWorld()->GetFirstPlayerController()->WasInputKeyJustPressed(EKeys::RightMouseButton);
	bool bRightMouseReleased = GetWorld()->GetFirstPlayerController()->WasInputKeyJustReleased(EKeys::RightMouseButton);

	bool bForceDrop = GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::LeftControl); // Enable ForceDrop
	bool bDragging = GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::LeftShift); // Enable Draggin

	if (bForceDrop) GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Green, FString("Force Drop Enabled"), true);

	if (bLockedByOtherComponent == false) {

		if ((bLeftMouseHold || bRightMouseHold) && bIsDragging)
		{
			DragItem(bRightMouseHold);
		}

		ItemToHandle = PlayerCharacter->FocusedActor;

		if (ItemToHandle != nullptr)
		{
			if (SetOfPickupItems.Contains(ItemToHandle) && bForceDrop == false)
			{
				if (bRightMouseClicked || bLeftMouseClicked)
				{
					if (bDragging)
					{
						StartDrag(bRightMouseClicked);
					}
					else {
						if (bRightMouseClicked) {
							if (ItemInRightHand == nullptr) {
								UsedHand = EHand::Right;
								StartPickup();
							}
							else {
								ShadowDropItem(bRightMouseClicked);
							}
						}
						else {
							if (ItemInLeftHand == nullptr) {
								UsedHand = EHand::Left;
								StartPickup();
							}
							else {
								ShadowDropItem(bRightMouseClicked);
							}
						}
					}
				}

				if (bRightMouseHold || bLeftMouseHold)
				{
					if (bIsDragging == false && bAllCanceled == false)	ShadowPickupItem(bRightMouseHold);
				}

				if (bRightMouseReleased || bLeftMouseReleased)
				{
					if (bIsDragging == false && bAllCanceled == false) {
						if (bDoStackCheck) {
							StartStackCheck();
						}
						else {
							bRightHandPickup = bRightMouseReleased;
							PickupItem(bRightMouseReleased);
						}
					}
				}
			}
		}
	}

	if (ItemToHandle == nullptr || ItemToHandle != nullptr && bForceDrop) // Drop only if we haven't focused on an interacable or force drop mode is enabled
	{
		if (bRightMouseHold || bLeftMouseHold)
		{
			if (bIsDragging == false && bAllCanceled == false) ShadowDropItem(bRightMouseHold);
		}

		if (bRightMouseReleased || bLeftMouseReleased) {
			// We clicked onto something else
			if (bIsDragging == false && bAllCanceled == false) DropItem(bRightMouseReleased);
		}
	}


	if (bLeftMouseReleased || bRightMouseReleased || bAllCanceled)
	{
		EndDrag();
		DisableShadowItems();
		SetLockedByComponent(false);
		CancelDetachItems();
	}

	AnimatePickup(DeltaTime);
	// MoveItemsRelativeToHands(DeltaTime);
}

void UCPickup::StartPickup()
{
	PlayerCharacter->bRaytraceEnabled = false;
	BaseItemToPick = GetItemStack(ItemToHandle);

	if (bUseBothHands && CalculateIfBothHandsNeeded()) UsedHand = EHand::Both;

	BaseItemToPick->GetStaticMeshComponent()->SetSimulatePhysics(false);

}

void UCPickup::PickupItem(bool bIsRightHand)
{
	if (BaseItemToPick == nullptr) return;

	DisableShadowItems();

	FAttachmentTransformRules TransformRules = FAttachmentTransformRules::KeepWorldTransform;
	TransformRules.bWeldSimulatedBodies = true;

	if (UsedHand == EHand::Right) {
		if (ItemInRightHand != nullptr) return; // We already carry something 
		BaseItemToPick->AttachToActor(RightHandActor, TransformRules);
		ItemInRightHand = BaseItemToPick;
	}
	else if (UsedHand == EHand::Left) {
		if (ItemInLeftHand != nullptr) return; // We already carry something 
		BaseItemToPick->AttachToActor(LeftHandActor, TransformRules);
		ItemInLeftHand = BaseItemToPick;
	}
	else if (UsedHand == EHand::Both) {
		if (ItemInRightHand != nullptr || ItemInLeftHand != nullptr) return;
		BaseItemToPick->AttachToActor(BothHandActor, TransformRules);
		ItemInRightHand = ItemInLeftHand = BaseItemToPick;
	}

	BaseItemToPick->SetActorRelativeLocation(FVector::ZeroVector, false, nullptr, ETeleportType::TeleportPhysics);
	BaseItemToPick->SetActorRelativeRotation(FRotator::ZeroRotator);

	BaseItemToPick = nullptr;

	PlayerCharacter->bRaytraceEnabled = true;
}

void UCPickup::ShadowPickupItem(bool bIsRightHand)
{
	if (ItemToHandle == nullptr) return;
	if (BaseItemToPick == nullptr) return;

	ItemsAndPositionToPickup.Empty();
	DisableShadowItems();

	FVector HandPosition;// = bIsRightHand ? PlayerCharacter->RightHandPosition->GetActorLocation() : PlayerCharacter->LeftHandPosition->GetActorLocation();

	if (UsedHand == EHand::Right) {
		HandPosition = RightHandActor->GetActorLocation();
	}
	else if (UsedHand == EHand::Left) {
		HandPosition = LeftHandActor->GetActorLocation();
	}
	else {
		HandPosition = BothHandActor->GetActorLocation();
	}

	TArray<AActor*> ChildItemsOfBaseItem;
	BaseItemToPick->GetAttachedActors(ChildItemsOfBaseItem);

	AStaticMeshActor* ShadowRoot = GetNewShadowItem(BaseItemToPick);
	ShadowItems.Add(ShadowRoot);

	for (auto& ItemToShadow : ChildItemsOfBaseItem) {
		AStaticMeshActor* CastActor = Cast<AStaticMeshActor>(ItemToShadow);

		if (CastActor == nullptr) return;

		AStaticMeshActor* ShadowItem = GetNewShadowItem(CastActor);
		ShadowItems.Add(ShadowItem);

		ShadowItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		FAttachmentTransformRules TransformRules = FAttachmentTransformRules::KeepWorldTransform;
		TransformRules.bWeldSimulatedBodies = true;

		ShadowItem->AttachToActor(ShadowRoot, TransformRules);
	}

	ShadowRoot->SetActorLocation(HandPosition);

	if (bCheckForCollisionsOnPickup) {
		// Check for collisions in between
		FVector CamLoc;
		FRotator CamRot;
		PlayerCharacter->Controller->GetPlayerViewPoint(CamLoc, CamRot); // Get the camera position and rotation

		FHitResult RaycastHit = RaytraceWithIgnoredActors(ChildItemsOfBaseItem);

		if (RaycastHit.Location != FVector::ZeroVector) {
			ShadowRoot->SetActorScale3D(FVector(PICKUP_SHADOW_SCALE_FACTOR));
			FVector FromPosition = RaycastHit.Location;
			FVector ToPosition = CamLoc;

			// *** Ignored Actors
			TArray<AActor*> IgnoredActors = ChildItemsOfBaseItem; // ignore items to pickup 
			IgnoredActors.Add(BaseItemToPick);
			IgnoredActors.Append(ShadowItems); //All shadow items are ignored
			IgnoredActors.Add(ItemInLeftHand);
			IgnoredActors.Add(ItemInRightHand);

			// Add all children in hands to ignored actors
			if (ItemInLeftHand != nullptr) {
				TArray<AActor*> ChildrenInLeftHand;
				ItemInLeftHand->GetAttachedActors(ChildrenInLeftHand);

				IgnoredActors.Append(ChildrenInLeftHand);
			}
			if (ItemInRightHand != nullptr) {
				TArray<AActor*> ChildrenInRightHand;
				ItemInRightHand->GetAttachedActors(ChildrenInRightHand);

				IgnoredActors.Append(ChildrenInRightHand);
			}
			// *****************

			FHitResult SweepHit = CheckForCollision(FromPosition, ToPosition, ShadowRoot, IgnoredActors);

			ShadowRoot->SetActorScale3D(FVector(1.0f));

			if (SweepHit.GetActor() != nullptr) {
				ShadowRoot->SetActorLocation(SweepHit.Location);
			}
			else {
				ShadowRoot->SetActorLocation(HandPosition);
			}
		}
	}
	else {
		ShadowRoot->SetActorLocation(HandPosition);
	}

	ShadowBaseItem = ShadowRoot;
}

TMap<AStaticMeshActor*, FVector> UCPickup::GetStackOfItems(AStaticMeshActor* ActorToPickup)
{
	TMap <AStaticMeshActor*, FVector> StackedItemsAndRelativePosition;
	int countNewItemsFound = 0;
	do
	{
		countNewItemsFound = 0;
		TArray<AActor*> IgnoredActors;

		for (auto &elem : StackedItemsAndRelativePosition) {
			IgnoredActors.Add(elem.Key);
		}

		//FVector SecondPointOfSweep = ActorToPickup->GetActorLocation() + ActorToPickup->GetActorUpVector() * 100; // TODO hardcoded range 
		FVector SecondPointOfSweep = ActorToPickup->GetActorLocation() + FVector::UpVector * 100; // TODO hardcoded range 
		TArray<FHitResult> SweptActors;
		FComponentQueryParams Params;
		Params.AddIgnoredComponent_LikelyDuplicatedRoot(ActorToPickup->GetStaticMeshComponent());
		Params.AddIgnoredActors(IgnoredActors);
		Params.bTraceComplex = true;
		Params.bFindInitialOverlaps = true;

		ActorToPickup->GetWorld()->ComponentSweepMulti(
			SweptActors,
			ActorToPickup->GetStaticMeshComponent(),
			ActorToPickup->GetActorLocation(),
			SecondPointOfSweep,
			ActorToPickup->GetActorRotation(),
			Params);

		for (auto& elem : SweptActors) {
			if (elem.GetActor() != nullptr) {
				if (SetOfPickupItems.Contains(elem.GetActor())) {
					AStaticMeshActor* CastedActor = Cast<AStaticMeshActor>(elem.GetActor());

					if (StackedItemsAndRelativePosition.Contains(CastedActor)) continue;

					UE_LOG(LogTemp, Warning, TEXT("Found item while sweeping %s"), *elem.GetActor()->GetName());
					FVector elemLocation = elem.GetActor()->GetActorLocation();
					FVector ActorToPickupLocation = ActorToPickup->GetActorLocation();

					if (elemLocation.Z < ActorToPickupLocation.Z) {
						UE_LOG(LogTemp, Warning, TEXT("Ignoring lower item %s"), *elem.GetActor()->GetName());
						continue; // ignore if the found item is lower than base item 
					}

					FVector DeltaPosition = elemLocation - ActorToPickupLocation;


					if (CastedActor != nullptr) {
						StackedItemsAndRelativePosition.Add(CastedActor, DeltaPosition);
						countNewItemsFound++;
					}


					// TODO Check if this item is on antoher shelf above
				}
			}
		}
	} while (countNewItemsFound > 0);

	return StackedItemsAndRelativePosition;
}

AStaticMeshActor * UCPickup::GetItemStack(AStaticMeshActor * BaseItem)
{
	TMap<AStaticMeshActor*, FVector> MapOfStackedItems = GetStackOfItems(BaseItem);

	MapOfStackedItems.Remove(BaseItem);

	for (auto& ChildItem : MapOfStackedItems) {
		FAttachmentTransformRules TransformRules = FAttachmentTransformRules::KeepWorldTransform;
		TransformRules.bWeldSimulatedBodies = true;

		ChildItem.Key->AttachToActor(BaseItem, TransformRules);
	}

	return BaseItem;
}

AStaticMeshActor * UCPickup::GetNewShadowItem(AStaticMeshActor * FromActor)
{
	FActorSpawnParameters Parameters;
	// Parameters.Template = FromActor;
	// Parameters.Name = FName(*FromActor->GetName().Append("_Shadow"));
	AStaticMeshActor* NewShadowPickupItem = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Parameters);

	NewShadowPickupItem->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
	NewShadowPickupItem->GetStaticMeshComponent()->SetSimulatePhysics(false);
	NewShadowPickupItem->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	NewShadowPickupItem->GetStaticMeshComponent()->SetStaticMesh(FromActor->GetStaticMeshComponent()->GetStaticMesh());
	NewShadowPickupItem->SetActorLocationAndRotation(FromActor->GetActorLocation(), FromActor->GetActorRotation());

	for (size_t i = 0; i < NewShadowPickupItem->GetStaticMeshComponent()->GetMaterials().Num(); i++)
	{
		NewShadowPickupItem->GetStaticMeshComponent()->SetMaterial(i, TransparentMaterial);
	}

	return NewShadowPickupItem;
}

FHitResult UCPickup::CheckForCollision(FVector From, FVector To, AStaticMeshActor * ItemToSweep, TArray<AActor*> IgnoredActors)
{
	TArray<FHitResult> Hits;
	FComponentQueryParams Params;

	Params.AddIgnoredActors(IgnoredActors);
	Params.AddIgnoredActor(GetOwner()); // Always ignore player

	GetWorld()->ComponentSweepMulti(Hits, ItemToSweep->GetStaticMeshComponent(), From, To, ItemToSweep->GetActorRotation(), Params);



	if (Hits.Num() > 0) {
		//for (auto& Hit : Hits) {
		//	if (Hit.GetActor() != nullptr) UE_LOG(LogTemp, Warning, TEXT("Hit %s"), *Hit.GetActor()->GetName());
		//}

		return Hits[0];
	}

	return FHitResult();
}

void UCPickup::DisableShadowItems()
{
	for (auto& shdwItm : ShadowItems) {
		shdwItm->Destroy();
	}

	ShadowItems.Empty();

	if (ShadowBaseItem != nullptr)
	{
		ShadowBaseItem->Destroy();
		ShadowBaseItem = nullptr;
	}
}

FHitResult UCPickup::RaytraceWithIgnoredActors(TArray<AActor*> IgnoredActors, FVector StartOffset, FVector TargetOffset)
{
	FHitResult RaycastResult;

	FVector CamLoc;
	FRotator CamRot;

	PlayerCharacter->Controller->GetPlayerViewPoint(CamLoc, CamRot); // Get the camera position and rotation
	const FVector StartTrace = CamLoc + StartOffset; // trace start is the camera location
	const FVector Direction = CamRot.Vector();
	const FVector EndTrace = StartTrace + Direction * RAYCAST_RANGE + TargetOffset; // and trace end is the camera location + an offset in the direction

	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActors(IgnoredActors);


	TArray<AActor*> ShadowActors;
	for (auto& shadow : ShadowItems) {
		ShadowActors.Add(shadow);
	}
	TraceParams.AddIgnoredActors(ShadowActors); // Always ignore shadow item

	if (ShadowBaseItem != nullptr) TraceParams.AddIgnoredActor(ShadowBaseItem); // Always ignore shadow item
	TraceParams.AddIgnoredActor(GetOwner()); // Always ignore player

	GetWorld()->LineTraceSingleByChannel(RaycastResult, StartTrace, EndTrace, ECollisionChannel::ECC_Visibility, TraceParams);
	return RaycastResult;
}

void UCPickup::ResetRotationOfItemsInHand()
{
	if (ItemInRightHand != nullptr) ItemInRightHand->SetActorRotation(FRotator::ZeroRotator);
	if (ItemInLeftHand != nullptr) ItemInLeftHand->SetActorRotation(FRotator::ZeroRotator);
}

void UCPickup::StartStackCheck()
{
	if (BaseItemToPick != nullptr && StackChecker != nullptr) {
		TArray<AActor*> Children;
		BaseItemToPick->GetAttachedActors(Children);
		if (Children.Num() == 0) {
			PickupItem(bRightHandPickup); // We don't pickup a stack but a single item
			return;
		}

		bIsStackChecking = true;
		StackChecker->StartCheck(BaseItemToPick);

		if (PlayerCharacter->MovementComponent != nullptr) {
			PlayerCharacter->MovementComponent->SetMovable(false);
		}
	}
}

void UCPickup::OnStackCheckIsDone(bool wasSuccessful)
{

	bStackCheckSuccess = wasSuccessful;

	if (bStackCheckSuccess) {
		UE_LOG(LogTemp, Warning, TEXT("Stackcheck sucessful"));
		BaseItemToPick = GetItemStack(ItemToHandle); // We need to reassign the whole stack
		PickupItem(bRightHandPickup);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Stackcheck failed"));
	}

	bIsStackChecking = false;

	if (PlayerCharacter->MovementComponent != nullptr) {
		PlayerCharacter->MovementComponent->SetMovable(true);
	}
}

void UCPickup::StartDrag(bool bIsRightHand)
{
	ItemToDrag = ItemToHandle;
	bIsDragging = true;

	SetLockedByComponent(true);
}

void UCPickup::DragItem(bool bIsRightHand)
{
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(ItemToDrag);

	FHitResult RaycastResult = RaytraceWithIgnoredActors(IgnoredActors);

	FVector RayPosition = RaycastResult.Location;

	if (RayPosition.IsZero() == false)
	{
		FVector ItemOrigin;
		FVector ItemBoundExtend;
		ItemToDrag->GetActorBounds(false, ItemOrigin, ItemBoundExtend);

		FVector DeltaOfPivotToCenter = ItemOrigin - ItemToDrag->GetActorLocation();

		//FVector NewPosition = RayPosition + FVector(0, 0, ItemBoundExtend.Z) - DeltaOfPivotToCenter;
		FVector NewPosition = FVector(RayPosition.X, RayPosition.Y, ItemOrigin.Z) - DeltaOfPivotToCenter;

		ItemToDrag->SetActorLocation(NewPosition);
		// ItemToDrag->SetActorRotation(FRotator::ZeroRotator);
	}

}

void UCPickup::EndDrag()
{
	ItemToDrag = nullptr;
	bIsDragging = false;
}

void UCPickup::DropItem(bool bIsRightHand)
{
	if (ShadowBaseItem == nullptr) return;

	AStaticMeshActor* ItemToDrop = bIsRightHand ? ItemInRightHand : ItemInLeftHand;

	if (ItemToDrop == nullptr) return;

	TArray<AActor*> ChildItemsOfBaseItem;
	ItemToDrop->GetAttachedActors(ChildItemsOfBaseItem);

	ItemToDrop->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	ItemToDrop->GetStaticMeshComponent()->SetSimulatePhysics(true);
	ItemToDrop->SetActorLocation(ShadowBaseItem->GetActorLocation());
	ItemToDrop->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);


	for (auto& DroppedItem : ChildItemsOfBaseItem) {
		AStaticMeshActor* CastActor = Cast<AStaticMeshActor>(DroppedItem);
		DroppedItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		if (CastActor == nullptr) return;
		CastActor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CastActor->GetStaticMeshComponent()->SetSimulatePhysics(true);
	}

	if (ItemInRightHand == ItemInLeftHand) {
		ItemInRightHand = ItemInLeftHand = nullptr;
	}
	else if (bIsRightHand) {
		ItemInRightHand = nullptr;
	}
	else {
		ItemInLeftHand = nullptr;
	}
}

void UCPickup::ShadowDropItem(bool bIsRightHand)
{
	DisableShadowItems();

	AStaticMeshActor* ItemToDrop = bIsRightHand ? ItemInRightHand : ItemInLeftHand;

	if (ItemToDrop == nullptr) return;

	TArray<AActor*> ChildItemsOfBaseItem;
	ItemToDrop->GetAttachedActors(ChildItemsOfBaseItem);

	AStaticMeshActor* ShadowRoot = GetNewShadowItem(ItemToDrop);
	ItemToDrop->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	ShadowItems.Add(ShadowRoot);

	for (auto& ItemToShadow : ChildItemsOfBaseItem) {
		AStaticMeshActor* CastActor = Cast<AStaticMeshActor>(ItemToShadow);

		if (CastActor == nullptr) return;
		CastActor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

		AStaticMeshActor* ShadowItem = GetNewShadowItem(CastActor);
		ShadowItems.Add(ShadowItem);

		ShadowItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		FAttachmentTransformRules TransformRules = FAttachmentTransformRules::KeepWorldTransform;
		TransformRules.bWeldSimulatedBodies = true;

		ShadowItem->AttachToActor(ShadowRoot, TransformRules);
	}

	ShadowBaseItem = ShadowRoot;

	FVector CamLoc;
	FRotator CamRot;
	PlayerCharacter->Controller->GetPlayerViewPoint(CamLoc, CamRot); // Get the camera position and rotation

	FHitResult RaycastHit = RaytraceWithIgnoredActors(ChildItemsOfBaseItem);

	if (RaycastHit.Location != FVector::ZeroVector) {
		FVector FromPosition = CamLoc;
		FVector ToPosition = RaycastHit.Location;

		// *** Ignored Actors
		TArray<AActor*> IgnoredActors = ChildItemsOfBaseItem; // ignore items to pickup 
		IgnoredActors.Add(BaseItemToPick);
		IgnoredActors.Append(ShadowItems); //All shadow items are ignored
		IgnoredActors.Add(ItemInLeftHand);
		IgnoredActors.Add(ItemInRightHand);

		// Add all children in hands to ignored actors
		if (ItemInLeftHand != nullptr) {
			TArray<AActor*> ChildrenInLeftHand;
			ItemInLeftHand->GetAttachedActors(ChildrenInLeftHand);

			IgnoredActors.Append(ChildrenInLeftHand);
		}
		if (ItemInRightHand != nullptr) {
			TArray<AActor*> ChildrenInRightHand;
			ItemInRightHand->GetAttachedActors(ChildrenInRightHand);

			IgnoredActors.Append(ChildrenInRightHand);
		}
		// *****************

		FHitResult SweepHit = CheckForCollision(FromPosition, ToPosition, ShadowRoot, IgnoredActors);

		// ShadowRoot->SetActorScale3D(FVector(1.0f));

		if (SweepHit.GetActor() != nullptr) {
			ShadowRoot->SetActorLocation(SweepHit.Location);
		}
	}
}

void UCPickup::CancelActions()
{
	bAllCanceled = true;
	CancelDetachItems();
}

void UCPickup::StopCancelActions()
{
	bAllCanceled = false;
}

void UCPickup::CancelDetachItems()
{
	if (BaseItemToPick != nullptr) {
		TArray<AActor*> Children;
		BaseItemToPick->GetAttachedActors(Children);
		for (auto& item : Children) {
			AStaticMeshActor* CastActor = Cast<AStaticMeshActor>(item);

			CastActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			CastActor->GetStaticMeshComponent()->SetSimulatePhysics(true);
		}
	}
}

void UCPickup::AnimatePickup(float DeltaTime)
{
	/*if (PlayerCharacter->MovementComponent == nullptr) return;
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

				// Setup welding
				elem.GetItem()->GetStaticMeshComponent()->BodyInstance.bAutoWeld = true;
				FAttachmentTransformRules TransformRules = FAttachmentTransformRules::KeepWorldTransform;
				TransformRules.bWeldSimulatedBodies = true;

				if (elem.HandPosition == EHand::Right)
				{
					//	ItemsInRightHand.Add(elem.GetItem());

						//	elem.GetItem()->AttachToActor(RightHandActor, TransformRules, NAME_None);
				}
				else if (elem.HandPosition == EHand::Left)
				{
					//	ItemsInLeftHand.Add(elem.GetItem());
						//	elem.GetItem()->AttachToActor(LeftHandActor, TransformRules, NAME_None);
				}
			}

			if (PlayerCharacter->MovementComponent != nullptr) PlayerCharacter->MovementComponent->SetMovable(true); // Let the player move again
		}
	}

	AnimationList = AnimsToKeep;
	*/
}

void UCPickup::SetLockedByComponent(bool bIsLocked)
{
	if (bIsLocked) {
		PlayerCharacter->LockedByComponent = this;
	}
	else {
		PlayerCharacter->LockedByComponent = nullptr;
	}
}

bool UCPickup::CalculateIfBothHandsNeeded()
{
	BaseItemToPick->GetStaticMeshComponent()->SetSimulatePhysics(true);
	if (bBothHandsDependOnMass) {
		float MassOfItem = BaseItemToPick->GetStaticMeshComponent()->GetMass();
		UE_LOG(LogTemp, Warning, TEXT("Mass of object %f"), MassOfItem);

		if (MassOfItem >= MassThresholdTouseBothHands) {
			BaseItemToPick->GetStaticMeshComponent()->SetSimulatePhysics(false);
			return true;
		}
	}

	if (bBothHandsDependOnVolume) {
		float Volume = BaseItemToPick->GetStaticMeshComponent()->Bounds.GetBox().GetSize().SizeSquared();
		UE_LOG(LogTemp, Warning, TEXT("Volume of object %f"), Volume);

		if (Volume >= VolumeThresholdTouseBothHands) {
			BaseItemToPick->GetStaticMeshComponent()->SetSimulatePhysics(false);
			return true;
		}
	}
	BaseItemToPick->GetStaticMeshComponent()->SetSimulatePhysics(false);
	return false;
}

//void UCPickup::MoveItemsRelativeToHands(float DeltaTime)
//{
//	
//	if (ItemsInRightHand.Num() > 0)
//	{
//		for (auto& item : ItemsInRightHand) {
//			FVector HandPosition = PlayerCharacter->RightHandPosition->GetActorLocation();
//			item.Key->SetActorLocation(HandPosition + item.Value);
//			item.Key->SetActorRotation(GetOwner()->GetActorRotation());
//
//			item.Key->GetRootComponent()->ComponentVelocity = FVector::ZeroVector;
//			item.Key->GetStaticMeshComponent()->SetPhysicsAngularVelocity(FVector::ZeroVector);
//			item.Key->GetStaticMeshComponent()->SetPhysicsLinearVelocity(FVector::ZeroVector);
//		}
//	}
//
//	if (ItemsInLeftHand.Num() > 0)
//	{
//		for (auto& item : ItemsInLeftHand) {
//			FVector HandPosition = PlayerCharacter->LeftHandPosition->GetActorLocation();
//			item.Key->SetActorLocation(HandPosition + item.Value);
//			item.Key->SetActorRotation(GetOwner()->GetActorRotation());
//
//			item.Key->GetRootComponent()->ComponentVelocity = FVector::ZeroVector;
//			item.Key->GetStaticMeshComponent()->SetPhysicsAngularVelocity(FVector::ZeroVector);
//			item.Key->GetStaticMeshComponent()->SetPhysicsLinearVelocity(FVector::ZeroVector);
//		}
//	}
//	
//}

//void UCPickup::UpdateShadowItem(AStaticMeshActor * FromActor)
//{
//	FActorSpawnParameters Parameters;
//	Parameters.Template = FromActor;
//	ShadowBaseItem = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Parameters);
//
//	for (size_t i = 0; i < ShadowBaseItem->GetStaticMeshComponent()->GetMaterials().Num(); i++)
//	{
//		ShadowBaseItem->GetStaticMeshComponent()->SetMaterial(i, TransparentMaterial);
//	}
//
//	ShadowBaseItem->GetStaticMeshComponent()->SetSimulatePhysics(false);
//	ShadowBaseItem->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
//}

