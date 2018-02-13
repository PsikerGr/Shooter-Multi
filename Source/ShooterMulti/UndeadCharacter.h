// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CharacterWithHealth.h"
#include "UndeadCharacter.generated.h"

UCLASS()
class SHOOTERMULTI_API AUndeadCharacter : public ACharacterWithHealth
{
	GENERATED_BODY()

public:

	UPROPERTY(Category = UndeadCharacter, VisibleDefaultsOnly, BlueprintReadOnly)
	USphereComponent* PunchCollision;
	
	UPROPERTY(Category = UndeadCharacter, EditAnywhere, BlueprintReadWrite)
	float Damages = 10.f;

	UPROPERTY(Category = UndeadCharacter, EditAnywhere, BlueprintReadWrite)
	float PunchCooldown = 1.5f;

	UPROPERTY(Category = UndeadCharacter, EditAnywhere, BlueprintReadWrite)
	float PunchKnockback = 300000.f;

	DECLARE_EVENT_OneParam(ACharacterWithHealth, FUndeadEvent, AUndeadCharacter*)
	static FUndeadEvent PunchEvent;
	static FUndeadEvent DeathEvent;

	// Sets default values for this character's properties
	AUndeadCharacter();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	bool Punch();

	UFUNCTION(NetMultiCast, Reliable)
		void RPCMultiCast_DoPunch();

	UFUNCTION(NetMultiCast, Reliable)
		void RPCMultiCast_Die();

	bool CanPunch();

	UFUNCTION(BlueprintPure, Category = Character)
	bool HasPunched();

	UFUNCTION(BlueprintCallable, Category = Character)
	void InflictPunch();

	virtual void Die() override;

private:
	bool hasPunched;
	float punchTimer;

};
