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
class AGrip;
class USpringArmComponent;
class AHelmet;
class AHeadset;
class ALaser;

namespace ELaserState
{
	enum Type
	{
		Idle,
		Start
	};
}

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

	ELaserState::Type LaserState;
	
	UPROPERTY(Replicated)
	AAutomaticRifle* AutomaticRifle;

	AHoloScope* HoloScope;

	AGrip* Grip;

	AHeadset* Headset;

	ALaser* Laser;

	AHelmet* Helmet;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPickUpItem();
	
	void LineTraceItem();

	//*Vaulting stuff//*
	void Vault();
	FVector WallLocation;
	FVector WallNormal;
	FVector WallHight;
	FVector NextWallHight;
	bool isAllowClimbing;
	bool isWallThick;
	bool isAbleToVault;
	bool isObjectTooHigh;

	UPROPERTY(EditDefaultsOnly, Category = "Vaulting")
	float MaxHeightForVault;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AAutomaticRifle> StarterWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AHoloScope> HoloClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AGrip> GripClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AHelmet> HelmetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AHeadset> HeadsetClass;


	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ALaser> LaserClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ZoomingTime;

	//Character Movement//
	void MoveForward(float Value);
	void MoveRight(float Value);
	void BeginCrouch();
	void EndCrouch();

	void StartFire();
	void StopFire();

	void SprintOn();
	void SprintOff();

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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	UCameraComponent* CameraComp;

	void TurnOnLaser();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	USpringArmComponent* SpringArm;

	FName HeadSocket;
	FName ArmSocket;
	FName WeaponSocket;
	FName HelmetSocket;
	FName HeadsetSocket;
	

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Zoom")
	bool IsZooming;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sprint")
	bool IsSprinting;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Zoom")
	bool IsSingleFire;

	//* Bool if we can pickup Rifle *//
	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bRiflePickUp;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bHoloPickUp;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bGripPickUp;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bHelmetPickUp;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bHeadsetPickUp;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bLaserPickUp;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool isLaserAttached;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool isHeadsetAttached;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool isGripAttached;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool isHoloAttached;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool isHelmetAttached;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool isLaserON;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	int32 SoldierCurrentAmmoInClip;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	int32 SoldierCurrentAmmo;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	int32 SoldierCurrentClips;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
