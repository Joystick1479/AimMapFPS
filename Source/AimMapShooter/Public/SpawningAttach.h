// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawningAttach.generated.h"

class AHelmet;
class AHeadset;
class AGrip;
class AHoloScope;
class ALaser;
class AMagazine;
class UParticleSystem;

UCLASS()
class AIMMAPSHOOTER_API ASpawningAttach : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawningAttach();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void SpawnRandom();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSpawnRandom();

	TArray<AActor*> Actors;

	UPROPERTY()
	AHoloScope* Holo = nullptr;

	UPROPERTY()
	AHelmet* Helmet = nullptr;

	UPROPERTY()
	AHeadset* Headset = nullptr;

	UPROPERTY()
	ALaser* Laser = nullptr;

	UPROPERTY()
	AGrip* Grip = nullptr;

	UPROPERTY()
	AMagazine* Magazine = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AHoloScope> HoloClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AHelmet> HelmetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AHeadset> HeadsetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ALaser> LaserClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AGrip> GripClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AMagazine> MagazineClass;

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "ParticlesEffects")
	UParticleSystem* ParticleSystem;


private:


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
