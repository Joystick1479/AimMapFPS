// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SoldierCharacter.generated.h"

class UCameraComponent;
class AAutomaticRifle;
class USkeletalMeshComponent;
class UHealthComponent;
class AHoloScope;

namespace ECharacterState
{
	enum Type
	{
		Idle,
		Firing
	};
}

namespace EHoldingWeapon
{
	enum Type
	{
		None,
		A4,
		Sniper,
	};
}

namespace EHoldingAttachment
{
	enum Type
	{
		None,
		Holo,
		Grip,
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

	EHoldingWeapon::Type HoldingWeaponState;

	EHoldingAttachment::Type HoldingAttachmentState;
	
	AAutomaticRifle* AutomaticRifle;

	AHoloScope* HoloScope;

	void LineTraceItem();

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AAutomaticRifle> StarterWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AHoloScope> HoloClass;

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

	void PickUp();

	UPROPERTY(EditDefaultsOnly, Category = "LineTrace")
	float MaxUseDistance;


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

	//* Bool if we can pickup Rifle *//
	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bRiflePickUp;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bHoloPickUp;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
