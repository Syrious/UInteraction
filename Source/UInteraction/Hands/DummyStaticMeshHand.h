#pragma once

#include "Engine/StaticMeshActor.h"
#include "DummyStaticMeshHand.generated.h"

/**
 * 
 */
UCLASS()
class UINTERACTION_API ADummyStaticMeshHand : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	// Constructor
	ADummyStaticMeshHand();

	// Called when game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Get the location controller output
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	FVector GetLocationControllerOutput();

	// Get the rotation controller output
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	FVector GetRotationControllerOutput();

	// Hand type
	UPROPERTY(EditAnywhere, Category = "Interaction")
	EControllerHand HandType;

	// Rotation controller output strenght
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float RotationOutputMagnitude;

	// PID controller proportional argument
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float PGain;

	// PID controller integral argument
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float IGain;

	// PID controller derivative argument
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float DGain;

	// PID controller maximum output
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float PIDMaxOutput;

	// PID controller minimum output
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float PIDMinOutput;

private:
	// PID Controller
	class PIDController3D* LocationController;

	// Motion controller component for the hand to follow
	class UMotionControllerComponent* MCComponent;

	// Current location controller output
	FVector RotationControllerOutput;

	// Current location controller output
	FVector LocationControllerOutput;
};
