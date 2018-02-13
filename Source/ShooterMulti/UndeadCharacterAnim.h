// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimInstance.h"
#include "UndeadCharacterAnim.generated.h"

class AUndeadCharacter;

/**
 * 
 */
UCLASS()
class SHOOTERMULTI_API UUndeadCharacterAnim : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient, EditAnywhere, BlueprintReadWrite)
	FVector2D Direction;

	UPROPERTY(Transient, EditAnywhere, BlueprintReadWrite)
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources|Montages")
	UAnimMontage* PunchMontage;

	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
	UFUNCTION()
	void AnimNotify_PunchHit(UAnimNotify* Notify);

private:
	AUndeadCharacter* UndeadCharacter;

};
