// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ShooterMultiGameMode.h"
#include "ShooterMultiGameState.generated.h"

/**
 * 
 */

UCLASS()
class SHOOTERMULTI_API AShooterMultiGameState : public AGameState
{
	GENERATED_BODY()
	
public:

	AShooterMultiGameState(const FObjectInitializer & ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
};
