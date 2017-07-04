#define TAG_OPENCLOSABLE "OpenCloseable"

// Fill out your copyright notice in the Description page of Project Settings.
#include "COpenClose.h"

#include "../Private/Character/CharacterController.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"



// Sets default values for this component's properties
UCOpenClose::UCOpenClose()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;


	ForceToApply = 1500000.0f;
	bLocksComponent = true;
	// ...
}


// Called when the game starts
void UCOpenClose::BeginPlay()
{
	Super::BeginPlay();
	// ...

}


// Called every frame
void UCOpenClose::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (PlayerCharacter == nullptr) return;

	bool bLeftMouseHold = GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::LeftMouseButton);
	bool bLeftMouseClicked = GetWorld()->GetFirstPlayerController()->WasInputKeyJustPressed(EKeys::LeftMouseButton);
	bool bLeftMouseReleased = GetWorld()->GetFirstPlayerController()->WasInputKeyJustReleased(EKeys::LeftMouseButton);

	if (PlayerCharacter->FocusedActor != nullptr && PlayerCharacter->FocusedActor->Tags.Contains(TAG_OPENCLOSABLE) && bLeftMouseClicked)
	{
		ClickedActor = PlayerCharacter->FocusedActor;
		PlayerCharacter->MovementComponent->SetMovable(false);
	}

	if (bLeftMouseHold && ClickedActor != nullptr)
	{
		AddForceToObject(DeltaTime);
		ClickedActor->GetStaticMeshComponent()->SetRenderCustomDepth(true); // Keep object highlighted
	}
	else if (bLeftMouseReleased && ClickedActor != nullptr)
	{
		ClickedActor->GetStaticMeshComponent()->SetRenderCustomDepth(false);
		PlayerCharacter->MovementComponent->SetMovable(true);
		ClickedActor = nullptr;
	}
}

void UCOpenClose::AddForceToObject(float DeltaTime)
{
	float MouseX;
	float MouseY;
	GetWorld()->GetFirstPlayerController()->GetInputMouseDelta(MouseX, MouseY);

	//UE_LOG(LogTemp, Warning, TEXT("MouseX %f MouseY %f"), MouseX, MouseY);
	AStaticMeshActor* Actor = Cast<AStaticMeshActor>(ClickedActor);

	if (Actor == nullptr) return;

	// Get Normal vector than add force along this vector
	UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent();
	//Mesh->AddForce(NormalOfFocusedObject * -MouseY * DeltaTime * ForceToApply, NAME_None, true);
	Mesh->AddForce(Actor->GetActorForwardVector() * -MouseY * DeltaTime * ForceToApply, NAME_None, true);
}

