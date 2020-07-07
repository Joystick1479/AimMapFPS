// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeaponClass.generated.h"

class USkeletalMeshComponent;
class UCameraComponent;
class UParticleSystem;
class USoundCue;
class ASoldierCharacter;
class USphereComponent;
class UCameraShake;
class ALaser;
class UAnimSequence;
class AHelmet;

//Contains information of a single hitscan weapon line trace//
USTRUCT()
struct  FHitScanTrace
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FVector_NetQuantize TraceTo;

};

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
namespace EReloadingState
{
	enum Type
	{
		None,
		Reloading,
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
	float TimeBetweenReload;

	FWeaponData()
	{
		MaxAmmo = 100;
		AmmoPerClip = 20;
		InitialClips = 2;
		TimeBetweenReload = 2.0f;
	}
};

UCLASS()
class AIMMAPSHOOTER_API ABaseWeaponClass : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseWeaponClass();

	UCameraComponent* GetCamera();
	USkeletalMeshComponent* GetSkelMeshComp();
	USphereComponent * GetSphereComp();

	FName GetMuzzleSocketName();
	FName GetScopeSocketName();
	FName GetGripSocketName();
	FName GetLaserSocketName();

	int32 GetCurrentAmmoInClip();
	int32 GetAllAmmo();
	int32 GetCurrentAmountOfClips();
	virtual void AddMagazine();
	virtual void Fire();

	UFUNCTION(BlueprintCallable)
	virtual void StartReload();
	virtual void StartFire();
	virtual void StopFire();

	EWeaponState::Type CurrentState;



protected:

	/** weapon data */
	UPROPERTY(EditDefaultsOnly, Category = Config)
	FWeaponData WeaponConfig;

	//Start location of holographic sight//
	FName LineSocket = "LineSocket";
	FName MuzzleSocket = "MuzzleSocket";
	FName CameraSocket = "CameraSocket";
	FName ScopeSocket = "ScopeSocket";
	FName GripSocket = "GripSocket";
	FName LaserSocket = "LaserSocket";
	FName LaserSocketEnd = "LaserSocketEnd";

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", Replicated)
	USkeletalMeshComponent* SkelMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USphereComponent* SphereComp;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<UCameraShake> CameShakeHipClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<UCameraShake> CameShakeZoomClass;


	UPROPERTY(EditDefaultsOnly, Category = "ActorHit")
	TSubclassOf<ASoldierCharacter> SoldierHit;

	//Weapon sway
	FRotator InitialWeaponRotation;
	FRotator FinalWeaponRotation;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon sway")
	float SmoothSway1;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon sway")
	float SmoothSway2;

	float DirectionSway;

	//Weapon clipping
	UPROPERTY(BlueprintReadWrite,EditDefaultsOnly, Category = "Weapon clipping")
	float DistanceToObject;

	UPROPERTY(BlueprintReadWrite,EditDefaultsOnly, Category = "Weapon clipping")
	float SmoothClipping;
	//
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
	float BulletSpreadGrip;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
	float BulletSpread;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
	float BulletSpreadZooming;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float TimeBetweenShots;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 CurrentAmmoInClip;

	UPROPERTY(BlueprintReadOnly, Category = "Ammo")
	int32 CurrentAmmo;

	UPROPERTY(BlueprintReadOnly, Category = "Ammo")
	int32 CurrentAmountOfClips;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	EReloadingState::Type ReloadingState;

	FTimerHandle TimerHandle_TimeBetweenShots;
	FTimerHandle TimerHandle_StopReload;
	FTimerHandle TimerHandle_ReloadWeapon;

	//* Bullets per minute fired*//
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;
	float LastFireTime;
	float LastReloadTime;

	virtual void BeginPlay() override;
	virtual void UseAmmo();
	virtual void ReloadWeapon();
	virtual void StopReload();
	virtual void CalculateWeaponSway();
	virtual void SetWeaponSway(float SwayDirection);
	virtual void SetbWeaponSway(bool bSway);
	virtual void LineTraceWeaponClipping();
	virtual void Tick(float DeltaTime) override;

	//*Particle effects*//
	virtual void PlayImpactEffects(FVector ImpactPoint);
	virtual void PlayFireEffects(FVector EndLocation);

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	USoundCue* NoAmmoSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	USoundCue* HeadshotSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	USoundCue* HelmetSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	USoundCue* HitSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	USoundCue* HitImpactSound;

	UPROPERTY(EditAnywhere, Category = "Animation reload")
	UAnimSequence* AnimSeqReload;


	///////****MUTLIPLAYER REPLICATION*****////
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartReload();

	UFUNCTION(Server, Reliable)
	void ServerHelmetHit(AHelmet*HitActor);

	UFUNCTION()
	void OnRep_HitScanTrace();

	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bFireAnimation;

	bool bStartWeaponSway;

	FRotator WeaponInitialRotation;

	AHelmet* Helmet;

	
};
