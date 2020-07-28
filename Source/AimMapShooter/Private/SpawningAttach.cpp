// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawningAttach.h"

#include "WeaponAttachments/HoloScope.h"
#include "WeaponAttachments/Helmet.h"
#include "WeaponAttachments/Headset.h"
#include "WeaponAttachments/Grip.h"
#include "WeaponAttachments/Laser.h"
#include "WeaponAttachments/Magazine.h"
#include "Character/SoldierCharacter.h"

#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"

#include "Net/UnrealNetwork.h"

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
			Holo->GetMesh3P()->bOnlyOwnerSee = false;

		}
	}
	if (randomNumber == 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("Helmet"));
		Helmet = GetWorld()->SpawnActor<AHelmet>(HelmetClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		if (Helmet)
		{
			Helmet->SetOwner(this);
			Helmet->GetMesh3P()->bOnlyOwnerSee = false;
		}
	}
	if (randomNumber == 3)
	{
		UE_LOG(LogTemp, Warning, TEXT("Headset"));
		Headset = GetWorld()->SpawnActor<AHeadset>(HeadsetClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		if (Headset)
		{
			Headset->SetOwner(this);
			Headset->GetMesh3P()->bOnlyOwnerSee = false;

		}
	}
	/*if (randomNumber == 4)
	{
		 UE_LOG(LogTemp, Warning, TEXT("Laser"));
		Laser = GetWorld()->SpawnActor<ALaser>(LaserClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		if (Laser)
		{
			Laser->SetOwner(this);
			Laser->GetMeshComponent()->bOnlyOwnerSee = false;

		}
	}*/
	if (randomNumber == 4)
	{
		UE_LOG(LogTemp, Warning, TEXT("Grip"));
		Grip = GetWorld()->SpawnActor<AGrip>(GripClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		if (Grip)
		{
			Grip->SetOwner(this);
			Grip->GetMesh3P()->bOnlyOwnerSee = false;
		}
	}
	if (randomNumber == 5)
	{
		UE_LOG(LogTemp, Warning, TEXT("Magazine"));
		Magazine = GetWorld()->SpawnActor<AMagazine>(MagazineClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		if (Magazine)
		{
			Magazine->SetOwner(this);
			Magazine->GetMesh3P()->bOnlyOwnerSee = false;
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
