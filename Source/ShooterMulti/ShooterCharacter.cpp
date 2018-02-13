// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterMulti.h"
#include "ShooterCharacter.h"
#include "ShooterMultiPlayerState.h"

#include "ParticleDefinitions.h"
#include "HitDamage.h"
#include "WeaponUtility.h"
#include "ShooterCharacterAnim.h"

AShooterCharacter::FShooterEvent AShooterCharacter::DeathEvent;
AShooterCharacter::FTeamEvent AShooterCharacter::TeamEvent;

// Sets default values
AShooterCharacter::AShooterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//Create Punch Collision
	PunchCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PunchCollision"));
	PunchCollision->SetupAttachment(RootComponent);
	PunchCollision->SetRelativeLocation(FVector(80.f, 0.f, 20.f));
	PunchCollision->InitSphereRadius(20.f);
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	RunSpeed = GetCharacterMovement()->MaxWalkSpeed;

	SpringArm = FindComponentByClass<USpringArmComponent>();
	if (SpringArm)
	{
		CameraRestDistance = SpringArm->TargetArmLength;
		TargetSpringArmLength = CameraRestDistance;
	}

	Camera = FindComponentByClass<UCameraComponent>();
	if (Camera)
	{
		CameraRestFOV = Camera->FieldOfView;
		TargetCameraFOV = CameraRestFOV;
	}

	ResetHealth();

	CanSprint = false;
	Shooting = false;
	ShootTimer = 0.f;
	CurrentSpread = 0.f;
	Ammo = MaxAmmo;
	GotHit = false;
}

void AShooterCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if (SpringArm && SpringArm->TargetArmLength != TargetSpringArmLength)
	{
		SpringArm->TargetArmLength = FMath::Lerp(SpringArm->TargetArmLength, TargetSpringArmLength, .1f);
	}
	if (Camera && Camera->FieldOfView != TargetCameraFOV)
	{
		Camera->FieldOfView = FMath::Lerp(Camera->FieldOfView, TargetCameraFOV, .1f);
	}

	if (colorChanged)
	{
		AShooterMultiPlayerState* pState = Cast<AShooterMultiPlayerState>(PlayerState);
		pState->SetTeam(pTeam);
		colorChanged = false;
	}

	if (IsDead())
		return;

	//update spread
	float minSpread = (CurrentState == EShooterCharacterState::Aim) ? WeaponMinSpreadAim : WeaponMinSpreadWalk;
	CurrentSpread = FMath::Max(minSpread, CurrentSpread - WeaponSpreadRecoveryRate * DeltaTime);

	//manage shot
	ShootTimer += DeltaTime;
	if (Shooting && ShootTimer > FireRate)
	{
		ShootTimer = 0.f;

		TakeShot();
	}

	//manage punch
	if (CurrentState == EShooterCharacterState::Punch)
	{
		StateTimer += DeltaTime;

		if (StateTimer >= PunchDuration)
		{
			CurrentState = EShooterCharacterState::IdleRun;
		}
	}

	//manage shake
	float movementIntensity = GetLastMovementInputVector().Size();

	auto playerController = Cast<APlayerController>(GetController());

	if (playerController)
	{
		if (CurrentState == EShooterCharacterState::Sprint && SprintShake)
			playerController->ClientPlayCameraShake(SprintShake, movementIntensity);
		else if (RunShake)
		{
			playerController->ClientPlayCameraShake(RunShake, movementIntensity);
		}
		if (FMath::Abs(movementIntensity) < .02f)
		{
			if (RunShake)
				playerController->ClientStopCameraShake(RunShake, false);
			if (SprintShake)
				playerController->ClientStopCameraShake(SprintShake, false);
		}
	}
}

void AShooterCharacter::RPCMulticast_SetTeam_Implementation(EShooterTeam team)
{
	
	/*if (Role == ROLE_Authority)
		return;*/

	
	SetColorWithTeam(team);
	colorChanged = true;
}

void AShooterCharacter::RPCServer_SetTeam_Implementation(EShooterTeam team)
{

	SetColorWithTeam(team);
	colorChanged = true;

	RPCMulticast_SetTeam(team);
}

