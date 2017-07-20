// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "../../StackChecker.h"
#include "CoreMinimal.h"
//#include "../Structs/PickupAnimation.h"
#include "CPickup.generated.h"

UENUM()
 enum EHand
{
	Right UMETA(DisplayName = "Right"),
	Left UMETA(DisplayName = "Left"),
	Both UMETA(DisplayName = "Both")
};

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

	// Wether or not both hands needed for certain items (heavy or large items)
	UPROPERTY(EditAnyWhere)
		bool bUseBothHands;

	UPROPERTY(EditAnyWhere)
		bool bBothHandsDependOnMass;

	UPROPERTY(EditAnywhere)
		float MassThresholdTouseBothHands;

	UPROPERTY(EditAnyWhere)
		bool bBothHandsDependOnVolume;

	UPROPERTY(EditAnywhere)
		float VolumeThresholdTouseBothHands;

	UPROPERTY(EditAnyWhere)
		bool bLocksComponent;

	UPROPERTY(EditAnyWhere)
		UMaterial* TransparentMaterial;

	UPROPERTY(EditAnyWhere)
		AStackChecker* StackChecker;

	UPROPERTY(EditAnyWhere)
		bool bDoStackCheck;

	UPROPERTY(EditAnywhere)
		bool bCheckForCollisionsOnPickup;

	ACharacterController * PlayerCharacter;
	TArray<AStaticMeshActor*> ShadowItems;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	AStaticMeshActor* LeftHandActor;
	AStaticMeshActor* RightHandActor;
	AStaticMeshActor* BothHandActor;

	AStaticMeshActor* BaseItemToPick;

	// TArray<FPickupAnimation> AnimationList;
	TSet<AActor*> SetOfPickupItems;


	// TArray<AStaticMeshActor*> ItemsToPickup;
	TMap<AStaticMeshActor*, FVector> ItemsAndPositionToPickup;

	// Default damping values for picked up items
	float ItemLeftHandDefaultLinDamping;
	float ItemLeftHandDefaultAngDamping;
	float ItemRightHandDefaultLinDamping;
	float ItemRightHandDefaultAngDamping;

	EHand UsedHand;

	bool bIsDragging;
	bool bAllCanceled;

	bool bIsStackChecking;
	bool bStackCheckSuccess;

	AStaticMeshActor* ItemToHandle;
	AActor* ItemToDrag;
	AStaticMeshActor* ShadowBaseItem;

	bool bRightHandPickup; // TODo change this to enum
	void StartStackCheck();

	// The callback function after stack check is done
	UFUNCTION()
		void OnStackCheckIsDone(bool wasSuccessful);


	void StartDrag(bool bIsRightHand);
	void DragItem(bool bIsRightHand);
	void EndDrag();

	void StartPickup();
	void PickupItem(bool bIsRightHand);
	void ShadowPickupItem(bool bIsRightHand);
	TMap<AStaticMeshActor*, FVector> GetStackOfItems(AStaticMeshActor* ActorToPickup);
	AStaticMeshActor* GetItemStack(AStaticMeshActor* BaseItem); // Converts all found items to children of BaseIten
	AStaticMeshActor* GetNewShadowItem(AStaticMeshActor* FromActor);

	void DropItem(bool bIsRightHand);
	void ShadowDropItem(bool bIsRightHand);

	void CancelActions(); // Cancels all actions
	void StopCancelActions();
	void CancelDetachItems();

	void AnimatePickup(float DeltaTime);
	void SetLockedByComponent(bool bIsLocked);

	bool CalculateIfBothHandsNeeded();

//	void MoveItemsRelativeToHands(float DeltaTime);

//	void UpdateShadowItem(AStaticMeshActor* FromActor);
	FHitResult CheckForCollision(FVector From, FVector To, AStaticMeshActor* ItemToSweep, TArray<AActor*> IgnoredActors);
	void DisableShadowItems();
	FHitResult RaytraceWithIgnoredActors(TArray<AActor*> IgnoredActors, FVector StartOffset = FVector::ZeroVector, FVector TargetOffset = FVector::ZeroVector);
	void ResetRotationOfItemsInHand();

};

