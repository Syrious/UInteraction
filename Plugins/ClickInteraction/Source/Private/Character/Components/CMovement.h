// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "Components/ActorComponent.h"
#include "Runtime/Engine/Classes/GameFramework/Character.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CMovement.generated.h"

//class ACharacterController; // Use Forward Declaration. Including the header in CMovement.cpp

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CLICKINTERACTION_API UCMovement : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCMovement();

	// Called when the game starts
	virtual void BeginPlay() override;

	// Handles moving forward/backward
	void MoveForward(const float Val);

	// Handles strafing Left/Right
	void MoveRight(const float Val);

	void AddControllerPitchInput(const float Val);

	void AddControllerYawInput(const float Val);


	// The maximum (or default) speed
	UPROPERTY(EditAnywhere, Category = "CI - Speed Setup")
		float MaxMovementSpeed;

	// The minimum movement speed. Used if player picks up items which then effects speed
	UPROPERTY(EditAnywhere, Category = "CI - Speed Setup")
		float MinMovementSpeed;

	float CurrentSpeed;

protected:


public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void SetupKeyBindings(UInputComponent* PlayerInputComponent);

	void SetMovable(bool bCanMove);

	// Toggle crouch
	void ToggleCrouch();

private:
	ACharacter* Character;

	// Stores the default speed set at start game
	float DefaultSpeed;

	bool bCanMove;

	// *** Crouching ***
	float DefaultHeight;
	bool bIsCrouching;
	// Smooth crouch timer handle
	FTimerHandle CrouchTimer;

	// Smooth crouch
	void SmoothCrouch();

	// Smooth stand up
	void SmoothStandUp();
	// *** *** *** *** *** *** 

};
