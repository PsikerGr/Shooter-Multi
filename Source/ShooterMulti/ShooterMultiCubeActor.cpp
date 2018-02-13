// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterMulti.h"
#include "ShooterMultiCubeActor.h"


// Sets default values
AShooterMultiCubeActor::AShooterMultiCubeActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AShooterMultiCubeActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AShooterMultiCubeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (Role == ROLE_Authority)
		RPCServer_UpdatePosRot(RootComponent->GetComponentLocation(), RootComponent->GetComponentRotation());

}

void AShooterMultiCubeActor::RPCServer_UpdatePosRot_Implementation(FVector Pos, FRotator Rot)
{
	RootComponent->SetWorldLocationAndRotation(Pos, Rot, false);
	RPCMulticast_UpdatePosRot(Pos, Rot);
}

bool AShooterMultiCubeActor::RPCServer_UpdatePosRot_Validate(FVector Pos, FRotator Rot)
{
	return true;
}

void AShooterMultiCubeActor::RPCMulticast_UpdatePosRot_Implementation(FVector Pos, FRotator Rot)
{
	if (Role == ROLE_Authority)
		return;

	RootComponent->SetWorldLocationAndRotation(Pos, Rot, false);
}