bool AShooterCharacter::RPCServer_SetTeam_Validate(EShooterTeam team)
{
	return true;
}

void AShooterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
	RPCServer_SendRotationInput(GetControlRotation());
}

void AShooterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
	RPCServer_SendRotationInput(GetControlRotation());
}

void AShooterCharacter::MoveForward(float Value)
{
	if (IsDead())
		return;

	CanSprint = Value > MinSprintMagnitude;

	if (CurrentState == EShooterCharacterState::Sprint && !CanSprint)
		EndSprint ();

	FRotator rotator = GetControlRotation();
	rotator.Pitch = 0.f;
	rotator.Roll = 0.f;
	AddMovementInput(Value * rotator.Vector());
}

void AShooterCharacter::MoveRight(float Value)
{
	if (CurrentState == EShooterCharacterState::Sprint || IsDead())
		return;

	FRotator rotator = GetControlRotation();
	rotator.Pitch = 0.f;
	rotator.Roll = 0.f;
	AddMovementInput(Value * rotator.RotateVector (FVector::RightVector));
}

void AShooterCharacter::StartJump()
{
	if (IsDead())
		return;

	if (Shooting)
		EndShoot();

	if (CanJump())
		Jump();
}

void AShooterCharacter::EndJump()
{
	StopJumping();
}

void AShooterCharacter::StartSprint()
{
	if ((CurrentState != EShooterCharacterState::IdleRun &&
		 CurrentState != EShooterCharacterState::Aim) ||
		!CanSprint ||
		IsDead())
		return;

	if (CurrentState == EShooterCharacterState::Aim)
		EndAim();

	if (Shooting)
		EndShoot();

	CurrentState = EShooterCharacterState::Sprint;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	RPCServer_ChangePlayerState(EShooterCharacterState::Sprint);
}

void AShooterCharacter::EndSprint()
{
	if (CurrentState != EShooterCharacterState::Sprint)
		return;

	CurrentState = EShooterCharacterState::IdleRun;
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	RPCServer_ChangePlayerState(EShooterCharacterState::IdleRun);
}

void AShooterCharacter::StartAim()
{
	if (CurrentState != EShooterCharacterState::IdleRun ||
		IsDead())
		return;

	CurrentState = EShooterCharacterState::Aim;
	GetCharacterMovement()->MaxWalkSpeed = AimWalkSpeed;
	TargetSpringArmLength = CameraAimDistance;
	TargetCameraFOV = CameraAimFOV;

	RPCServer_ChangePlayerState(EShooterCharacterState::Aim);
}

void AShooterCharacter::EndAim()
{
	if (CurrentState != EShooterCharacterState::Aim)
		return;

	CurrentState = EShooterCharacterState::IdleRun;
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	TargetSpringArmLength = CameraRestDistance;
	TargetCameraFOV = CameraRestFOV;

	RPCServer_ChangePlayerState(EShooterCharacterState::IdleRun);
}

int AShooterCharacter::GetAmmo()
{
	return Ammo;
}

EShooterTeam AShooterCharacter::GetTeam()
{
	return pTeam;
}

void AShooterCharacter::SetColorWithTeam(EShooterTeam cTeam)
{
	
	switch (cTeam)
	{
	case EShooterTeam::Red:
	{
		SetColor(FLinearColor::Red);
		pTeam = EShooterTeam::Red;
	}
	break;
	case EShooterTeam::Blue:
	{
		SetColor(FLinearColor::Blue);
		pTeam = EShooterTeam::Blue;
	}
	break;
	case EShooterTeam::None:
		SetColor(FLinearColor::Black);
		break;
	default:
		break;
	}
	
}

void AShooterCharacter::SetColor(FLinearColor cColor)
{
	UMaterialInstanceDynamic* DynamicMaterial = GetMesh()->CreateAndSetMaterialInstanceDynamic(0);
	DynamicMaterial->SetVectorParameterValue("BodyColor", cColor);
}

bool AShooterCharacter::IsShooting()
{
	return Shooting;
}

