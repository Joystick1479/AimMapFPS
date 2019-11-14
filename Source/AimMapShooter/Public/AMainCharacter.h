// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AMainCharacter.generated.h"

class UCameraComponent;
class AAutomaticRifle;
class USkeletalMeshComponent;

UCLASS()
class AIMMAPSHOOTER_API AAMainCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAMainCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USkeletalMeshComponent* SkelMesh;

	//Weapons stuff, spawning, sockets//
	UPROPERTY(VisibleDefaultsOnly, Category = "Sockets")
	FName WeaponSocket;

	UPROPERTY(VisibleDefaultsOnly, Category = "Sockets")
	FName HeadSocket;

	UPROPERTY(VisibleDefaultsOnly, Category = "Sockets")
	FName CameraSocket;

	AAutomaticRifle* AutomaticRifle;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AAutomaticRifle> StarterWeaponClass;

	//Character Movement//
	void MoveForward(float Value);
	void MoveRight(float Value);
	void BeginCrouch();
	void EndCrouch();

	UFUNCTION()
	void ZoomIn();

	UFUNCTION()
	void ZoomOut();


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};