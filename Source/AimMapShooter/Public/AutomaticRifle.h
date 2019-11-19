// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AutomaticRifle.generated.h"

class USkeletalMeshComponent;
class UCameraComponent;
class UParticleSystem;
class USoundCue;
class ASoldierCharacter;

namespace EWeaponState
{
	enum Type
	{
		Idle,
		Firing,
		Reloading,
		Equipping,
	};
}

USTRUCT()
struct FWeaponData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 MaxAmmo;


	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 AmmoPerClip;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 InitialClips;

	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float TimeBetweenShots;

	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float TimeBetweenReload;

	FWeaponData()
	{
		MaxAmmo = 100;
		AmmoPerClip = 20;
		InitialClips = 4;
		TimeBetweenShots = 0.2f;
		TimeBetweenReload = 2.0f;
	}
};

UCLASS()
class AIMMAPSHOOTER_API AAutomaticRifle : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAutomaticRifle();
	
	enum class EAmmoType
	{
		EStandardAmmo,
		EBpAmmo,
		EBtAmmo,
	};


	ASoldierCharacter* SoldierChar;


	void UseAmmo();

	void Fire();
	void StartReload();
	void StopReload();
	void ReloadWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FName MuzzleSocket;
	FName CameraSocket;

	//Start location of holographic sight//
	FName LineSocket;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
	float BulletSpread;

	EWeaponState::Type CurrentState;


	/** weapon data */
	UPROPERTY(EditDefaultsOnly, Category = Config)
	FWeaponData WeaponConfig;

	UPROPERTY(Transient)
	int32 CurrentAmmo;

	UPROPERTY(Transient)
	int32 CurrentAmmoInClip;


	FTimerHandle TimerHandle_TimeBetweenShots;
	FTimerHandle TimerHandle_StopReload;
	FTimerHandle TimerHandle_ReloadWeapon;

	float LastFireTime;
	float LastReloadTime;

	//* Bullets per minute fired*//
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;


	//*Particle effects*//

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* ImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	USoundCue* ShootSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	USoundCue* ReloadSound;

	void PlayFireEffects(FVector EndLocation);


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USkeletalMeshComponent* SkelMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UCameraComponent* Camera;


	void StartFire();

	void StopFire();
};
