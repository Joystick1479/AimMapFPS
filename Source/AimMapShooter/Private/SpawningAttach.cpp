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
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);


}

// Called when the game starts or when spawned
void ASpawningAttach::BeginPlay()
{
	Super::BeginPlay();


	Actors.Push(Holo);
	Actors.Push(Headset);
	Actors.Push(Helmet);
	Actors.Push(Grip);
	Actors.Push(Laser);

	SpawnRandom();
}

void ASpawningAttach::SpawnRandom()
{
	if (Role < ROLE_Authority)
	{
		ServerSpawnRandom();
		return;
	}
	int32 range = Actors.Num();
	UE_LOG(LogTemp, Warning, TEXT("Numbers: %i"), range);
	int32 randomNumber = FMath::RandRange(0, range);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	UE_LOG(LogTemp, Warning, TEXT("RandomNumbers: %i"), randomNumber);

	switch (randomNumber) {
	case 1:
		UE_LOG(LogTemp, Warning, TEXT("HoloSpawn"));
		Holo = GetWorld()->SpawnActor<AHoloScope>(HoloClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		UGameplayStatics::SpawnEmitterAtLocation(this, ParticleSystem, GetActorLocation());
		break;
	case 2:
		UE_LOG(LogTemp, Warning, TEXT("Headset"));
		Helmet = GetWorld()->SpawnActor<AHelmet>(HelmetClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		UGameplayStatics::SpawnEmitterAtLocation(this, ParticleSystem, GetActorLocation());
		break;
	case 3:
		UE_LOG(LogTemp, Warning, TEXT("Helmet"));
		Headset = GetWorld()->SpawnActor<AHeadset>(HeadsetClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		UGameplayStatics::SpawnEmitterAtLocation(this, ParticleSystem, GetActorLocation());
		break;
	case 4:
		UE_LOG(LogTemp, Warning, TEXT("Grip"));
		Laser = GetWorld()->SpawnActor<ALaser>(LaserClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		UGameplayStatics::SpawnEmitterAtLocation(this, ParticleSystem, GetActorLocation());
		break;
	case 5:
		UE_LOG(LogTemp, Warning, TEXT("Laser"));
		Grip = GetWorld()->SpawnActor<AGrip>(GripClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		UGameplayStatics::SpawnEmitterAtLocation(this, ParticleSystem, GetActorLocation());
		break;
	case 0:
		UE_LOG(LogTemp, Warning, TEXT("NoAttachment, destroy particle"));
		break;
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

