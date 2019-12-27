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
class UCapsuleComponent;
class UUserWidget;
class USoundCue;

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
		Walking,
		Firing,
		Reloading
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
	////***Replication stuff***///
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPickUpItem();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReload();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerZoomIn();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerZoomOut();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSprintOn();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSprintOff();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerVault();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerResetTimerVault();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerBeginCrouch();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEndCrouch();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShowingPickUpHud();

	/////*** DISPLAYING HUD ****////
	void ShowingPickUpHud();
	
	void LineTraceItem();

	//*Vaulting stuff//*
	void Vault();
	void ResetVaultTimer();
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	FVector WallLocation;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	FVector WallNormal;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	FVector WallHight;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	FVector NextWallHight;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	bool isAllowClimbing;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	bool isWallThick;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	bool isAbleToVault;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	bool isObjectTooHigh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	FTimerHandle TimerHandle_Vault;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Animation")
	UAnimMontage* ClimbAnim;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Animation")
	UAnimMontage* VaultAnim;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vaulting")
	bool GoVault;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vaulting")
	bool GoClimb;

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Vaulting")
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
	FTimerHandle ReloadTimer;
	void StopReload();

	void ZoomIn();
	void ZoomOut();


	void PickUp();

	void OnDeath();

	UPROPERTY(EditDefaultsOnly, Category = "LineTrace")
	float MaxUseDistance;


	UPROPERTY(Replicated,BlueprintReadOnly, Category = "Player")
	bool bDied;

	UFUNCTION()
	void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComp;

	/////*SOUNDS WHEN PICK UP OBJECTS***////

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* RiflePickUp;


	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* ItemsPickUp;
	

public:	
	/// Variable for sprinting ///
	float MaxWalkSpeed;


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
	

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated,Category = "Zoom")
	bool IsZooming;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Sprint")
	bool IsSprinting;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Zoom")
	bool IsSingleFire;

	///Bool for crouching///
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Crouch")
	bool IsCrouching;


	///Bool for reloading///
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Reloading")
	bool IsReloading;

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

	UPROPERTY(EditDefaultsOnly, Replicated,Category = "Player")
	TSubclassOf<UUserWidget> wPickUp;

	UPROPERTY(Replicated)
	UUserWidget* wPickUpvar;


	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Player")
	TSubclassOf<UUserWidget> wAmmoCount;

	UPROPERTY(Replicated)
	UUserWidget* wAmmoCountvar;

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Player")
	TSubclassOf<UUserWidget> wHealthIndicator;

	UPROPERTY(Replicated)
	UUserWidget* wHealthIndicatorvar;

	UPROPERTY(BlueprintReadOnly, Replicated,Category = "Player")
	bool bRemoveHud;

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