void AShooterCharacter::StartShoot()
{
	if (CurrentState != EShooterCharacterState::IdleRun &&
		CurrentState != EShooterCharacterState::Aim ||
		IsDead())
		return;

	Shooting = true;
}

void AShooterCharacter::EndShoot()
{
	Shooting = false;
}

void AShooterCharacter::Reload()
{
	if ((CurrentState != EShooterCharacterState::IdleRun &&
		 CurrentState != EShooterCharacterState::Aim) ||
		Ammo >= MaxAmmo ||
		IsDead())
		return;
	

	if (CurrentState == EShooterCharacterState::Aim)
		EndAim();

	if (Shooting)
		EndShoot();

	CurrentState = EShooterCharacterState::Reload;
	GetCharacterMovement()->MaxWalkSpeed = ReloadWalkSpeed;
	RPCServer_ChangePlayerState(EShooterCharacterState::Reload);
}

void AShooterCharacter::EndReload()
{
	if (CurrentState != EShooterCharacterState::Reload)
		return;

	CurrentState = EShooterCharacterState::IdleRun;
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	Ammo = MaxAmmo;
	RPCServer_ChangePlayerState(EShooterCharacterState::IdleRun);
}

void AShooterCharacter::AbortReload()
{
	if (CurrentState != EShooterCharacterState::Reload)
		return;

	CurrentState = EShooterCharacterState::IdleRun;
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	RPCServer_ChangePlayerState(EShooterCharacterState::IdleRun);
}

void AShooterCharacter::Punch()
{
	if (CurrentState == EShooterCharacterState::Aim ||
		IsDead())
		EndAim();

	if (CurrentState != EShooterCharacterState::IdleRun)
		return;

	CurrentState = EShooterCharacterState::Punch;
	StateTimer = 0.f;
	RPCServer_ChangePlayerState(EShooterCharacterState::Punch);
}

void AShooterCharacter::InflictPunch()
{
	TArray<struct FHitResult> outHits;

	FVector pos = PunchCollision->GetComponentLocation();

	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	params.bTraceComplex = true;

	GetWorld()->SweepMultiByObjectType(
		outHits, pos, pos, FQuat::Identity, 0,
		FCollisionShape::MakeSphere(PunchCollision->GetUnscaledSphereRadius()),
		params);

	TArray<ACharacterWithHealth*> hitActors;

	for (auto& hit : outHits)
	{
		ACharacterWithHealth* character = Cast<ACharacterWithHealth>(hit.Actor.Get());

		if (character && character->IsPlayerFriendly != IsPlayerFriendly && !hitActors.Contains(character))
		{
			FPointDamageEvent damageEvent = FPointDamageEvent(WeaponPunchDamages, hit, GetActorForwardVector(), TSubclassOf<UDamageType>(UHitDamage::StaticClass()));
			character->TakeDamage(WeaponPunchDamages, damageEvent, nullptr, this);
			hitActors.Add(character);
		}
	}
}

void AShooterCharacter::TakeShot()
{
	if (!CheckAmmo())
		return;


	FHitResult hitResult;

	if (Role == ROLE_AutonomousProxy)
	{
		FTransform meshTransform = Cast<UMeshComponent>(GetMesh()->GetChildComponent(0))->GetSocketTransform("MuzzleFlashSocket");
		RPCServer_FireShot(Camera->GetComponentTransform() , meshTransform, this);
		return;
	}

	FLaserWeaponData weaponData = CreateWeaponLaser();
	DisplayLaser(weaponData, hitResult);
	PlayWeaponFX(weaponData);
	ShakeCameraAfterShooting();
	CurrentSpread = FMath::Min(WeaponMaxSpread, CurrentSpread + WeaponSpreadPerShot);
	
	RPCMulticast_DisplayFireToClient(weaponData.LookTransform, weaponData.MuzzleTransform);
}

float AShooterCharacter::TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	if (DamageAmount == 0.0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Damage Amount is equal to 0"));
		return 0.0f;
	}
	if (CurrentState == EShooterCharacterState::Reload)
		AbortReload();

	float actualDamages = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (actualDamages > 0)
		GotHit = true;

	return actualDamages;
}

