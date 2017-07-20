// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "Components/ActorComponent.h"
#include "../Components/CPickup.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "CoreMinimal.h"
#include "PickupAnimation.generated.h"



USTRUCT()
struct FPickupAnimation
{
	GENERATED_BODY()

private:
	AStaticMeshActor* ObjectToMove;
	AActor* ActorToAttachOn;
	FVector StartPosition;
	FVector EndPosition;
	float AnimationTime;
	float currentTime;
	bool bSweep;
	bool isDone;

	UStaticMeshComponent* Mesh;

public:
	bool bDropItem;
	EHand HandPosition;
	FPickupAnimation() {}

	// _bDropItem = Are we droping or picking up?
	FPickupAnimation(AStaticMeshActor* ActorToMove, AActor* HandActor, float TimeOfAnimation, bool _bDropItem = false) {
		ObjectToMove = ActorToMove;
		StartPosition = ActorToMove->GetActorLocation();
		EndPosition = HandActor->GetActorLocation();
		AnimationTime = TimeOfAnimation;
		ActorToAttachOn = HandActor;
		bDropItem = _bDropItem;
		currentTime = 0;

		Mesh = ObjectToMove->GetStaticMeshComponent();
		Mesh->SetEnableGravity(false);
	}

	FPickupAnimation(AStaticMeshActor* ActorToMove, FVector PositionToMove, EHand FinalHandPosition, float TimeOfAnimation, bool _bDropItem = false) {
		ObjectToMove = ActorToMove;
		StartPosition = ActorToMove->GetActorLocation();
		EndPosition = PositionToMove;
		AnimationTime = TimeOfAnimation;
		bDropItem = _bDropItem;
		HandPosition = FinalHandPosition;
		currentTime = 0;

		Mesh = ObjectToMove->GetStaticMeshComponent();
		Mesh->SetEnableGravity(false);
	}

	void AnimateTick(float DeltaTime) {
		currentTime += DeltaTime;

		FVector NewPosition = FMath::Lerp(StartPosition, EndPosition, currentTime / AnimationTime);
		ObjectToMove->SetActorLocation(NewPosition);
	}

	bool IsAnimationDone() {
		float PercentDone = currentTime / AnimationTime;

		if (PercentDone >= 1)
		{
			Mesh->SetEnableGravity(true); // Enable gravity if we droped the item
			// Stop velocity
			Mesh->SetPhysicsAngularVelocity(FVector::ZeroVector);
			Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
			ObjectToMove->GetRootComponent()->ComponentVelocity = FVector::ZeroVector;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool operator==(const FPickupAnimation& Other) {
		return ObjectToMove == Other.ObjectToMove;
	}

	AActor* GetHandActor() {
		return ActorToAttachOn;
	}

	AStaticMeshActor* GetItem() {
		return ObjectToMove;
	}
};
