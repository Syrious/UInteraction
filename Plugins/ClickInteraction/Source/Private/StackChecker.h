// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "HUD/CIHUD.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Curves/CurveFloat.h"
#include "Components/TimelineComponent.h"
#include "StackChecker.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStackCheckDone, bool, wasSuccessful);

UCLASS()
class CLICKINTERACTION_API AStackChecker : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AStackChecker();

	UPROPERTY(EditAnywhere)
		UCurveFloat* AnimationCurveRotation;

	UPROPERTY(EditAnywhere)
		UCurveFloat* AnimationCurveTilting;

	UPROPERTY(EditAnywhere)
		UCurveFloat* AnimationCurveReverseTilting;
	// How much rotate
	UPROPERTY(EditAnywhere)
		float Rotation;

	// The maximium pitch
	UPROPERTY(EditAnywhere)
		float MaxTilt;

	// The maximium deviation of the item's relative distance squared
	UPROPERTY(EditAnywhere)
		float MaxDeviation;

	UPROPERTY()
		FStackCheckDone OnStackCheckDone;

	UPROPERTY(EditAnywhere)
		bool bShowStabilityCheckWindow;




	void StartCheck(AActor* BaseItemToCheck);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	FTimeline TimelineRotation;
	FTimeline TimelineTilting;
	FTimeline TimelineReverseTilting;

	FRotator StartRotation;
	FRotator EndRotation;

	FRotator StartTilt;
	FRotator EndTilt;

	AStaticMeshActor* StackCopy;

	TMap<AStaticMeshActor*, FVector> PositionsOnStart;

	float SumOfTimelineSeconds;
	float SecondsLeft;

	ACIHUD* ClickInteractionHUD;

	UFUNCTION()
		void Rotate(float Value);

	UFUNCTION()
		void Tilt(float Value);

	UFUNCTION()
		void TiltDone();

	UFUNCTION()
		void RotateDone();

	UFUNCTION()
		void TiltReverse(float Value);

	UFUNCTION()
		void TiltReverseDone();

	void CheckStackRelativePositions();

	bool bDone;

	AStaticMeshActor* CopyStack(AActor* ActorToCopy);
	AStaticMeshActor* CopyActor(AActor* ActorToCopy);
	void SetScreenCaptureEnabled(bool bEnabled);
};
