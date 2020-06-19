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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void NotifyActorBeginOverlap(AActor* OtherActor);
	void NotifyActorEndOverlap(AActor* OtherActor);

	void DestroyOnUse();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;

	FTimerHandle DestroyTimer;

	UPROPERTY(Replicated)
	int32 NumberOfHits;

	bool IsPickedUp;

public:	

	UStaticMeshComponent* GetStaticMeshComponent();

	int32 GetNumberOfHits();

	bool CheckIfPickedUp();

};
