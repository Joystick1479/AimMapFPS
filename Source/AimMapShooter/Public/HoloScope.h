// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HoloScope.generated.h"

class USkeletalMeshComponent;
class USphereComponent;

UCLASS()
class AIMMAPSHOOTER_API AHoloScope : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHoloScope();


	UPROPERTY(VisibleAnywhere, Category = "Components")
	USkeletalMeshComponent* SkelMeshComp;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	void NotifyActorBeginOverlap(AActor* OtherActor);
	void NotifyActorEndOverlap(AActor* OtherActor);


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;




};
