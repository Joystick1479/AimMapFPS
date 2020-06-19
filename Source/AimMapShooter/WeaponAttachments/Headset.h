// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Headset.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class AIMMAPSHOOTER_API AHeadset : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHeadset();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	void NotifyActorBeginOverlap(AActor* OtherActor);
	void NotifyActorEndOverlap(AActor* OtherActor);

	void DestroyOnUse();
	FTimerHandle DestroyTimer;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool IsPickedUp;

};
