// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterMultiCubeActor.generated.h"

UCLASS()
class SHOOTERMULTI_API AShooterMultiCubeActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShooterMultiCubeActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Server, Reliable, WithValidation)
		void RPCServer_UpdatePosRot(FVector Pos, FRotator Rot);
	void RPCServer_UpdatePosRot_Implementation(FVector Pos, FRotator Rot);
	bool RPCServer_UpdatePosRot_Validate(FVector Pos, FRotator Rot);
	
	UFUNCTION(NetMulticast, Reliable)
		void RPCMulticast_UpdatePosRot(FVector Pos, FRotator Rot);
	void RPCMulticast_UpdatePosRot_Implementation(FVector Pos, FRotator Rot);

};
