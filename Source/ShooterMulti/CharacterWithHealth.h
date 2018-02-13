// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

class AShooterMultiPlayerState;

#include "CharacterWithHealth.generated.h"

UCLASS()
class SHOOTERMULTI_API ACharacterWithHealth : public ACharacter
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Health, meta = (ClampMin = "0.0"))
	float MaxHealth = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Health, meta = (ClampMin = "0.0"))
	bool IsPlayerFriendly = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Health, meta = (ClampMin = "0.0"))
	float TimeBeforeDisapearing = 10.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Health, meta = (ClampMin = "0.0"))
	float DisapearingTime = 5.f;

	// Sets default values for this character's properties
	ACharacterWithHealth();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, Category = Health)
	float GetHealth();
	UFUNCTION(BlueprintCallable, Category = Health)
	virtual float TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser) override;
	UFUNCTION(BlueprintCallable, Category = Health)
	virtual float GainHealth(float GainAmount);
	UFUNCTION(BlueprintCallable, Category = Health)
	virtual void ResetHealth();

	UFUNCTION(BlueprintPure, Category = Health)
	bool IsDead();
	UFUNCTION(BlueprintCallable, Category = Health)
	virtual void Die();

	UFUNCTION(BlueprintCallable, Category = Health)
	virtual void StartDisapear();
	UFUNCTION()
	void UpdateDisapear();
	UFUNCTION(BlueprintCallable, Category = Health)
	virtual void FinishDisapear();
	UFUNCTION(BlueprintPure, Category = Health)
	FORCEINLINE bool IsDisapearing() { return Disapearing; }

	AShooterMultiPlayerState* GetShooterMultiPlayerState(const FString& playerStateName);

	void StartReaparing();
	void UpdateReaparing();
	void StopReaparing();

	// REPLICATED VARIABLESS

	UPROPERTY(Replicated)
		bool rep_hit = false;

	// REPLICATED FUNCTIONS

	virtual void GetLifetimeReplicatedProps(TArray <FLifetimeProperty> & OutLifetimeProps);

	// RPCServer

	UFUNCTION(Server, Reliable, WithValidation)
		void RPCServer_TakeDamage(float damage, float health);
	
	void RPCServer_TakeDamage_Implementation(float damage, float health);
	bool RPCServer_TakeDamage_Validate(float damage, float health);

	UFUNCTION(Server, Reliable, WithValidation)
		void RPCServer_DieNotify(const FString& causerName);
	
	void RPCServer_DieNotify_Implementation(const FString& causerName);
	bool RPCServer_DieNotify_Validate(const FString& causerName);

	// RPCMulticast

	UFUNCTION(NetMulticast, Reliable)
		void RPCMulticast_TakeDamage(float damage, float health);

	void RPCMulticast_TakeDamage_Implementation(float damage, float health);

	UFUNCTION(NetMulticast, Reliable)
		void RPCMulticast_UpdateKD(int kill, int death, const FString& playerStateName);

	void RPCMulticast_UpdateKD_Implementation(int kill, int death, const FString& playerStateName);

private:
	float Health;
	float DisapearTimer;
	bool Disapearing;
	bool reaparing;
	float ReaparingTimer;
	TArray<UMaterialInstanceDynamic*> DissolveMaterials;
	APlayerState* lastDamagerCauser;
};