bool AShooterCharacter::ConsumeHitTrigger()
{
	if (GotHit)
	{
		GotHit = false;
		return true;
	}

	return false;
}

void AShooterCharacter::FinishDisapear()
{
	Super::FinishDisapear();

	AController* controller = GetController();
	Destroy();
	
	if (Role == ROLE_Authority)
	{
		AShooterMultiGameMode* gm = Cast<AShooterMultiGameMode>(GetWorld()->GetAuthGameMode());
		gm->Respawn(controller);
	}

}

void AShooterCharacter::Die()
{
	Super::Die();
	DeathEvent.Broadcast(this);
}

void AShooterCharacter::JoinTeam()
{
	UE_LOG(LogTemp, Warning, TEXT("Character is %s"), *this->GetName());
	
	AShooterMultiPlayerState* currentPlayerState = Cast<AShooterMultiPlayerState>(PlayerState);

	if (currentPlayerState != nullptr)
	{
		//AGameState::PlayerArray
		switch (currentPlayerState->Team)
		{
		case EShooterTeam::Red:
			teamColor = FLinearColor::Red;
			break;
		case EShooterTeam::Blue:
			teamColor = FLinearColor::Blue;
			break;

		default:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("This player haven't a team!"));
			break;
		}

		UMaterialInstanceDynamic* DynamicMaterial = GetMesh()->CreateAndSetMaterialInstanceDynamic(0);
		DynamicMaterial->SetVectorParameterValue("BodyColor", teamColor);

		const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EShooterTeam"), true);
		UE_LOG(LogTemp, Warning, TEXT("Team is %s"), *EnumPtr->GetNameByValue((uint8)currentPlayerState->Team).ToString());
	}


	TeamEvent.Broadcast(this);
}

bool AShooterCharacter::CheckAmmo()
{
	if (Ammo <= 0)
	{
		EndShoot();
		Reload();
		return false;
	}
	else
	{
		--Ammo;
		return true;
	}
}

FLaserWeaponData AShooterCharacter::CreateWeaponLaser()
{
	UMeshComponent* weaponMesh = Cast<UMeshComponent>(GetMesh()->GetChildComponent(0));

	FLaserWeaponData weaponData;
	weaponData.MuzzleTransform = weaponMesh->GetSocketTransform("MuzzleFlashSocket");
	weaponData.LookTransform = Camera->GetComponentTransform();
	weaponData.Damages = WeaponDamages;
	weaponData.Knockback = WeaponKnokback;
	weaponData.Spread = CurrentSpread;

	return weaponData;
}

FLaserWeaponData AShooterCharacter::CreateWeaponLaser(FTransform cameraTransform, FTransform weaponTransform)
{
	FLaserWeaponData weaponData;
	weaponData.MuzzleTransform = weaponTransform;
	weaponData.LookTransform = cameraTransform;
	weaponData.Damages = 0.0f;
	if (Role == ROLE_Authority)
		weaponData.Damages = WeaponDamages;
	weaponData.Knockback = WeaponKnokback;
	weaponData.Spread = CurrentSpread;

	return weaponData;
}

void AShooterCharacter::DisplayLaser(FLaserWeaponData weaponData, FHitResult hitResult, AActor* causer)
{
	if (causer == nullptr)
		causer = this;
	
	//make the beam visuals
	if (UWeaponUtility::ShootLaser(GetWorld(), causer, hitResult, weaponData))
	{
		//make impact decal
		UWeaponUtility::MakeImpactDecal(hitResult, ImpactDecalMat, .9f * ImpactDecalSize, 1.1f * ImpactDecalSize);

		//create impact particles
		UWeaponUtility::MakeImpactParticles(GetWorld(), ImpactParticle, hitResult, .66f);
	}

	UWeaponUtility::MakeLaserBeam(GetWorld(), weaponData.MuzzleTransform.GetLocation(), hitResult.ImpactPoint, BeamParticle, BeamIntensity, FLinearColor(1.f, 0.857892f, 0.036923f), BeamIntensityCurve);
}

