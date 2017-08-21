// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Misc/FileHelper.h"
#include "SLOwl.h"
#include "SLRuntimeManager.h"
#include "CLogger.generated.h"

class ACharacterController; // Use Forward Declaration. Including the header in CLogger.cpp

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CLICKINTERACTION_API UCLogger : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCLogger();

	UPROPERTY(EditAnywhere, Category = "CI - Log")
		bool bEnableLogging;

	UPROPERTY(EditAnywhere, Category = "CI - Log")
		FString SaveDirectory;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	ACharacterController * PlayerCharacter;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void StartLogger(FString Scenario, FString Mode);
	void EndLogger();

	void WriteSummary();

	void StartEvent(TSharedPtr<FOwlNode> Event);
	void FinishEvent(TSharedPtr<FOwlNode> Event);
	void CancelEvent(TSharedPtr<FOwlNode> Event);

private:
	FString CurrentLog;
	FString FileName;
	int ActionCounter;
	float SumOfActionTime;
	float SumOfStackingTime;
	int CountStackEvents;
	float EndTimeOfLastEvent;

	TMap<TSharedPtr<FOwlNode>, float> StartTimeOfEvents;
	ASLRuntimeManager* SemLogRuntimeManager;

		
	
};
