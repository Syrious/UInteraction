#define TAG_KEY_ROTATEBUTTON "RotateButton"

// Fill out your copyright notice in the Description page of Project Settings.
#include "CRotateButton.h"

#include "../Private/Character/CharacterController.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "TagStatics.h"
#include "GameFramework/PlayerController.h"


// Sets default values for this component's properties
UCRotateButton::UCRotateButton()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	ForceToApply = 50.0f;
	//bLocksComponent = true;
	// ...
}


// Called when the game starts
void UCRotateButton::BeginPlay()
{
	Super::BeginPlay();

	// ...
	SetOfRotateableItems = FTagStatics::GetActorSetWithKeyValuePair(GetWorld(), "ClickInteraction", TAG_KEY_ROTATEBUTTON, "True");
}


// Called every frame
void UCRotateButton::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PlayerCharacter == nullptr) return;

	bool bLeftMouseHold = GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::LeftMouseButton);
	bool bLeftMouseClicked = GetWorld()->GetFirstPlayerController()->WasInputKeyJustPressed(EKeys::LeftMouseButton);
	bool bLeftMouseReleased = GetWorld()->GetFirstPlayerController()->WasInputKeyJustReleased(EKeys::LeftMouseButton);

	if (PlayerCharacter->FocusedActor != nullptr && SetOfRotateableItems.Contains(PlayerCharacter->FocusedActor) && bLeftMouseClicked)
	{
		ClickedActor = PlayerCharacter->FocusedActor;
		PlayerCharacter->MovementComponent->SetMovable(false);
	}

	if (bLeftMouseHold && ClickedActor != nullptr)
	{
		AddRadialForceToObject(DeltaTime);
		ClickedActor->GetStaticMeshComponent()->SetRenderCustomDepth(true); // Keep object highlighted
	}
	else if (bLeftMouseReleased && ClickedActor != nullptr)
	{
		ClickedActor->GetStaticMeshComponent()->SetRenderCustomDepth(false);
		PlayerCharacter->MovementComponent->SetMovable(true);
		ClickedActor = nullptr;
	}
}

void UCRotateButton::AddRadialForceToObject(float DeltaTime)
{
	float MouseX;
	float MouseY;
	GetWorld()->GetFirstPlayerController()->GetInputMouseDelta(MouseX, MouseY);

	// UE_LOG(LogTemp, Warning, TEXT("MouseX %f MouseY %f"), MouseX, MouseY);
	AStaticMeshActor* Actor = Cast<AStaticMeshActor>(ClickedActor);

	if (Actor == nullptr) return;

	// Get Normal vector than add force along this vector
	UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent();
	Mesh->AddTorque(FVector(1, 0, 0) * MouseX * ForceToApply);
	//Mesh->AddRadialForce(FocusedObject->GetActorLocation(), 360, 1000000, ERadialImpulseFalloff::RIF_MAX, true);

	// Mesh->AddForce(FVector(MouseY * DeltaTime * 1000000, 0, 0));
}

