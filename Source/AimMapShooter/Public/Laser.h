// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AutomaticRifle.h"
#include "Laser.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class ASoldierCharacter;
class AAutomaticRifle;
class UPointLightComponent;

UCLASS()
class AIMMAPSHOOTER_API ALaser : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALaser();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp2;

	FName LaserSocket;

	AAutomaticRifle* Rifle;

	ASoldierCharacter* SoldierChar;

	void StartLaser();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UPointLightComponent* PointLight;

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void NotifyActorBeginOverlap(AActor* OtherActor);
	void NotifyActorEndOverlap(AActor* OtherActor);

	void DestroyOnUse();
	FTimerHandle DestroyTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Laser")
	float LengthOfLaser;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	UPROPERTY(EditDefaultsOnly, Category = "Laser")
	double ThickOfLaser;



	//UPROPERTY(EditDefaultsOnly, Category = "Player")
	//TSubclassOf<PointLight> PointLightClass;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool IsPickedUp;
};
