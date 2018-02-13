// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterMulti.h"
#include "FindNearestPlayer.h"
#include "UndeadAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UFindNearestPlayer::UFindNearestPlayer(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	NodeName = "Find Nearest Player";
}


void UFindNearestPlayer::ReceiveTick(AActor* OwnerActor, float DeltaSeconds)
{
	UE_LOG(LogTemp, Warning, TEXT("In the iterator"));
}

void UFindNearestPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AUndeadAIController* AI = Cast<AUndeadAIController>(OwnerComp.GetAIOwner());
	FVector AILocation = AI->GetPawn()->GetActorLocation();
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	AShooterCharacter* target = nullptr;

	for (TActorIterator<AShooterCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (!ActorItr->IsDead())
		{
			FVector playerLocation = ActorItr->GetActorLocation();

			if (ActorItr->IsDead())
				continue;

			if (target == nullptr)
			{
				target = *ActorItr;
			}

			if (FVector::Dist(AILocation, playerLocation) < 250.f)
			{
				target = *ActorItr;
			}

			else
				continue;

		}
	}
		BlackboardComponent->SetValueAsObject(FName("Player"), Cast<ACharacter>(target));
}

