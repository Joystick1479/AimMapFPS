// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawningAttach.h"

#include "HoloScope.h"
#include "Helmet.h"
#include "Headset.h"
#include "Grip.h"
#include "Laser.h"
#include "SoldierCharacter.h"

#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASpawningAttach::ASpawningAttach()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SetReplicates(true);


}

// Called when the game starts or when spawned
void ASpawningAttach::BeginPlay()
{
	Super::BeginPlay();


	//Actors.Push(Holo);
	//Actors.Push(Headset);
	//Actors.Push(Helmet);
	//Actors.Push(Grip);
	//Actors.Push(Laser);

	SpawnRandom();
}

void ASpawningAttach::SpawnRandom()
{
	UGameplayStatics::SpawnEmitterAtLocation(this, ParticleSystem, GetActorLocation());

	if (Role < ROLE_Authority)
	{
		ServerSpawnRandom();
		return;
	}
	/*int32 range = Actors.Num();
	UE_LOG(LogTemp, Warning, TEXT("Numbers: %i"), range);
	int32 randomNumber = FMath::RandRange(0, range);*/
	int32 range = 5;
	int32 randomNumber = FMath::RandRange(0, range);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	UE_LOG(LogTemp, Warning, TEXT("RandomNumbers: %i"), randomNumber);

	if (randomNumber == 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("HoloSpawn"));
		Holo =GetWorld()->SpawnActor<AHoloScope>(HoloClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		if (Holo)
		{
			Holo->SetOwner(this);
			Holo->SkelMeshComp->bOnlyOwnerSee = false;

		}
	}
	if (randomNumber == 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("Helmet"));
		Helmet = GetWorld()->SpawnActor<AHelmet>(HelmetClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		if (Helmet)
		{
			Helmet->SetOwner(this);
			Helmet->MeshComp->bOnlyOwnerSee = false;

		}
	}
	if (randomNumber == 3)
	{
		UE_LOG(LogTemp, Warning, TEXT("Headset"));
		Headset = GetWorld()->SpawnActor<AHeadset>(HeadsetClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		if (Headset)
		{
			Headset->SetOwner(this);
			Headset->MeshComp->bOnlyOwnerSee = false;

		}
	}
	if (randomNumber == 4)
	{
		 UE_LOG(LogTemp, Warning, TEXT("Laser"));
		Laser = GetWorld()->SpawnActor<ALaser>(LaserClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		if (Laser)
		{
			Laser->SetOwner(this);
			Laser->MeshComp->bOnlyOwnerSee = false;

		}
	}
	if (randomNumber == 5)
	{
		UE_LOG(LogTemp, Warning, TEXT("Grip"));
		Grip = GetWorld()->SpawnActor<AGrip>(GripClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		if (Grip)
		{
			Grip->SetOwner(this);
			Grip->MeshComp->bOnlyOwnerSee = false;
		}
	}
	

}

void ASpawningAttach::ServerSpawnRandom_Implementation()
{
	SpawnRandom();
}
bool ASpawningAttach::ServerSpawnRandom_Validate()
{
	return true;
}

// Called every frame
void ASpawningAttach::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void ASpawningAttach::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//This function tells us how we want to replicate things//
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpawningAttach, ParticleSystem);
}
