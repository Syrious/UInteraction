// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "Components/ActorComponent.h"
#include "Runtime/Engine/Classes/GameFramework/Character.h"
#include "CoreMinimal.h"
#include "CMovement.generated.h"

class ACharacterController; // Use Forward Declaration. Including the header in COpenClose.cpp

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
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

protected:


public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetMovable(bool bCanMove);

private:
	ACharacter* Character;
	bool bCanMove;

};
