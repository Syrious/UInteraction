// Fill out your copyright notice in the Description page of Project Settings.
#include "CMovement.h"

#include "../Private/Character/CharacterController.h"
#include "Runtime/Engine/Classes/GameFramework/Character.h"
#include "Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h"



// Sets default values for this component's properties
UCMovement::UCMovement()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UCMovement::BeginPlay()
{
	Super::BeginPlay();

	Character = Cast<ACharacter>(GetOwner()); // Setup player

	// TODO This has been moved to CharacterController. Delete this if no problems occure


	//if (Character)
	//{
	//	UInputComponent* PlayerInputComponent = Character->InputComponent;

	//	/* Setup Input
	//	/* In Project setings -> Input
	//	/* Axis Mappings:
	//	/*   MoveForward: W 1.0, S -1,0
	//	/*   MoveRight: D 1.0, A -1.0
	//	/*   CameraPitch: MouseY -1.0
	//	/*   CameraYaw: MouseX 1.0
	//	*/
	//	if (PlayerInputComponent)
	//	{
	//		// Set up gameplay key bindings
	//		PlayerInputComponent->BindAxis("MoveForward", this, &UCMovement::MoveForward);
	//		PlayerInputComponent->BindAxis("MoveRight", this, &UCMovement::MoveRight);
	//		// Default Camera view bindings
	//		PlayerInputComponent->BindAxis("CameraPitch", this, &UCMovement::AddControllerPitchInput);
	//		PlayerInputComponent->BindAxis("CameraYaw", this, &UCMovement::AddControllerYawInput);
	//	}
	//}
	//***************************************************************
	SetMovable(true);

}

// TODO make it analog
void UCMovement::MoveForward(const float Val) {
	if (bCanMove == false) return;

	float SpeedFactor = 1.0f; // TODO Hardcoded

	if ((Character->Controller != nullptr) && (Val != 0.0f))
	{
		// Find out which way is forward
		FRotator Rotation = Character->Controller->GetControlRotation();
		// Limit pitch when walking or falling
		if (Character->GetCharacterMovement()->IsMovingOnGround() || Character->GetCharacterMovement()->IsFalling())
		{
			Rotation.Pitch = 0.0f;
		}
		// add movement in that direction
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
		Character->AddMovementInput(Direction, Val * SpeedFactor);
	}
}

void UCMovement::MoveRight(const float Val) {
	if (bCanMove == false) return;

	float SpeedFactor = 1.0f; // TODO Hardcoded

	if ((Character->Controller != nullptr) && (Val != 0.0f))
	{
		// Find out which way is right
		const FRotator Rotation = Character->Controller->GetControlRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
		// add movement in that direction
		Character->AddMovementInput(Direction, Val * SpeedFactor);
	}
}

void UCMovement::AddControllerPitchInput(const float Val) {
	if (bCanMove == false) return;

	if (Character != nullptr)
	{
		Character->AddControllerPitchInput(Val);
	}
}

void UCMovement::AddControllerYawInput(const float Val) {
	if (bCanMove == false) return;
	if (Character != nullptr)
	{
		Character->AddControllerYawInput(Val);
	}
}


// Called every frame
void UCMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCMovement::SetMovable(bool bCanMove)
{
	this->bCanMove = bCanMove;
}