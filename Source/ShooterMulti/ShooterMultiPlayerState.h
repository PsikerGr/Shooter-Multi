// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ShooterMultiGameState.h"
#include "ShooterMultiPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERMULTI_API AShooterMultiPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Transient, Category = GameMode)
		int Kills;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Transient, Category = GameMode)
		int Deaths;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Transient, Category = GameMode)
		EShooterTeam Team;
	
	AShooterMultiPlayerState();
	virtual void GetLifetimeReplicatedProps(TArray <FLifetimeProperty> & OutLifetimeProps);
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	void AddKill();
	void AddDeath();
	void SetTeam(EShooterTeam pTeam);
	
private:
};
