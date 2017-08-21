// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "CRotateButton.generated.h"

class ACharacterController; // Use Forward Declaration. Including the header in CRotateButton.cpp

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CLICKINTERACTION_API UCRotateButton : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCRotateButton();

	ACharacterController * PlayerCharacter;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere)
		float ForceToApply;

	//UPROPERTY(EditAnyWhere)
	//	bool bLocksComponent;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	AStaticMeshActor* ClickedActor;
	TSet <AActor*> SetOfRotateableItems;

	void AddRadialForceToObject(float DeltaTime);
};
