// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterMulti.h"
#include "ShooterMultiGameMode.h"
#include "ShooterCharacter.h"
#include "ShooterMultiPlayerState.h"


AShooterMultiGameMode::AShooterMultiGameMode():
	Super()
{
//	PrimaryActorTick.bCanEverTick = true;
}

void AShooterMultiGameMode::BeginPlay()
{
	Super::BeginPlay();	
	TeamEventHandle = AShooterCharacter::TeamEvent.AddLambda([this](AShooterCharacter* charac) { JoinTeam(Team); });
}

void AShooterMultiGameMode::Destroyed()
{
}

void AShooterMultiGameMode::Respawn(AController* pController)
{
	if (pController == nullptr)
		return;
	
	AActor* playerStart = ChoosePlayerStart(pController);
	if (playerStart != nullptr)
		RestartPlayerAtPlayerStart(pController, playerStart);
}

void AShooterMultiGameMode::JoinTeam(EShooterTeam team)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Team: %d"), team));
}

void AShooterMultiGameMode::Quit()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit);
}

