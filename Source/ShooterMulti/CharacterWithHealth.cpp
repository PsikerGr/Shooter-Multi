// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterMulti.h"
#include "CharacterWithHealth.h"
#include "ShooterCharacter.h"
#include "ShooterMultiPlayerState.h"


// Sets default values
ACharacterWithHealth::ACharacterWithHealth()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACharacterWithHealth::BeginPlay()
{
	Super::BeginPlay();

	Disapearing = false;
	ResetHealth();

	for (auto uskelMesh : GetComponentsByClass(USkeletalMeshComponent::StaticClass()))
	{
		auto skelMesh = Cast<USkeletalMeshComponent>(uskelMesh);
		auto mats = skelMesh->GetMaterials();

		for (int i = 0; i < mats.Num(); ++i)
		{
			auto newMat = UMaterialInstanceDynamic::Create(mats[i], this);
			skelMesh->SetMaterial(i, newMat);
			DissolveMaterials.Add(newMat);
		}
	}

	for (auto ustaticMesh : GetComponentsByClass(UStaticMeshComponent::StaticClass()))
	{
		auto staticMesh = Cast<UStaticMeshComponent>(ustaticMesh);
		auto mats = staticMesh->GetMaterials();

		for (int i = 0; i < mats.Num(); ++i)
		{
			auto newMat = UMaterialInstanceDynamic::Create(mats[i]->GetMaterial(), this);
			staticMesh->SetMaterial(i, newMat);
			DissolveMaterials.Add(newMat);
		}
	}

	for (auto mat : DissolveMaterials)
	{
		mat->SetScalarParameterValue(FName(TEXT("DissolveAmmount")), 0.f);
	}

	bReplicates = true;
	bReplicateMovement = true;
}

void ACharacterWithHealth::Tick(float DeltaTime)
{
	if (IsDead())
	{
		DisapearTimer += DeltaTime;

		if (!Disapearing)
			StartDisapear();

		if (Disapearing)
			UpdateDisapear();
	}
	rep_hit = false;
}

float ACharacterWithHealth::GetHealth()
{
	return Health;
}

float ACharacterWithHealth::TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser)
{
	float doneDamages = 0.f;

	if (DamageAmount == 0.0f)
	{
		return 0.0f;
	}

	if (DamageCauser && !rep_hit)
	{
		if (!DamageCauser->GetName().Contains("Undead"))
		{
			AShooterCharacter* damageCauserCharacter = Cast<AShooterCharacter>(DamageCauser);
			lastDamagerCauser = damageCauserCharacter->PlayerState;
		}
	}

	const FPointDamageEvent* pointDamage = nullptr;

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
		pointDamage = (FPointDamageEvent*)&DamageEvent;

	if (!IsDead() && !rep_hit)
	{
		if (pointDamage && pointDamage->HitInfo.PhysMaterial.Get())
			doneDamages = DamageAmount * pointDamage->HitInfo.PhysMaterial->DestructibleDamageThresholdScale;
		else
			doneDamages = DamageAmount;

		RPCServer_TakeDamage(doneDamages, FMath::Max(0.f, Health - doneDamages));
	}

	return doneDamages;
}

float ACharacterWithHealth::GainHealth(float GainAmount)
{
	if (!IsDead())
	{
		if (Health + GainAmount > MaxHealth)
		{
			GainAmount = MaxHealth - Health;
			Health = MaxHealth;
		}
		else
		{
			Health = FMath::Min(MaxHealth, Health + GainAmount);
		}

		return GainAmount;
	}

	return 0.f;
}

void ACharacterWithHealth::ResetHealth()
{
	Health = MaxHealth;
}

bool ACharacterWithHealth::IsDead()
{
	return Health <= 0.f;
}

void ACharacterWithHealth::Die()
{
	if (Role == ROLE_Authority)
	{
		if (lastDamagerCauser)
			RPCServer_DieNotify(lastDamagerCauser->GetName());
		else
			RPCServer_DieNotify("None");
	}

	auto capsule = GetCapsuleComponent();
	capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	capsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

	auto mesh = GetMesh();

	for (auto aSkeletalMesh : GetComponentsByClass(USkeletalMeshComponent::StaticClass()))
	{
		USkeletalMeshComponent* skeletalMesh = Cast<USkeletalMeshComponent>(aSkeletalMesh);
		skeletalMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		skeletalMesh->bPauseAnims = true;
		skeletalMesh->SetCollisionObjectType(ECC_GameTraceChannel1);
		skeletalMesh->SetCollisionResponseToAllChannels(ECR_Block);
		skeletalMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		skeletalMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		skeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		skeletalMesh->SetSimulatePhysics(true);
	}
}

