// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../Private/Character/Components/CMovement.h"
#include "../Private/Character/Components/COpenClose.h"
#include "../Private/Character/Components/CRotateButton.h"
#include "../Private/Character/Components/CPickup.h"

#include "Engine/StaticMeshActor.h"
#include "CoreMinimal.h"


#include "GameFramework/Character.h"
#include "CharacterController.generated.h"

UCLASS()
class CLICKINTERACTION_API ACharacterController : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterController();

	UPROPERTY(EditAnywhere)
		float RaycastRange;

	UPROPERTY(EditAnywhere)
		bool bIsDebugMode;

	UPROPERTY(EditAnywhere)
		AActor* LeftHandPosition;

	UPROPERTY(EditAnywhere)
		AActor* RightHandPosition;

	UPROPERTY(EditAnywhere)
		AActor* BothHandPosition;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	/* Components attached to this actor*/
	UCMovement* MovementComponent;
	UCOpenClose* OpenCloseComponent;
	UCRotateButton* RotateButtonComponent;
	UCPickup* PickupComponent;

	AStaticMeshActor* FocusedActor;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FHitResult GetRaycastResult();

private:
	ACharacter* Character;
	FHitResult RaycastResult; // The result of the constant raycasting
	FVector PreviousPosition;
	FRotator PreviousRotation;

	bool bIsMovementLocked; // Whether or not the player can move

	bool bComponentsLocked;
							// Gets all the needed components of this character
	void SetupComponents();

	void StartRaytrace();

	void CheckIntractability();

	void SetPlayerMovable(bool bIsMovable);
};
