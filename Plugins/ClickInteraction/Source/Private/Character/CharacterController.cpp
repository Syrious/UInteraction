#define TAG_KEY_INTERACTABLE "Interactable"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 100000.0f,FColor::White,text, false);
#define printAndStop(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 100000.0f,FColor::Red,text, false); GetWorld()->GetFirstPlayerController()->SetPause(true);
#define logText(text) UE_LOG(LogTemp, Warning, TEXT(text));
#define logAndStop(text) UE_LOG(LogTemp, Warning, TEXT(text)); GetWorld()->GetFirstPlayerController()->SetPause(true);

#include "CharacterController.h"
#include "DrawDebugHelpers.h"
#include "TagStatics.h"
#include "SLContactManager.h"
#include "Engine.h" // Needed for GEngine

// #include "Runtime/Engine/Public/EngineGlobals.h"
//#include "GameFramework/Actor.h"
//#include "GameFramework/Controller.h"
//#include "Components/StaticMeshComponent.h"
//#include "Components/InputComponent.h"
//#include "Components/CapsuleComponent.h"

// Sets default values
ACharacterController::ACharacterController()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GraspRange = 120.0f;
	bRaytraceEnabled = true;

	GetCapsuleComponent()->SetCapsuleRadius(0.01f);

	SetupComponentsOnConstructor();
}

// Called when the game starts or when spawned
void ACharacterController::BeginPlay()
{
	Super::BeginPlay();

	//if (MovementComponent == nullptr)logText("ACharacterController::SetupComponents: No MovementComponent found");
	//if (OpenCloseComponent == nullptr)logText("ACharacterController::SetupComponents: No OpenCloseComponent found");
	////	if (RotateButtonComponent == nullptr)logText("ACharacterController::SetupComponents: No RotateButtonComponent found");
	//if (PickupComponent == nullptr)logText("ACharacterController::SetupComponents: No PickupComponent found");
	//if (LogComponent == nullptr)logText("ACharacterController::SetupComponents: No LogComponent found");


	Character = Cast<ACharacter>(this);

	if (Character == nullptr)
	{
		printAndStop("ACharacterController::BeginPlay: Character is null. Game paused to prevent crash.");
		Character->PlayerState->bIsSpectator = true;
	}

	SetOfInteractableItems = FTagStatics::GetActorSetWithKeyValuePair(GetWorld(), "ClickInteraction", TAG_KEY_INTERACTABLE, "True");


	// Find all SLContactManagerComponents in the world which then will be ignored when raycasting
	if (GetWorld() != nullptr) {
		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
			AActor* Actor = *ActorItr;

			UActorComponent* ActorComponent = Actor->GetComponentByClass(USLContactManager::StaticClass());
			if (ActorComponent != nullptr) {
				UE_LOG(LogTemp, Warning, TEXT("Found component %s in Actor %s"), *ActorComponent->GetName(), *Actor->GetName());

				UPrimitiveComponent* CastComponent = Cast<UPrimitiveComponent>(ActorComponent);

				if (CastComponent != nullptr) {
					USemLogContactManagers.Add(CastComponent);
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("Cast failed %s"), *ActorComponent->GetName());
				}
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("Added %i SLContactManagers to be ignored"), USemLogContactManagers.Num());
	}

	// Get Levelinfo
	// Get the semantic log runtime manager from the world
	for (TActorIterator<ASLLevelInfo>RMItr(GetWorld()); RMItr; ++RMItr)
	{
		LevelInfo = *RMItr;
		break;
	}

	SetupScenario();
	GenerateLevelInfo(); // Start logging
}

// Called every frame
void ACharacterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check if the player can move or should be movable again
	//if (bInteractionKeyWasJustReleased)
	//{
	//	SetPlayerMovable(true);
	//	FocusedActor = nullptr;
	//	bInteractionKeyWasJustReleased = false;
	//}

	StartRaytrace();
	//	UpdateIgnoredActors();
	CheckIntractability();

	if (bIsDebugMode && FocusedActor != nullptr)
	{
		DrawDebugLine(GetWorld(), RaycastResult.ImpactPoint, RaycastResult.ImpactPoint + 20 * RaycastResult.ImpactNormal, FColor::Blue, false, 0, 0, .5f);
	}

	if (PickupComponent == nullptr) UE_LOG(LogTemp, Warning, TEXT("NULL"));
}

// Called to bind functionality to input
void ACharacterController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (MovementComponent != nullptr) MovementComponent->SetupKeyBindings(PlayerInputComponent);
	if (PickupComponent != nullptr) PickupComponent->SetupKeyBindings(PlayerInputComponent);
	if (OpenCloseComponent != nullptr) OpenCloseComponent->SetupKeyBindings(PlayerInputComponent);
}

