// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../Private/Character/Components/CMovement.h"
#include "../Private/Character/Components/COpenClose.h"
#include "../Private/Character/Components/CPickup.h"
#include "../Private/Character/Components/CLogger.h"
#include "SLLevelInfo.h"

#include "Engine/StaticMeshActor.h"
#include "CoreMinimal.h"

#include "GameFramework/Character.h"
#include "CharacterController.generated.h"

UENUM(BlueprintType)
enum class EScenarioType : uint8
{
	OnePersonBreakfast,
	TwoPersonBreakfast,
	FourPersonBreakfast 
};

UENUM(BlueprintType)
enum class EInteractionMode : uint8
{
	OneHandMode,
	TwoHandMode,
	TwoHandStackingMode
};

UCLASS()
class CLICKINTERACTION_API ACharacterController : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterController();

	// The context scenario type
	UPROPERTY(EditAnywhere, Category = "CI - Scenario Setup")
		EScenarioType ScenarioType;

	// The interaction mode
	UPROPERTY(EditAnywhere, Category = "CI - Scenario Setup")
		EInteractionMode InteractionMode;

	// The grasp range of the player
	UPROPERTY(EditAnywhere, Category = "CI - Player Setup")
		float GraspRange;

	// The left hand dummy actor
	UPROPERTY(EditAnywhere, Category = "CI - Player Setup")
		AActor* LeftHandPosition;

	// The right hand dummy actor
	UPROPERTY(EditAnywhere, Category = "CI - Player Setup")
		AActor* RightHandPosition;

	// The dummy actor for the both hand position
	UPROPERTY(EditAnywhere, Category = "CI - Player Setup")
		AActor* BothHandPosition;

	UPROPERTY(EditAnywhere, Category = "CI - Debug")
		bool bIsDebugMode;

	ASLLevelInfo* LevelInfo;

	bool bRaytraceEnabled;

	TArray<UPrimitiveComponent*> USemLogContactManagers;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	/* Components attached to this actor*/
	UPROPERTY(EditAnywhere, Instanced)
	UCMovement* MovementComponent;

	UPROPERTY(EditAnywhere, Instanced)
	UCOpenClose* OpenCloseComponent;

	//UPROPERTY(EditAnywhere)
	//UCRotateButton* RotateButtonComponent;

	UPROPERTY(EditAnywhere, Instanced)
	UCPickup* PickupComponent;

	UPROPERTY(EditAnywhere, Instanced)
	UCLogger* LogComponent;

	AStaticMeshActor* FocusedActor;

	UActorComponent* LockedByComponent; // If this isn't nuullptr this component has exclusive rights

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

	TSet<AActor*> SetOfInteractableItems;
	TMap<AActor*, TArray<UMaterialInterface*>> DefaultActorMaterial;
	

	bool bIsMovementLocked; // Whether or not the player can move

	bool bComponentsLocked;

	// Creates all actor components.
	void SetupComponentsOnConstructor();

	// Gets all the needed components of this character
	void SetupComponents(); // TODO delete

	void StartRaytrace();

	void CheckIntractability();

	void SetPlayerMovable(bool bIsMovable);

	void SetupScenario();

	void GenerateLevelInfo();
};
