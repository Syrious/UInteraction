// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "CoreMinimal.h"
#include "SLRuntimeManager.h"
#include "COpenClose.generated.h"

class ACharacterController; // Use Forward Declaration. Including the header in COpenClose.cpp

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CLICKINTERACTION_API UCOpenClose : public UActorComponent
{
	GENERATED_BODY()


public:
	// Sets default values for this component's properties
	UCOpenClose();

	UPROPERTY(EditAnyWhere, Category = "CI - General")
		float ForceToApply;

	//UPROPERTY(EditAnyWhere)
	//	bool bLocksComponent;

	ACharacterController * PlayerCharacter;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void SetupKeyBindings(UInputComponent* PlayerInputComponent);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
//	bool bLastUsedHandWasRightHand;
	TSet<AActor*> SetOfOPenCloasableItems;

	void SetLockedByComponent(bool bIsLocked);

	AStaticMeshActor* ClickedActor;
	void AddForceToObject(float DeltaTime);

	ASLRuntimeManager* SemLogRuntimeManager;
	TSharedPtr<FOwlNode> LogEvent;

	void StartLogEvent(AActor* ActorToHandle);
	void FinishLogEvent(AActor* ActorToHandle);

	// *** Input ***
	bool bLeftMouseHold;
	//bool bLeftMouseClicked;
	//bool bLeftMouseReleased;

	bool bRightMouseHold;
	//bool bRightMouseClicked;
	//bool bRightMouseReleased;

	void InputLeftHandPressed();
	void InputLeftHandReleased();
	void InputRightHandPressed();
	void InputRightHandReleased();


	void OnInteractionKeyPressed(bool bIsRightKey);
	void OnInteractionKeyHold(bool bIsRightKey, float DeltaTime);
	void OnInteractionKeyReleased(bool bIsRightKey);

	// void ResetInput();
};
