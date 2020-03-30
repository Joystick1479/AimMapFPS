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
	
	enum class EAmmoType
	{
		EStandardAmmo,
		EBpAmmo,
		EBtAmmo,
	};

	UPROPERTY(Replicated)
	ASoldierCharacter* SoldierChar;


	void UseAmmo();

	void Fire();
	///////****MUTLIPLAYER REPLICATION*****////

	UFUNCTION(Server,Reliable, WithValidation)
	void ServerFire();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartReload();

	UFUNCTION(BlueprintCallable)
	void StartReload();
	void StopReload();
	void ReloadWeapon();



	FName MuzzleSocket;
	FName CameraSocket;
	FName ScopeSocket;
	FName GripSocket;
	FName LaserSocket;
	FName LaserSocketEnd;

	ALaser* Laser;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 CurrentAmmoInClip;


	UPROPERTY(BlueprintReadOnly, Category = "Ammo")
	int32 CurrentAmmo;

	UPROPERTY(BlueprintReadOnly, Category = "Ammo")
	int32 CurrentAmountOfClips;

	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float TimeBetweenShots;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	//void NotifyActorBeginOverlap(AActor* OtherActor);
	//void NotifyActorEndOverlap(AActor* OtherActor);

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AAutomaticRifle> StarterWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<UCameraShake> CameShakeHipClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<UCameraShake> CameShakeZoomClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USphereComponent* SphereComp;

	//Start location of holographic sight//
	FName LineSocket;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
	float BulletSpreadGrip;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
	float BulletSpread;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
	float BulletSpreadZooming;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	EWeaponState::Type CurrentState;
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

	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();


	//*Particle effects*//

	void PlayImpactEffects(FVector ImpactPoint);

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
	USoundCue* HitSound;

	void PlayFireEffects(FVector EndLocation);



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", Replicated)
	USkeletalMeshComponent* SkelMeshComp;

	UPROPERTY(EditAnywhere, Category = "Animation reload")
	UAnimSequence* AnimSeqReload;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UCameraComponent* Camera;

	void StartFire();

	void StopFire();
};