void ACharacterWithHealth::StartDisapear()
{
	DisapearTimer = 0.f;
	Disapearing = true;
}

void ACharacterWithHealth::UpdateDisapear()
{
	if (DisapearTimer > DisapearingTime)
		return FinishDisapear();

	GetMesh()->SetScalarParameterValueOnMaterials(FName(TEXT("DissolveAmmount")), DisapearTimer / DisapearingTime);
}

void ACharacterWithHealth::FinishDisapear()
{
	Disapearing = false;
	Health = MaxHealth;
	reaparing = true;
	ReaparingTimer = DisapearTimer;
}

void ACharacterWithHealth::StartReaparing()
{
	ReaparingTimer = 0.0f;
	reaparing = true;
}

void ACharacterWithHealth::UpdateReaparing()
{
	if (ReaparingTimer > DisapearTimer)
		return StopReaparing();

	GetMesh()->SetScalarParameterValueOnMaterials(FName(TEXT("DissolveAmmount")), ReaparingTimer / DisapearingTime);
}

void ACharacterWithHealth::StopReaparing()
{
	reaparing = false;
}

AShooterMultiPlayerState* ACharacterWithHealth::GetShooterMultiPlayerState(const FString& playerStateName)
{
	TArray<APlayerState*> playerStateArray = GetWorld()->GetGameState()->PlayerArray;
	for (uint32 idx = 0; idx != playerStateArray.Max(); idx++)
	{
		if (playerStateArray[idx]->GetName() == playerStateName)
			return Cast<AShooterMultiPlayerState>(playerStateArray[idx]);
	}
	return nullptr;
}

	/* REPLICATED FUNCTIONS */

void ACharacterWithHealth::GetLifetimeReplicatedProps(TArray <FLifetimeProperty> & OutLifetimeProps)
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACharacterWithHealth, rep_hit);
}

	/* RPC SERVER */

void ACharacterWithHealth::RPCServer_TakeDamage_Implementation(float damage, float health)
{
	Health = health;
	if (Health <= 0)
		Die();

	RPCMulticast_TakeDamage(damage, health);
}

bool ACharacterWithHealth::RPCServer_TakeDamage_Validate(float damage, float health)
{
	return true;
}

void ACharacterWithHealth::RPCServer_DieNotify_Implementation(const FString& causerName)
{
	if (causerName == "None")
	{
		AShooterMultiPlayerState* pState = Cast<AShooterMultiPlayerState>(PlayerState);
		pState->AddDeath();
		RPCMulticast_UpdateKD(pState->Kills, pState->Deaths, pState->GetName());
		return;
	}
	AShooterMultiPlayerState* pState = GetShooterMultiPlayerState(causerName);
	
		pState->AddKill();
		RPCMulticast_UpdateKD(pState->Kills, pState->Deaths, causerName);

		if (Cast<AActor>(this)->GetName().Contains("Undead"))
			return;
		pState = Cast<AShooterMultiPlayerState>(PlayerState);
		pState->AddDeath();
		RPCMulticast_UpdateKD(pState->Kills, pState->Deaths, pState->GetName());
}

bool ACharacterWithHealth::RPCServer_DieNotify_Validate(const FString& causerName)
{
	return true;
}

	/* RPC MULTICAST */

void ACharacterWithHealth::RPCMulticast_TakeDamage_Implementation(float damage, float health)
{
	if (Role == ROLE_Authority)
		return;

	rep_hit = true;

	Health = health;
	if (Health <= 0)
	{
		Die();
	}
}

void ACharacterWithHealth::RPCMulticast_UpdateKD_Implementation(int kill, int death, const FString& playerStateName)
{
	if (Role == ROLE_Authority || Role == ROLE_SimulatedProxy)
		return;

	AShooterMultiPlayerState* pState = GetShooterMultiPlayerState(playerStateName);

	if (pState->Kills != kill)
		pState->Kills = kill;
		//pState->AddKill();

	if (pState->Deaths < death)
		pState->Deaths = death;
		//pState->AddDeath();
}