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
class AIMMAPSHOOTER_API AAutomaticRifle : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAutomaticRifle();

	UCameraComponent* GetCamera();
	USkeletalMeshComponent* GetSkelMeshComp();

	FName GetMuzzleSocketName();
	FName GetScopeSocketName();
	FName GetGripSocketName();
	FName GetLaserSocketName();

	int32 GetCurrentAmmoInClip();
	int32 GetAllAmmo();
	int32 GetCurrentAmountOfClips();
	void Fire();

	UFUNCTION(BlueprintCallable)
	void StartReload();
	void StartFire();
	void StopFire();

	EWeaponState::Type CurrentState;

protected:

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

	/** weapon data */
	UPROPERTY(EditDefaultsOnly, Category = Config)
	FWeaponData WeaponConfig;

	FTimerHandle TimerHandle_TimeBetweenShots;
	FTimerHandle TimerHandle_StopReload;
	FTimerHandle TimerHandle_ReloadWeapon;

	float LastFireTime;
	float LastReloadTime;

	//* Bullets per minute fired*//
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;

	virtual void BeginPlay() override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	void UseAmmo();
	void ReloadWeapon();
	void StopReload();

	//*Particle effects*//

	void PlayImpactEffects(FVector ImpactPoint);

	void PlayFireEffects(FVector EndLocation);

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

private:

	UPROPERTY(Replicated)
	ASoldierCharacter* SoldierChar;

	AHelmet* HelmetTest;

	
};
