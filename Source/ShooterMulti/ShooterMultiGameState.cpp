// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterMulti.h"
#include "ShooterMultiGameState.h"
#include "UndeadCharacter.h"
#include "ShooterCharacter.h"

AShooterMultiGameState::AShooterMultiGameState(const FObjectInitializer & ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void AShooterMultiGameState::BeginPlay()
{
	Super::BeginPlay();
	PrimaryActorTick.bCanEverTick = true;
}


void AShooterMultiGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UE_LOG(LogTemp, Warning, TEXT(""));
}
