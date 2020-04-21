// Bartosz Jastrzebski


#include "AI_Animal_Controller.h"

#include "AI_Animal_FOX.h"
#include "SoldierCharacter.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "Engine/TargetPoint.h"

#include "Math/UnrealMathUtility.h"

#include "TimerManager.h"

#include <stack>

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

	index2 = -1;
	
}

void AAI_Animal_Controller::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AAI_Animal_FOX* Fox = Cast<AAI_Animal_FOX>(GetPawn());
	if (Fox)
	{
		if (Fox->IsAttacking == true)
		{
			//GetWorldTimerManager().ClearTimer(TimerHandle);
			//GetWorldTimerManager().SetTimer(TestTimerHandle, this, &AAI_Animal_Controller::GoToRandomWaypoint, 0.01f, false);
		}
	}

}

AAI_Animal_FOX * AAI_Animal_Controller::GetControlledFox() const
{
	return Cast<AAI_Animal_FOX>(GetPawn());
}

ATargetPoint * AAI_Animal_Controller::GetRandomWaypoint()
{



	index = FMath::RandRange(0, Waypoints.Num() - 1);
	dupa.push(index);


	if (!dupa.empty())
	{
		if (dupa.top() == index2)
		{
			GetRandomWaypoint();
			dupa.pop();
		}
	}


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
			//GetWorldTimerManager().ClearTimer(TimerHandle);
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
					//GetWorldTimerManager().ClearTimer(TimerHandle);

					FVector SoldierLocation = SoldierChar->GetActorLocation() - CorrectingVectorForSoldierHeight;
					FRotator LookAtSoldier = UKismetMathLibrary::FindLookAtRotation(GetPawn()->GetActorLocation(), SoldierLocation);
					GetPawn()->SetActorRotation(LookAtSoldier);
					IsMoving = true;
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
	if (IsRunning)
	{
		randnomNumber = 1;
	}

	GetWorldTimerManager().SetTimer(TimerHandle, this, &AAI_Animal_Controller::GoToRandomWaypoint, randnomNumber, false);

	/*AAI_Animal_FOX* Fox = Cast<AAI_Animal_FOX>(GetPawn());
	if (Fox)
	{
		if (Fox->IsAttacking == true)
		{
			randnomNumber = 1;
			UE_LOG(LogTemp, Warning, TEXT("FOX ATAKUUJEEE"));
			GetWorldTimerManager().SetTimer(TestTimerHandle, this, &AAI_Animal_Controller::GoToRandomWaypoint, randnomNumber, false);
		}
		else
		{
			GetWorldTimerManager().SetTimer(TimerHandle, this, &AAI_Animal_Controller::GoToRandomWaypoint, randnomNumber, false);
		}
	}*/


}
