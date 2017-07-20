// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
#define printTimelineInfo(text) if (GEngine) GEngine->AddOnScreenDebugMessage(1, 5.0f,FColor::Emerald,text, false);

#include "StackChecker.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Engine.h" // Needed for GEngine
#include "Components/StaticMeshComponent.h"



// Sets default values
AStackChecker::AStackChecker()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Rotation = 180.0f;
	MaxTilt = 20.0f;
	MaxDeviation = 50.0f;

	bShowStabilityCheckWindow = true;
}

// Called when the game starts or when spawned
void AStackChecker::BeginPlay()
{
	Super::BeginPlay();

	// Find HUD
	ClickInteractionHUD = Cast<ACIHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());

	// *** First tilt
	if (AnimationCurveTilting != nullptr) {
		FOnTimelineFloat TiltFunction;
		FOnTimelineEvent OnDoneFunction;

		TiltFunction.BindUFunction(this, FName("Tilt"));
		TimelineTilting.AddInterpFloat(AnimationCurveTilting, TiltFunction);
		TimelineTilting.SetLooping(false);

		OnDoneFunction.BindUFunction(this, FName("TiltDone"));
		TimelineTilting.SetTimelineFinishedFunc(OnDoneFunction);

		StartTilt = EndTilt = GetActorRotation();
		EndTilt.Pitch += MaxTilt;

		SumOfTimelineSeconds += TimelineTilting.GetTimelineLength();
	}

	// ** Rotate
	if (AnimationCurveRotation != nullptr) {
		FOnTimelineFloat RotateFunction;
		FOnTimelineEvent OnDoneFunction;

		RotateFunction.BindUFunction(this, FName("Rotate"));

		TimelineRotation.AddInterpFloat(AnimationCurveRotation, RotateFunction);
		TimelineRotation.SetLooping(false);

		OnDoneFunction.BindUFunction(this, FName("RotateDone"));
		TimelineRotation.SetTimelineFinishedFunc(OnDoneFunction);

		SumOfTimelineSeconds += TimelineRotation.GetTimelineLength();
	}

	// ** Reverse tilt
	if (AnimationCurveReverseTilting != nullptr) {
		FOnTimelineFloat TiltFunction;
		FOnTimelineEvent OnDoneFunction;

		TiltFunction.BindUFunction(this, FName("TiltReverse"));
		TimelineReverseTilting.AddInterpFloat(AnimationCurveReverseTilting, TiltFunction);
		TimelineReverseTilting.SetLooping(false);

		OnDoneFunction.BindUFunction(this, FName("TiltReverseDone"));
		TimelineReverseTilting.SetTimelineFinishedFunc(OnDoneFunction);

		SumOfTimelineSeconds += TimelineReverseTilting.GetTimelineLength();
	}

}

// Called every frame
void AStackChecker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TimelineTilting.TickTimeline(DeltaTime);
	TimelineRotation.TickTimeline(DeltaTime);
	TimelineReverseTilting.TickTimeline(DeltaTime);

	if (TimelineTilting.IsPlaying() || TimelineRotation.IsPlaying() || TimelineReverseTilting.IsPlaying()) {
		SecondsLeft -= DeltaTime;
		printTimelineInfo(FString("Running stability test: ") + FString::SanitizeFloat(SecondsLeft));
	}
}

void AStackChecker::StartCheck(AActor * BaseItemToCheck)
{
	SetScreenCaptureEnabled(true);
	SecondsLeft = SumOfTimelineSeconds;
	StackCopy = CopyStack(BaseItemToCheck);
	TimelineTilting.PlayFromStart();
}

void AStackChecker::Rotate(float Value)
{
	FRotator NewRotation = FMath::Lerp(StartRotation, EndRotation, Value);
	SetActorRotation(NewRotation);
}

void AStackChecker::Tilt(float Value)
{
	FRotator NewRotator = FMath::Lerp(StartTilt, EndTilt, Value);
	SetActorRotation(NewRotator);
}

void AStackChecker::TiltDone()
{
	StartRotation = EndRotation = GetActorRotation();
	EndRotation.Yaw += Rotation;

	TimelineRotation.PlayFromStart();
}

void AStackChecker::RotateDone()
{
	TimelineReverseTilting.PlayFromStart();
}

void AStackChecker::TiltReverse(float Value)
{
	FRotator NewRotator = FMath::Lerp(EndTilt, StartTilt, Value);
	SetActorRotation(NewRotator);
}

