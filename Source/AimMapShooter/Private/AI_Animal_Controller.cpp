// Bartosz Jastrzebski


#include "AI_Animal_Controller.h"

#include "AI_Animal_FOX.h"
#include "SoldierCharacter.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "Engine/TargetPoint.h"

#include "Math/UnrealMathUtility.h"

#include "TimerManager.h"

void AAI_Animal_Controller::BeginPlay()
{
	Super::BeginPlay();

	auto ControlledFox = GetControlledFox();
	if (ControlledFox)
	{
		UE_LOG(LogTemp, Warning, TEXT("AIController possesing %s"), *(ControlledFox->GetName()));
	}

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetPoint::StaticClass(), Waypoints);
	GoToRandomWaypoint();
}

AAI_Animal_FOX * AAI_Animal_Controller::GetControlledFox() const
{
	return Cast<AAI_Animal_FOX>(GetPawn());
}

ATargetPoint * AAI_Animal_Controller::GetRandomWaypoint()
{
	index = FMath::RandRange(0, Waypoints.Num() - 1);
	index2 = index;

	

	UE_LOG(LogTemp, Warning, TEXT("RandomWaypoint number is: %i"), index);

	return Cast<ATargetPoint>(Waypoints[index]);
}

void AAI_Animal_Controller::GoToRandomWaypoint()
{
	//Rotate pawn to face waypoint while moving//
	AAI_Animal_FOX* Fox = Cast<AAI_Animal_FOX>(GetPawn());
	if (Fox)
	{
		if (Fox->IsAttacking == false)
		{
			auto Waypoint = GetRandomWaypoint();
			FVector WaypointLocation = Waypoint->GetActorLocation();
			FRotator LookDirection = UKismetMathLibrary::FindLookAtRotation(GetPawn()->GetActorLocation(), WaypointLocation);
			GetPawn()->SetActorRotation(LookDirection);
			IsMoving = true;
			IsRunning = false;
			MoveToActor(Waypoint);
		}
		else if(Fox->IsAttacking == true)
		{
			///Charge,attack player
			TArray<AActor*> Target;
			UGameplayStatics::GetAllActorsOfClass(this, Soldier, Target);
			for (int i = 0; i < Target.Num(); i++)
			{
				ASoldierCharacter* SoldierChar = Cast<ASoldierCharacter>(Target[i]);
				if (SoldierChar)
				{
					FVector SoldierLocation = SoldierChar->GetActorLocation() - CorrectingVectorForSoldierHeight;
					FRotator LookAtSoldier = UKismetMathLibrary::FindLookAtRotation(GetPawn()->GetActorLocation(), SoldierLocation);
					GetPawn()->SetActorRotation(LookAtSoldier);
					IsRunning = true;
					MoveToActor(SoldierChar);
				}
			}
		}
	}
	

}

void AAI_Animal_Controller::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult & Result)
{
	Super::OnMoveCompleted(RequestID, Result);
	IsMoving = false;

	auto randnomNumber = FMath::RandRange(1, 5);
	UE_LOG(LogTemp, Warning, TEXT("Roll: %i"), randnomNumber);
	if (IsRunning)
	{
		randnomNumber = 1;
	}


	GetWorldTimerManager().SetTimer(TimerHandle, this, &AAI_Animal_Controller::GoToRandomWaypoint, randnomNumber, false);

}
