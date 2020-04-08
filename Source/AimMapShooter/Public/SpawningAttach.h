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
	AHoloScope* Holo;

	UPROPERTY()
	AHelmet* Helmet;

	UPROPERTY()
	AHeadset* Headset;

	UPROPERTY()
	ALaser* Laser;

	UPROPERTY()
	AGrip* Grip;

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

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "ParticlesEffects")
	UParticleSystem* ParticleSystem;


private:


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
