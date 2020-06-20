// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grip.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class AIMMAPSHOOTER_API AGrip : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrip();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;

	void NotifyActorBeginOverlap(AActor* OtherActor);
	void NotifyActorEndOverlap(AActor* OtherActor);

	void DestroyOnUse();
	FTimerHandle DestroyTimer;

	bool IsPickedUp;

public:	

	bool GetIfPickeditem();
	void SetIfPickedItem(bool PickedUp);
	UStaticMeshComponent* GetMeshComponent();
	

};
