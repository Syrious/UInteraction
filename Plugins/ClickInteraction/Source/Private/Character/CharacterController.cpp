#define TAG_KEY_INTERACTABLE "Interactable"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 100000.0f,FColor::White,text, false);
#define printAndStop(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 100000.0f,FColor::Red,text, false); GetWorld()->GetFirstPlayerController()->SetPause(true);
#define logText(text) UE_LOG(LogTemp, Warning, TEXT(text));
#define logAndStop(text) UE_LOG(LogTemp, Warning, TEXT(text)); GetWorld()->GetFirstPlayerController()->SetPause(true);

#include "CharacterController.h"
#include "DrawDebugHelpers.h"
#include "TagStatics.h"
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
	RaycastRange = 150.0f;
	bRaytraceEnabled = true;

	GetCapsuleComponent()->SetCapsuleRadius(1);
}

// Called when the game starts or when spawned
void ACharacterController::BeginPlay()
{
	Super::BeginPlay();

	Character = Cast<ACharacter>(this);

	if (Character == nullptr)
	{
		printAndStop("ACharacterController::BeginPlay: Character is null. Game paused to prevent crash.");
		Character->PlayerState->bIsSpectator = true;
	}

	SetOfInteractableItems = FTagStatics::GetActorSetWithKeyValuePair(GetWorld(), "ClickInteraction", TAG_KEY_INTERACTABLE, "True");
}

// Called every frame
void ACharacterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check if the player can move or should be movable again
	if (GetWorld()->GetFirstPlayerController()->WasInputKeyJustReleased(EKeys::LeftMouseButton))
	{
		SetPlayerMovable(true);
		FocusedActor = nullptr;
	}

	StartRaytrace();
	CheckIntractability();

	if (bIsDebugMode && FocusedActor != nullptr)
	{
		DrawDebugLine(GetWorld(), RaycastResult.ImpactPoint, RaycastResult.ImpactPoint + 20 * RaycastResult.ImpactNormal, FColor::Blue, false, 0, 0, .5f);
	}
}

// Called to bind functionality to input
void ACharacterController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	SetupComponents();

	/* Setup Input
	/* In Project setings -> Input
	/* Axis Mappings:
	/*   MoveForward: W 1.0, S -1,0
	/*   MoveRight: D 1.0, A -1.0
	/*   CameraPitch: MouseY -1.0
	/*   CameraYaw: MouseX 1.0
	*/
	if (MovementComponent != nullptr)
	{
		// Set up gameplay key bindings
		PlayerInputComponent->BindAxis("MoveForward", MovementComponent, &UCMovement::MoveForward);
		PlayerInputComponent->BindAxis("MoveRight", MovementComponent, &UCMovement::MoveRight);
		// Default Camera view bindings
		PlayerInputComponent->BindAxis("CameraPitch", MovementComponent, &UCMovement::AddControllerPitchInput);
		PlayerInputComponent->BindAxis("CameraYaw", MovementComponent, &UCMovement::AddControllerYawInput);
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
		if (Component->IsA(UCRotateButton::StaticClass()))
		{
			RotateButtonComponent = Cast<UCRotateButton>(Component);
			if (RotateButtonComponent) RotateButtonComponent->PlayerCharacter = this;
			continue;
		}

		// Look for PickupComponent
		if (Component->IsA(UCPickup::StaticClass()))
		{
			PickupComponent = Cast<UCPickup>(Component);
			if (PickupComponent) PickupComponent->PlayerCharacter = this;
			continue;
		}
	}

	if (MovementComponent == nullptr)logText("ACharacterController::SetupComponents: No MovementComponent found");
	if (OpenCloseComponent == nullptr)logText("ACharacterController::SetupComponents: No OpenCloseComponent found");
	if (RotateButtonComponent == nullptr)logText("ACharacterController::SetupComponents: No RotateButtonComponent found");
	if (PickupComponent == nullptr)logText("ACharacterController::SetupComponents: No PickupComponent found");
}

void ACharacterController::StartRaytrace()
{
	if (bRaytraceEnabled == false) return;
	FVector CamLoc;
	FRotator CamRot;

	Character->Controller->GetPlayerViewPoint(CamLoc, CamRot); // Get the camera position and rotation
	const FVector StartTrace = CamLoc; // trace start is the camera location
	const FVector Direction = CamRot.Vector();
	const FVector EndTrace = StartTrace + Direction * RaycastRange; // and trace end is the camera location + an offset in the direction

	FCollisionQueryParams TraceParams;
	
	TraceParams.AddIgnoredActor(this); // We don't want to hit ourself

	GetWorld()->LineTraceSingleByChannel(RaycastResult, StartTrace, EndTrace, ECollisionChannel::ECC_Visibility, TraceParams);

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

void ACharacterController::SetPlayerMovable(bool bIsMovable)
{
	if (MovementComponent != nullptr)
	{
		bIsMovementLocked = !bIsMovable;
		MovementComponent->SetMovable(bIsMovable);
	}
}

FHitResult ACharacterController::GetRaycastResult()
{
	return RaycastResult;
}