void ACharacterController::SetupComponentsOnConstructor()
{
	if (MovementComponent == nullptr) {
		MovementComponent = CreateDefaultSubobject<UCMovement>(TEXT("Movement Component"));
		MovementComponent->bEditableWhenInherited = true;
		AddInstanceComponent(MovementComponent);
		MovementComponent->RegisterComponent();

		//	AddOwnedComponent(MovementComponent);		
	}

	if (OpenCloseComponent == nullptr) {
		OpenCloseComponent = CreateDefaultSubobject<UCOpenClose>(TEXT("OpenClose Component"));
		OpenCloseComponent->bEditableWhenInherited = true;
		AddInstanceComponent(OpenCloseComponent);
		OpenCloseComponent->RegisterComponent();

		//AddOwnedComponent(OpenCloseComponent);
		OpenCloseComponent->PlayerCharacter = this;
	}

	if (PickupComponent == nullptr) {
		PickupComponent = CreateDefaultSubobject<UCPickup>(TEXT("Pickup Component"));
		PickupComponent->bEditableWhenInherited = true;
		AddInstanceComponent(PickupComponent);
		PickupComponent->RegisterComponent();

		//AddOwnedComponent(PickupComponent);
		PickupComponent->PlayerCharacter = this;
	}

	if (LogComponent == nullptr) {
		LogComponent = CreateDefaultSubobject<UCLogger>(TEXT("Log Component"));
		LogComponent->bEditableWhenInherited = true;
		AddInstanceComponent(LogComponent);
		LogComponent->RegisterComponent();

		//AddOwnedComponent(LogComponent);
		LogComponent->PlayerCharacter = this;
	}
}

void ACharacterController::SetupComponents()
{
	TArray<UActorComponent*> Components;
	GetComponents(Components);

	for (auto & Component : Components)
	{
		// Look for MovementComponent
		if (Component->IsA(UCMovement::StaticClass()))
		{
			MovementComponent = Cast<UCMovement>(Component);
			continue;
		}

		// Look for OpenCloseComponent
		if (Component->IsA(UCOpenClose::StaticClass()))
		{
			OpenCloseComponent = Cast<UCOpenClose>(Component);
			if (OpenCloseComponent) OpenCloseComponent->PlayerCharacter = this;
			continue;
		}

		// Look for RotateButtonComponent
		//if (Component->IsA(UCRotateButton::StaticClass()))
		//{
		//	RotateButtonComponent = Cast<UCRotateButton>(Component);
		//	if (RotateButtonComponent) RotateButtonComponent->PlayerCharacter = this;
		//	continue;
		//}

		// Look for PickupComponent
		if (Component->IsA(UCPickup::StaticClass()))
		{
			PickupComponent = Cast<UCPickup>(Component);
			if (PickupComponent) {
				PickupComponent->PlayerCharacter = this;
			}
			continue;
		}

		// Look for LoggerComponent
		if (Component->IsA(UCLogger::StaticClass()))
		{
			LogComponent = Cast<UCLogger>(Component);
			if (LogComponent) {
				LogComponent->PlayerCharacter = this;
			}
			continue;
		}
	}

	if (MovementComponent == nullptr)logText("ACharacterController::SetupComponents: No MovementComponent found");
	if (OpenCloseComponent == nullptr)logText("ACharacterController::SetupComponents: No OpenCloseComponent found");
	//	if (RotateButtonComponent == nullptr)logText("ACharacterController::SetupComponents: No RotateButtonComponent found");
	if (PickupComponent == nullptr)logText("ACharacterController::SetupComponents: No PickupComponent found");
	if (LogComponent == nullptr)logText("ACharacterController::SetupComponents: No LogComponent found");
}

void ACharacterController::StartRaytrace()
{
	if (bRaytraceEnabled == false) return;
	FVector CamLoc;
	FRotator CamRot;

	Character->Controller->GetPlayerViewPoint(CamLoc, CamRot); // Get the camera position and rotation
	const FVector StartTrace = CamLoc; // trace start is the camera location
	const FVector Direction = CamRot.Vector();
	const FVector EndTrace = StartTrace + Direction * GraspRange; // and trace end is the camera location + an offset in the direction

	FCollisionQueryParams TraceParams;

	TraceParams.AddIgnoredActor(this); // We don't want to hit ourself
	TraceParams.AddIgnoredComponents(USemLogContactManagers);

	TArray<AActor*> IgnoredActors;
	if (PickupComponent != nullptr) {
		if (PickupComponent->ItemInRightHand != nullptr) {
			IgnoredActors.Add(PickupComponent->ItemInRightHand);

			TArray<AActor*> ChildrenOfItem;
			PickupComponent->ItemInRightHand->GetAttachedActors(ChildrenOfItem);
			IgnoredActors.Append(ChildrenOfItem);
		}

		if (PickupComponent->ItemInLeftHand != nullptr) {
			IgnoredActors.Add(PickupComponent->ItemInLeftHand);

			TArray<AActor*> ChildrenOfItem;
			PickupComponent->ItemInLeftHand->GetAttachedActors(ChildrenOfItem);
			IgnoredActors.Append(ChildrenOfItem);
		}
	}

	TraceParams.AddIgnoredActors(IgnoredActors);

	GetWorld()->LineTraceSingleByChannel(RaycastResult, StartTrace, EndTrace, ECollisionChannel::ECC_Visibility, TraceParams);

	//if (RaycastResult.Component != nullptr) {
	//	UE_LOG(LogTemp, Warning, TEXT("looking at actor %s"), *RaycastResult.Component->GetName());
	//}

	//if (bIsDebugMode)
	//{
	//	 DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Green, true, 3, 0, 0.05f);
	//}
}



