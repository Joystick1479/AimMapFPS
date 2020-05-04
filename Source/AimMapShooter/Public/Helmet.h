// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Helmet.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class AIMMAPSHOOTER_API AHelmet : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHelmet();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void NotifyActorBeginOverlap(AActor* OtherActor);
	void NotifyActorEndOverlap(AActor* OtherActor);


	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	void DestroyOnUse();
	FTimerHandle DestroyTimer;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool IsPickedUp;
};
