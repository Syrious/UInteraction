// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "CoreMinimal.h"
#include "../Structs/PickupAnimation.h"
#include "CPickup.generated.h"

class ACharacterController; // Use Forward Declaration. Including the header in CPickup.cpp

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CLICKINTERACTION_API UCPickup : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCPickup();

	AStaticMeshActor* ItemInLeftHand;
	AStaticMeshActor* ItemInRightHand;
	AStaticMeshActor* ItemInBothHands;
	
	UPROPERTY(EditAnyWhere)
		bool bLocksComponent;
	
	ACharacterController * PlayerCharacter;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	TArray<FPickupAnimation> AnimationList;

	// Default damping values for picked up items
	float ItemLeftHandDefaultLinDamping;
	float ItemLeftHandDefaultAngDamping;
	float ItemRightHandDefaultLinDamping;
	float ItemRightHandDefaultAngDamping;

	void PickupItem(bool bIsRightHand);
	void DropItem(bool bIsRightHand);
	void AnimatePickup(float DeltaTime);

	void MoveItemsRelativeToHands(float DeltaTime);
};

