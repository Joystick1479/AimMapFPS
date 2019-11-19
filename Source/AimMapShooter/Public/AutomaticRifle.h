// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AutomaticRifle.generated.h"

class USkeletalMeshComponent;
class UCameraComponent;

UCLASS()
class AIMMAPSHOOTER_API AAutomaticRifle : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAutomaticRifle();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FName MuzzleSocket;
	FName CameraSocket;

	//Start location of holographic sight//
	FName LineSocket;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
	float BulletSpread;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USkeletalMeshComponent* SkelMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UCameraComponent* Camera;

	void Fire();
};
