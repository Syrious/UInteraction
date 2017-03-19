#include "UInteraction.h"
#include "MotionControllerComponent.h"
#include "Utils/PIDCOntroller3D.h"
#include "DummyStaticMeshHand.h"

// Constructor
ADummyStaticMeshHand::ADummyStaticMeshHand()
{
	// Call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	// Default PID parameters
	PGain = 140.0f;
	IGain = 0.0f;
	DGain = 20.0f;
	PIDMaxOutput = 1500.0f;
	PIDMinOutput = -1500.0f;

	// Rot output magnitude
	RotationOutputMagnitude = 100.f;

	// Default hand type
	HandType = EControllerHand::Left;
}

// Called when the game starts or when spawned
void ADummyStaticMeshHand::BeginPlay()
{
	Super::BeginPlay();

	// Get the MC character 
	ACharacter* PossesedCharacter =
		Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	// Get the motion controller component for the hand
	if (PossesedCharacter)
	{
		TArray<UActorComponent*> MCs = 
			PossesedCharacter->GetComponentsByClass(UMotionControllerComponent::StaticClass());
		for (const auto& Comp : MCs)
		{			
			UMotionControllerComponent* MC = Cast<UMotionControllerComponent>(Comp);
			if (MC && MC->Hand == HandType)
			{
				MCComponent = MC;
			}
		}

		// If no motion controller component found, disable tick and return
		if (!MCComponent)
		{
			// Disable tick for hand
			SetActorTickEnabled(false);
			UE_LOG(LogTemp, Error,
				TEXT("No motion control component was found! Tracking disabled for %s !"), *GetName());
			return;
		}
	}
	else
	{
		// Disable tick for hand
		SetActorTickEnabled(false);
		UE_LOG(LogTemp, Error,
			TEXT("No character with motion controller found! Tracking disabled for %s !"), *GetName());
		return;
	}

	// Check if the actor has a static mesh component set
	if (GetStaticMeshComponent()->GetStaticMesh())
	{
		// Linear movement PIDs
		LocationController = new PIDController3D(PGain, IGain, DGain, PIDMaxOutput, PIDMinOutput);
	}
	else
	{
		// Disable tick for hand
		SetActorTickEnabled(false);
		UE_LOG(LogTemp, Error,
			TEXT("No static mesh component was found! Tracking disabled for %s !"), *GetName());
		return;
	}
}

// Called every frame, used for motion control
void ADummyStaticMeshHand::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FVector CurrLoc = GetStaticMeshComponent()->GetComponentLocation();
	FQuat CurrQuat = GetStaticMeshComponent()->GetComponentQuat();

	// Get the target (motion controller) location and rotation
	const FVector TargetLoc = MCComponent->GetComponentLocation();
	const FQuat TargetQuat = MCComponent->GetComponentQuat();

	//// Location
	// Get the pos errors
	const FVector LocError = TargetLoc - CurrLoc;
	// Compute the pos output
	LocationControllerOutput = LocationController->Update(LocError, DeltaTime);
	// Apply force to the hands control body 
	//GetStaticMeshComponent()->AddForce(LocationControllerOutput);
	GetStaticMeshComponent()->SetPhysicsLinearVelocity(LocationControllerOutput);
	//GetStaticMeshComponent()->SetAllPhysicsLinearVelocity(LocationControllerOutput);

	//// Rotation
	// Dot product to get costheta
	const float CosTheta = TargetQuat | CurrQuat;
	// Avoid taking the long path around the sphere
	if (CosTheta < 0)
	{
		CurrQuat = CurrQuat * (-1.f);
	}
	// Use the xyz part of the quat as the rotation velocity
	const FQuat OutputFromQuat = TargetQuat * CurrQuat.Inverse();
	// Get the rotation output
	RotationControllerOutput = FVector(OutputFromQuat.X, OutputFromQuat.Y, OutputFromQuat.Z) * RotationOutputMagnitude;
	// Apply torque/angularvel to the hands control body 
	GetStaticMeshComponent()->SetPhysicsAngularVelocity(RotationControllerOutput);

}

// Get linear controller output
FVector ADummyStaticMeshHand::GetLocationControllerOutput()
{
	return LocationControllerOutput;
}

// Get rotation controller output
FVector ADummyStaticMeshHand::GetRotationControllerOutput()
{
	return RotationControllerOutput;
}