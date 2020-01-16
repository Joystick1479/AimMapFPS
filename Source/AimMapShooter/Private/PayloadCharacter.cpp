// Bartosz Jastrzebski


#include "PayloadCharacter.h"

#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "UnrealNetwork.h"

#include "TimerManager.h"

#include "Math/Quat.h"

#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NavigationSystem.h"

#include "SoldierCharacter.h"
#include "BlueEndgame.h"
#include "RedEndgame.h"
#include "AimMapGameModeBase.h"

// Sets default values
APayloadCharacter::APayloadCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp2"));
	PhysicsComp = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsComp"));

	UCapsuleComponent* CapsuleComp = this->GetCapsuleComponent();
	if (CapsuleComp)
	{
		MeshComp->SetupAttachment(CapsuleComp);
		MeshComp2->SetupAttachment(CapsuleComp);
		PhysicsComp->SetupAttachment(CapsuleComp);
	}

	SetReplicates(true);

}

// Called when the game starts or when spawned
void APayloadCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<AActor*> Characters;
	UGameplayStatics::GetAllActorsOfClass(this, SoldierCharacterClass, Characters);
	UE_LOG(LogTemp, Warning, TEXT("Numer : %i"), Characters.Num());
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

	if (Role < ROLE_Authority)
	{
		ServerPayloadMove(DeltaTime);
	}

	TArray<AActor*> Characters;
	UGameplayStatics::GetAllActorsOfClass(this, SoldierCharacterClass, Characters);
	UE_LOG(LogTemp, Warning, TEXT("Numer : %i"), Characters.Num());
	if (Characters.Num() > 1)
	{

		AActor* Dupa = Characters[0];
		AActor* Dupa2 = Characters[1];
		if (Dupa && Dupa2)
		{
			if (this->IsOverlappingActor(Dupa) && this->IsOverlappingActor(Dupa2))
			{
				ForceToPush = 0;
				ShouldPush = false;
			}
			else if (this->IsOverlappingActor(Dupa))
			{
				ForceToPush = 50;
				ShouldPush = true;
			}
			else if (this->IsOverlappingActor(Dupa2))
			{
				ForceToPush = -50;
				ShouldPush = true;
			}
		}
	}


	if (ShouldPush == true)
	{
		FVector Translation = GetActorForwardVector() * ForceToPush * DeltaTime;
		AddActorWorldOffset(Translation);
	}


}

// Called to bind functionality to input
void APayloadCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
void APayloadCharacter::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{
		APlayerController* PC = Cast<APlayerController>(SoldierCharacter->GetController());
		if (PC)
		{
			FString FindName = "PlayerController_0";
			FString ProbaNazwy = UKismetSystemLibrary::GetObjectName(PC);

			if (FindName == ProbaNazwy)
			{
				//ShouldPush = true;

				//ForceToPush = 50;

			}
			else if ( FindName != ProbaNazwy)
			{
				//ShouldPush = true;

				//ForceToPush = -50;
				
			}

		}
	}

	//UCharacterMovementComponent* CharMovement = this->FindComponentByClass<UCharacterMovementComponent>();
	//if (CharMovement)
	//{
	//	CharMovement->Activate();
	//}

	//ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	//if (SoldierCharacter)
	//{

	//	APlayerController* PC = Cast<APlayerController>(SoldierCharacter->GetController());
	//	if (PC)
	//	{
	//			///CHECKING IF CONTROLLED PLAYER IS HOSTING ///
	//			FString FindName = "PlayerController_0";
	//			FString ProbaNazwy = UKismetSystemLibrary::GetObjectName(PC);
	//			UE_LOG(LogTemp, Warning, TEXT("Checkingname is : %s"), *ProbaNazwy);
	//			
	//			if ( FindName == ProbaNazwy)
	//			{
	//				TArray<AActor*> BlueEndGame;
	//				UGameplayStatics::GetAllActorsOfClass(this, BlueEndgameClass, BlueEndGame);

	//				if (BlueEndGame.Num() > 0)
	//				{
	//					//FVector BlueLocation = BlueEndGame[0]->GetActorLocation();
	//					//UE_LOG(LogTemp, Warning, TEXT("Vector is : %s"), *BlueLocation.ToString());
	//					//UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), BlueLocation);
	//					FRotator Rotation = FRotator(0.0f, 0.0f, 0.0f);
	//					FQuat Quater = Rotation.Quaternion();
	//					this->SetActorRotation(Quater);
	//					GetWorld()->GetTimerManager().SetTimer(BlueTimerHandle, this, &APayloadCharacter::BlueMove, 1.5f);

	//				}
	//			}
	//			else
	//			{
	//				TArray<AActor*> RedEndGame;
	//				UGameplayStatics::GetAllActorsOfClass(this, RedEndgameClass, RedEndGame);
	//				if (RedEndGame.Num() > 0)
	//				{	
	//					/*					FVector RedLocation = RedEndGame[0]->GetActorLocation();
	//										UE_LOG(LogTemp, Warning, TEXT("Vector is : %s"), *RedLocation.ToString());
	//										UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), RedLocation);*/
	//					FRotator Rotation2 = FRotator(0.0f, 180.0f, 0.0f);
	//					FQuat Quater2 = Rotation2.Quaternion();
	//					this->SetActorRotation(Quater2);

	//					GetWorld()->GetTimerManager().SetTimer(RedTimerHandle, this, &APayloadCharacter::RedMove, 1.5f);
	//					
	//				}
	//			}
	//	}
	//}


}
void APayloadCharacter::NotifyActorEndOverlap(AActor * OtherActor)
{

	/*ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{
		UCharacterMovementComponent* CharMovement = this->FindComponentByClass<UCharacterMovementComponent>();
		if (CharMovement)
		{
			CharMovement->Deactivate();

		}
		GetWorld()->GetTimerManager().ClearTimer(RedTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(BlueTimerHandle);
		

	}*/
	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{
		ShouldPush = false;
		
	}
}

void APayloadCharacter::RedMove()
{
	TArray<AActor*> RedEndGame;
	UGameplayStatics::GetAllActorsOfClass(this, RedEndgameClass, RedEndGame);
	if (RedEndGame.Num() > 0)
	{
		FVector RedLocation = RedEndGame[0]->GetActorLocation();
		UE_LOG(LogTemp, Warning, TEXT("Vector is : %s"), *RedLocation.ToString());
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), RedLocation);

		//GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APayloadCharacter::PhysicsTurn, 3.0f);

	}
}

void APayloadCharacter::BlueMove()
{
	TArray<AActor*> BlueEndGame;
	UGameplayStatics::GetAllActorsOfClass(this, BlueEndgameClass, BlueEndGame);
	if (BlueEndGame.Num() > 0)
	{
		FVector BlueLocation = BlueEndGame[0]->GetActorLocation();
		UE_LOG(LogTemp, Warning, TEXT("Vector is : %s"), *BlueLocation.ToString());
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), BlueLocation);

		//GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APayloadCharacter::PhysicsTurn, 3.0f);

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

	DOREPLIFETIME(APayloadCharacter, ShouldPush);
	DOREPLIFETIME(APayloadCharacter, ForceToPush);
	DOREPLIFETIME(APayloadCharacter, Translation);


}
