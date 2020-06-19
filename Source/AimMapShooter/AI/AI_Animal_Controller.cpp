// Bartosz Jastrzebski


#include "AI_Animal_Controller.h"

#include "AI/AI_Animal_FOX.h"
#include "Character/SoldierCharacter.h"

#include "GameFramework/CharacterMovementComponent.h" 

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "Engine/TargetPoint.h"

#include "Math/UnrealMathUtility.h"

#include "TimerManager.h"

#include "Net/UnrealNetwork.h"

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
	if (Role == ROLE_Authority)
	{
		GoToRandomWaypoint();
	}

	index2 = -1;

	CheckIfChargePlayer();
}

void AAI_Animal_Controller::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*if (IsRunning)
	{
		randomNumber = 0.35;
	}*/

	//CheckIfChargePlayer();


	///If Fox is attacking, charge player immediately

	//if (DoOnce == false)
	//{
	//	AAI_Animal_FOX* Fox = Cast<AAI_Animal_FOX>(GetPawn());
	//	if (Fox)
	//	{
	//		if (Fox->IsAttacking == true && Fox->bDied == false)
	//		{
	//			FTimerDelegate DelegateFunc = FTimerDelegate::CreateUObject(this, &AAI_Animal_Controller::FastAttack, Fox);
	//			GetWorldTimerManager().SetTimer(FastAttackTimer, DelegateFunc, 1.0f, false);
	//			DoOnce = true;
	//		}
	//		else
	//		{
	//			bAttacking = false;
	//			GetWorldTimerManager().ClearTimer(FastAttackTimer);
	//		}
	//	}
	//}


}
void AAI_Animal_Controller::CheckIfChargePlayer()
{

	if (IsRunning)
	{
		randomNumber = 0.35;
	}

	if (DoOnce == false)
	{
		AAI_Animal_FOX* Fox = Cast<AAI_Animal_FOX>(GetPawn());
		if (Fox)
		{
			if (Fox->IsAttacking == true && Fox->bDied == false)
			{
				FTimerDelegate DelegateFunc = FTimerDelegate::CreateUObject(this, &AAI_Animal_Controller::FastAttack, Fox);
				GetWorldTimerManager().SetTimer(FastAttackTimer, DelegateFunc, 1.0f, false);
				DoOnce = true;
			}
			else
			{
				bAttacking = false;
				GetWorldTimerManager().ClearTimer(FastAttackTimer);
			}
		}
	}

	GetWorldTimerManager().SetTimer(ChargeTimer, this, &AAI_Animal_Controller::CheckIfChargePlayer, 0.3f, false);
}
void AAI_Animal_Controller::FastAttack(AAI_Animal_FOX* Fox)
{

	bAttacking = true;

	///Fast charge player nearby
	UE_LOG(LogTemp, Warning, TEXT("Casting OK, Fox Is Attaking"));

	GetWorldTimerManager().ClearTimer(TimerHandle);

	auto MovementComponent = Fox->GetCharacterMovementComponent();
	if (MovementComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found movementcomponent"));
		MovementComponent->DisableMovement();
		MovementComponent->StopActiveMovement();
		MovementComponent->SetMovementMode(EMovementMode::MOVE_Walking);
	}

	GoToRandomWaypoint();
	DoOnce = false;
}

AAI_Animal_FOX * AAI_Animal_Controller::GetControlledFox() const
{
	return Cast<AAI_Animal_FOX>(GetPawn());
}

ATargetPoint * AAI_Animal_Controller::GetRandomWaypoint()
{
	///Don't repeat the same waypoint twice in a row.
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
	if (Role < ROLE_Authority)
	{
		Server_GoToRandomWaypoint();
	}
	//Rotate pawn to face waypoint while moving//
	AAI_Animal_FOX* Fox = Cast<AAI_Animal_FOX>(GetPawn());
	if (Fox)
	{
		if (Fox->IsAttacking == false)
		{
			//GetWorldTimerManager().ClearTimer(TimerHandle);
			auto Waypoint = GetRandomWaypoint();
			if (Waypoint)
			{
				FVector WaypointLocation = Waypoint->GetActorLocation();
				FRotator LookDirection = UKismetMathLibrary::FindLookAtRotation(GetPawn()->GetActorLocation(), WaypointLocation);
				GetPawn()->SetActorRotation(LookDirection);
				IsMoving = true;
				IsRunning = false;
				MoveToActor(Waypoint);
			}
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
					IsMoving = true;
					IsRunning = true;
					MoveToActor(SoldierChar);
				}
			}
		}
	}
}

void AAI_Animal_Controller::Server_GoToRandomWaypoint_Implementation()
{
	GoToRandomWaypoint();
}
bool AAI_Animal_Controller::Server_GoToRandomWaypoint_Validate()
{
	return true;
}

void AAI_Animal_Controller::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult & Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	IsMoving = false;

	randomNumber = FMath::RandRange(1, 5);

	GetWorldTimerManager().SetTimer(TimerHandle, this, &AAI_Animal_Controller::GoToRandomWaypoint, randomNumber, false);


}

void AAI_Animal_Controller::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//This function tells us how we want to replicate things//
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAI_Animal_Controller, IsMoving);
	DOREPLIFETIME(AAI_Animal_Controller, IsRunning);



}
