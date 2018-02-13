// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "ShooterMultiGameMode.generated.h"


UENUM(BlueprintType)
enum class EShooterTeam : uint8
{
	Red,
	Blue,
	None
};

/**
 * 
 */
UCLASS()
class SHOOTERMULTI_API AShooterMultiGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:

	AShooterMultiGameMode();

	UPROPERTY(Transient, BlueprintReadOnly, Category = GameMode)
		EShooterTeam Team;

	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	virtual void Respawn(AController* pController);
	
	void JoinTeam(EShooterTeam team);
	void Quit();

private:
	FDelegateHandle TeamEventHandle;
};
