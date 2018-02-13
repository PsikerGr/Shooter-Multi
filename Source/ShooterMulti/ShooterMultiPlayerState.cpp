// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterMulti.h"
#include "ShooterMultiPlayerState.h"
#include "UndeadCharacter.h"
#include "ShooterCharacter.h"


AShooterMultiPlayerState::AShooterMultiPlayerState()
	: Super()
{

}

void AShooterMultiPlayerState::GetLifetimeReplicatedProps(TArray <FLifetimeProperty> & OutLifetimeProps)
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterMultiPlayerState, Kills);
	DOREPLIFETIME(AShooterMultiPlayerState, Deaths);
}

void AShooterMultiPlayerState::BeginPlay()
{
	bReplicates = true;
	Deaths = 0;
	Kills = 0;
	bReplicates = true;
}

void AShooterMultiPlayerState::Destroyed()
{
}

void AShooterMultiPlayerState::AddKill()
{
	++Kills;
}

void AShooterMultiPlayerState::AddDeath()
{
	++Deaths;
}

void AShooterMultiPlayerState::SetTeam(EShooterTeam pTeam)
{
	Team = pTeam;
}
