// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "ShooterCharacter.h"
#include "FindNearestPlayer.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERMULTI_API UFindNearestPlayer : public UBTService
{
	GENERATED_BODY()


	UFindNearestPlayer(const FObjectInitializer& ObjectInitializer);
	void ReceiveTick(AActor* OwnerActor, float DeltaSeconds);
	void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds);
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector PlayerKey;
};