void AShooterCharacter::PlayWeaponFX(FLaserWeaponData weaponData)
{
	UMeshComponent* weaponMesh = Cast<UMeshComponent>(GetMesh()->GetChildComponent(0));

	//play the shot sound
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ShotSound, weaponData.MuzzleTransform.GetLocation());
	
	//make muzzle smoke
	auto smokeParticle = UGameplayStatics::SpawnEmitterAttached(MuzzleSmokeParticle, weaponMesh, FName("MuzzleFlashSocket"));

	//play sound if ammo is empty
	if (Ammo == 0)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ShotEmptySound, GetActorLocation());
}

void AShooterCharacter::ShakeCameraAfterShooting()
{
	//apply shake
	auto playerController = Cast<APlayerController>(GetController());
	if (playerController && ShootShake)
		playerController->ClientPlayCameraShake(ShootShake);
}

	/* REPLICATE PROPS */

void AShooterCharacter::GetLifetimeReplicatedProps(TArray < FLifetimeProperty > & OutLifetimeProps)
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterCharacter, rep_currentControlRotation);
	DOREPLIFETIME(AShooterCharacter, CurrentState);
}

	/* RPC SERVER*/

void AShooterCharacter::RPCServer_FireShot_Implementation(FTransform cameraTransform, FTransform weaponTransform, const AShooterCharacter* shooter)
{
	FLaserWeaponData weaponData = CreateWeaponLaser(cameraTransform, weaponTransform);
	FHitResult hitResult;

	AActor* causer = Cast<AActor>(const_cast<AShooterCharacter*>(shooter));

	DisplayLaser(weaponData, hitResult, causer);
	PlayWeaponFX(weaponData);
	
	RPCMulticast_DisplayFireToClient(cameraTransform, weaponTransform);
}


bool AShooterCharacter::RPCServer_FireShot_Validate(FTransform cameraTransform, FTransform weaponTransform, const AShooterCharacter* shooter)
{
	return true;
}

void AShooterCharacter::RPCServer_SendRotationInput_Implementation(FRotator tmp_rotator)
{
	rep_currentControlRotation = tmp_rotator;
	RPCMulticast_SendRotator(tmp_rotator);
}

bool AShooterCharacter::RPCServer_SendRotationInput_Validate(FRotator tmp_rotator)
{
	return true;
}

void AShooterCharacter::RPCServer_ChangePlayerState_Implementation(EShooterCharacterState state)
{
	CurrentState = state;

	if (state == EShooterCharacterState::Sprint)
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	else if (state == EShooterCharacterState::IdleRun && GetCharacterMovement()->MaxWalkSpeed != RunSpeed)
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;

	RPCMulticast_ChangePlayerState(state);
}

bool AShooterCharacter::RPCServer_ChangePlayerState_Validate(EShooterCharacterState state)
{
	return true;
}

	/* RPC CLIENT */



	/* RPC MULTICAST */

void AShooterCharacter::RPCMulticast_DisplayFireToClient_Implementation(FTransform cameraTransform, FTransform weaponTransform)
{
	if (Role == ROLE_Authority)
		return;

	FLaserWeaponData weaponData = CreateWeaponLaser(cameraTransform, weaponTransform);
	FHitResult hitResult;

	DisplayLaser(weaponData, hitResult);
	PlayWeaponFX(weaponData);
	ShakeCameraAfterShooting();
	CurrentSpread = FMath::Min(WeaponMaxSpread, CurrentSpread + WeaponSpreadPerShot);
}

void AShooterCharacter::RPCMulticast_SendRotator_Implementation(FRotator newRotator)
{
	if (Role == ROLE_Authority)
		return;

	rep_currentControlRotation = newRotator;
}

void AShooterCharacter::RPCMulticast_ChangePlayerState_Implementation(EShooterCharacterState state)
{
	if (Role == ROLE_Authority)
		return;

	CurrentState = state;
	if (state == EShooterCharacterState::Sprint)
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	else if (state == EShooterCharacterState::IdleRun && GetCharacterMovement()->MaxWalkSpeed != RunSpeed)
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}