void AStackChecker::TiltReverseDone()
{
	CheckStackRelativePositions();
}

void AStackChecker::CheckStackRelativePositions()
{
	bool bIsSuccess = true;

	for (auto& StackedItem : PositionsOnStart) {

		FVector RelativePosition = StackedItem.Key->GetActorLocation() - StackCopy->GetActorLocation();

		UE_LOG(LogTemp, Warning, TEXT("%s old Relative position %s"), *StackedItem.Key->GetName(), *StackedItem.Value.ToCompactString());
		UE_LOG(LogTemp, Warning, TEXT("%s new Relative position %s"), *StackedItem.Key->GetName(), *RelativePosition.ToCompactString());

		float OldSquareSize = StackedItem.Value.SizeSquared();
		float NewSquareSize = RelativePosition.SizeSquared();

		UE_LOG(LogTemp, Warning, TEXT("%s old delta square distance %f"), *StackedItem.Key->GetName(), OldSquareSize);
		UE_LOG(LogTemp, Warning, TEXT("%s new delta square distance %f"), *StackedItem.Key->GetName(), NewSquareSize);

		if (FMath::Abs(NewSquareSize - OldSquareSize) > MaxDeviation) {
			UE_LOG(LogTemp, Warning, TEXT("AStackChecker::CheckStackRelativePositions: Stack test failed. Stack unstable."));
			bIsSuccess = false;
		}
	}

	OnStackCheckDone.Broadcast(bIsSuccess);

	StackCopy->Destroy();
	for (auto& StackedItem : PositionsOnStart) {
		StackedItem.Key->Destroy();
	}
	PositionsOnStart.Empty();

	UE_LOG(LogTemp, Warning, TEXT("AStackChecker::CheckStackRelativePositions: Test done"));

	if (bIsSuccess) {
		printTimelineInfo("Stability test done... success");
	}
	else {
		printTimelineInfo("Stability test done... failed");
	}
	SetScreenCaptureEnabled(false);
}

AStaticMeshActor* AStackChecker::CopyStack(AActor * ActorToCopy)
{
	AStaticMeshActor* BaseCopy = CopyActor(ActorToCopy);

	FAttachmentTransformRules TransformRules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
	TransformRules.bWeldSimulatedBodies = true;
	BaseCopy->GetStaticMeshComponent()->SetSimulatePhysics(true);

	BaseCopy->AttachToActor(this, TransformRules);
	BaseCopy->SetActorRelativeLocation(FVector::ZeroVector);

	TArray<AActor*> ChildActors;
	ActorToCopy->GetAttachedActors(ChildActors);

	for (auto& Child : ChildActors) {
		if (BaseCopy == nullptr) break;

		AStaticMeshActor* ChildCopy = CopyActor(Child);

		ChildCopy->GetStaticMeshComponent()->SetSimulatePhysics(true);
		// ChildCopy->AttachToActor(BaseCopy, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

		FVector RelativeLocation = Child->GetActorLocation() - ActorToCopy->GetActorLocation();
		FVector NewPosition = BaseCopy->GetActorLocation() + RelativeLocation;
		ChildCopy->SetActorRelativeLocation(NewPosition);

		PositionsOnStart.Add(ChildCopy, RelativeLocation);
	}

	BaseCopy->GetStaticMeshComponent()->UnWeldChildren();
	return BaseCopy;
}

AStaticMeshActor * AStackChecker::CopyActor(AActor * ActorToCopy)
{
	AStaticMeshActor* CastActor = Cast<AStaticMeshActor>(ActorToCopy);
	if (CastActor == nullptr) return nullptr;

	AStaticMeshActor* CopyActorBase = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass());
	CopyActorBase->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
	CopyActorBase->GetStaticMeshComponent()->SetStaticMesh(CastActor->GetStaticMeshComponent()->GetStaticMesh());

	TArray<UMaterialInterface*> Materials = CastActor->GetStaticMeshComponent()->GetMaterials();
	for (size_t i = 0; i < Materials.Num(); i++)
	{
		CopyActorBase->GetStaticMeshComponent()->SetMaterial(i, Materials[i]);
	}

	return CopyActorBase;
}

void AStackChecker::SetScreenCaptureEnabled(bool bEnabled)
{
	if (ClickInteractionHUD != nullptr) {
		ClickInteractionHUD->bStabilityCheckScreenEnabled = bEnabled;
	}
}
