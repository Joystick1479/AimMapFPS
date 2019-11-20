// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SoldierCharacter.generated.h"

class UCameraComponent;
class AAutomaticRifle;
class USkeletalMeshComponent;
class UHealthComponent;

namespace ECharacterState
{
	enum Type
	{
		Idle,
		Firing
	};
}

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

	ECharacterState::Type CharacterState;
	
	AAutomaticRifle* AutomaticRifle;


	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AAutomaticRifle> StarterWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ZoomingTime;

	//Character Movement//
	void MoveForward(float Value);
	void MoveRight(float Value);
	void BeginCrouch();
	void EndCrouch();

	void StartFire();
	void StopFire();

	void FireMode();

	void Reload();

	void ZoomIn();
	void ZoomOut();


	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bDied;

	UFUNCTION()
	void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComp;

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	UCameraComponent* CameraComp;

	FName HeadSocket;
	FName WeaponSocket;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Zoom")
	bool IsZooming;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Zoom")
	bool IsSingleFire;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
