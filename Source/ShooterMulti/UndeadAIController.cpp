// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterMulti.h"
#include "UndeadAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "UndeadCharacter.h"


void AUndeadAIController::BeginPlay()
{
	Super::BeginPlay();

	if (BehaviorTree && BlackboardData)
	{
		UseBlackboard(BlackboardData, BlackboardComponent);
		RunBehaviorTree(BehaviorTree);
	}
}

void AUndeadAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	auto character = Cast<AUndeadCharacter>(GetPawn());

	if (character && character->IsDead())
		Destroy();
	
}

bool AUndeadAIController::Punch()
{
	auto character = Cast<AUndeadCharacter>(GetPawn());

	bool res = false;

	if (character && character->CanPunch())
	{
		
			character->RPCMultiCast_DoPunch();
			res = true;
	}

	return res;
}