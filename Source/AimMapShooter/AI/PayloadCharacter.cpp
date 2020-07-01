// Bartosz Jastrzebski


#include "PayloadCharacter.h"

#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "UnrealNetwork.h"

#include "TimerManager.h"

#include "Math/Quat.h"

#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NavigationSystem.h"

#include "Character/SoldierCharacter.h"
#include "AimMapGameModeBase.h"

// Sets default values
APayloadCharacter::APayloadCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp2"));
	PhysicsComp = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsComp"));

	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
	SetReplicates(true);

}

// Called when the game starts or when spawned
void APayloadCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APayloadCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	PayloadMove(DeltaTime);

}

void APayloadCharacter::PayloadMove(float DeltaTime)
{
	Super::Tick(DeltaTime);

	///*** IF GAME OVER, STOP THE PAYLOAD ***///

	//ABlueEndgame* BlueEnd = Cast<ABlueEndgame>(GetOwner());
	//if (BlueEnd)
	//{
	//	if (BlueEnd->BlueWins == true)
	//	{
	//		ShouldPush = false;
	//	}
	//}
	//ARedEndgame* RedEnd = Cast<ARedEndgame>(GetOwner());
	//if (BlueEnd)
	//{
	//	if (RedEnd->RedWins == true)
	//	{
	//		ShouldPush = false;
	//	}
	//}


	


	//if (Role < ROLE_Authority)
	//{
	//	ServerPayloadMove(DeltaTime);
	//}

	//**MOVING PAYLOAD**//

	/*TArray<AActor*> Characters;
	UGameplayStatics::GetAllActorsOfClass(this, SoldierCharacterClass, Characters);
	if (Characters.Num() > 1)
	{

		AActor* FirstCharacter = Characters[0];
		AActor* SecondCharacter = Characters[1];
		if (FirstCharacter && SecondCharacter)
		{
			if (this->IsOverlappingActor(FirstCharacter) && this->IsOverlappingActor(SecondCharacter))
			{
				AudioComp->Deactivate();
				ForceToPush = 0;
				ShouldPush = false;

				OnePlayerPushing = false;
				ContestedPushing = true;
			}
			else if (this->IsOverlappingActor(FirstCharacter))
			{
				AudioComp->Activate();
				ForceToPush = 50;
				ShouldPush = true;


				OnePlayerPushing = true;
				ContestedPushing = false;
			}
			else if (this->IsOverlappingActor(SecondCharacter))
			{
				AudioComp->Activate();
				ForceToPush = -50;
				ShouldPush = true;

				OnePlayerPushing = true;
				ContestedPushing = false;
			}
			else
			{
				AudioComp->Deactivate();
				OnePlayerPushing = false;
				ContestedPushing = false;
			}
		}
	}

	TArray<AActor*> RedEndGame;
	UGameplayStatics::GetAllActorsOfClass(this, RedEndgameClass, RedEndGame);

	for (int i = 0; i < RedEndGame.Num(); i++)
	{
		ARedEndgame* RedObjective = Cast<ARedEndgame>(RedEndGame[i]);
		if (RedObjective)
		{
			if (RedObjective->RedWins == true)
			{
				ShouldPush = false;
			}
		}
	}
	TArray<AActor*> BlueEndGame;
	UGameplayStatics::GetAllActorsOfClass(this, BlueEndgameClass, BlueEndGame);

	for (int i = 0; i < BlueEndGame.Num(); i++)
	{
		ABlueEndgame* BlueObjective = Cast<ABlueEndgame>(BlueEndGame[i]);
		if (BlueObjective)
		{
			if (BlueObjective->BlueWins == true)
			{
				ShouldPush = false;
			}
		}
	}

	/// PUSH LOGIC ///
	if (ShouldPush == true)
	{
		FVector MovePayload = GetActorForwardVector() * ForceToPush * DeltaTime;
		AddActorWorldOffset(MovePayload);
	}
*/
}

// Called to bind functionality to input
void APayloadCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
void APayloadCharacter::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	
}
void APayloadCharacter::NotifyActorEndOverlap(AActor * OtherActor)
{
	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{
		bShouldPush = false;
	}
}

void APayloadCharacter::ServerPayloadMove_Implementation(float DeltaTime)
{
	PayloadMove(DeltaTime);
}
bool APayloadCharacter::ServerPayloadMove_Validate(float DeltaTime)
{
	return true;
}
void APayloadCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//This function tells us how we want to replicate things//
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APayloadCharacter, bShouldPush);
	DOREPLIFETIME(APayloadCharacter, ForceToPush);
	DOREPLIFETIME(APayloadCharacter, Translation);
	DOREPLIFETIME(APayloadCharacter, bOnePlayerPushing);
	DOREPLIFETIME(APayloadCharacter, bContestedPushing);


}