void ACharacterController::CheckIntractability()
{
	AActor* Actor = RaycastResult.GetActor();

	if (Actor != nullptr)
	{
		if (bIsDebugMode)
		{
			// UE_LOG(LogTemp, Warning, TEXT("Hit actor %s"), *Actor->GetName());
		}

		if (SetOfInteractableItems.Contains(Actor))
		{
			// Deactivate outline of previous object
			if (FocusedActor != nullptr && FocusedActor != Actor)
			{
				FocusedActor->GetStaticMeshComponent()->SetRenderCustomDepth(false);
			}

			FocusedActor = Cast<AStaticMeshActor>(Actor);
			FocusedActor->GetStaticMeshComponent()->SetRenderCustomDepth(true);
		}
		else if (FocusedActor != nullptr && bComponentsLocked == false)
		{
			FocusedActor->GetStaticMeshComponent()->SetRenderCustomDepth(false);
			FocusedActor = nullptr;
		}
	}
	else
	{
		if (FocusedActor != nullptr)
		{
			FocusedActor->GetStaticMeshComponent()->SetRenderCustomDepth(false);
			FocusedActor = nullptr;
		}
	}
}

//void ACharacterController::ReleaseInteractionKey()
//{
//	bInteractionKeyWasJustReleased = true;
//}

void ACharacterController::SetPlayerMovable(bool bIsMovable)
{
	if (MovementComponent != nullptr)
	{
		bIsMovementLocked = !bIsMovable;
		MovementComponent->SetMovable(bIsMovable);
	}
}

void ACharacterController::SetupScenario()
{
	switch (InteractionMode) {
	case EInteractionMode::OneHandMode:
		if (PickupComponent != nullptr) {
			PickupComponent->bTwoHandMode = false;
			PickupComponent->bStackModeEnabled = false;
		}
		break;
	case EInteractionMode::TwoHandMode:
		if (PickupComponent != nullptr) {
			PickupComponent->bTwoHandMode = true;
			PickupComponent->bStackModeEnabled = false;
		}
		break;
	case EInteractionMode::TwoHandStackingMode:
		if (PickupComponent != nullptr) {
			PickupComponent->bTwoHandMode = true;
			PickupComponent->bStackModeEnabled = true;
		}
		break;
	}
}

void ACharacterController::GenerateLevelInfo()
{
	if (LevelInfo != nullptr) {

		FOwlTriple LevelProperties;
		FOwlTriple LevelNumberHands;
		FOwlTriple LevelStackingEnabled;

		FString ScenarioText; // Scenario for own logger
		FString GameMode; // Mode for own logger


		switch (ScenarioType) {
		case EScenarioType::OnePersonBreakfast:
			LevelProperties = FOwlTriple("knowrob:taskContext", "rdf:datatype", "&xsd;string", "OnePersonBreakfast");
			ScenarioText = FString("OnePersonBreakfast");
			break;
		case EScenarioType::TwoPersonBreakfast:
			LevelProperties = FOwlTriple("knowrob:taskContext", "rdf:datatype", "&xsd;string", "TwoPersonBreakfast");
			ScenarioText = FString("TwoPersonBreakfast");
			break;
		case EScenarioType::FourPersonBreakfast:
			LevelProperties = FOwlTriple("knowrob:taskContext", "rdf:datatype", "&xsd;string", "FourPersonBreakfast");
			ScenarioText = FString("FourPersonBreakfast");
			break;
		}

		if (PickupComponent != nullptr) {

			if (PickupComponent->bTwoHandMode) {
				LevelNumberHands = FOwlTriple("knowrob:numberOfHands", "rdf:datatype", "&xsd;integer", "2");
				GameMode.Append("Two-Hand");
			}
			else {
				LevelNumberHands = FOwlTriple("knowrob:numberOfHands", "rdf:datatype", "&xsd;integer", "1");
				GameMode.Append("One-Hand");
			}

			if (PickupComponent->bStackModeEnabled) {
				LevelStackingEnabled = FOwlTriple("knowrob:stackingEnabled", "rdf:datatype", "&xsd;bool", "True");
				GameMode.Append(" (Stack)");
			}
			else {
				LevelStackingEnabled = FOwlTriple("knowrob:stackingEnabled", "rdf:datatype", "&xsd;bool", "False");
			}
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("ACharacterController::GenerateLevelInfo: Pickup component not found"));
		}

		LevelInfo->LevelProperties.Add(LevelProperties);
		LevelInfo->LevelProperties.Add(LevelNumberHands);
		LevelInfo->LevelProperties.Add(LevelStackingEnabled);

		if (LogComponent != nullptr) {
			LogComponent->StartLogger(ScenarioText, GameMode);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("ACharacterController::GenerateLevelInfo: Log component not found"));
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("ACharacterController::GenerateLevelInfo: Skipping LevelInfo. LevelInfo actor not found"));
	}
}

FHitResult ACharacterController::GetRaycastResult()
{
	return RaycastResult;
}





