// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SoldierCharacter.generated.h"

class UCameraComponent;
class AAutomaticRifle;
class USkeletalMeshComponent;

UCLASS()
class AIMMAPSHOOTER_API ASoldierCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASoldierCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	AAutomaticRifle* AutomaticRifle;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USkeletalMeshComponent* SkelMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AAutomaticRifle> StarterWeaponClass;

	//Character Movement//
	void MoveForward(float Value);
	void MoveRight(float Value);
	void BeginCrouch();
	void EndCrouch();


	void ZoomIn();
	void ZoomOut();

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	UCameraComponent* CameraComp;

	FName HeadSocket;
	FName WeaponSocket;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
