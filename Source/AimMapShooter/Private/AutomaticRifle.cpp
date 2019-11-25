// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "SoldierCharacter.h"
#include "TimerManager.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include "AutomaticRifle.h"

// Sets default values
AAutomaticRifle::AAutomaticRifle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraSocket = "CameraSocket";

	SkelMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkelMesh"));
	RootComponent = SkelMeshComp;
	

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SkelMeshComp, CameraSocket);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(SkelMeshComp);
	

	MuzzleSocket = "MuzzleSocket";
	LineSocket = "LineSocket";
	ScopeSocket = "ScopeSocket";
	GripSocket = "GripSocket";

	CurrentState = EWeaponState::Idle;

	CurrentAmmo = 0;
	CurrentAmmoInClip = 0;

	RateOfFire = 600;
	BaseDamage = 20.0f;
	BulletSpread = 2.0f;
	BulletSpreadZooming = 0.5f;
	BulletSpreadGrip = 1.0f;

}



void AAutomaticRifle::UseAmmo()
{
	CurrentAmmoInClip--;
	CurrentAmmo--;

}

// Called when the game starts or when spawned
void AAutomaticRifle::BeginPlay()
{
	Super::BeginPlay();

	if (WeaponConfig.InitialClips > 0)
	{
		CurrentAmmoInClip = WeaponConfig.AmmoPerClip;
		CurrentAmmo = WeaponConfig.AmmoPerClip * WeaponConfig.InitialClips;
	}

	WeaponConfig.TimeBetweenShots = 60 / RateOfFire;
	
}

// Called every frame
void AAutomaticRifle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AAutomaticRifle::StartFire()
{

	float FirstDelay = FMath::Max(LastFireTime + WeaponConfig.TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AAutomaticRifle::Fire, WeaponConfig.TimeBetweenShots, true, FirstDelay);
	Fire();
}

void AAutomaticRifle::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void AAutomaticRifle::Fire()
{
	if (CurrentAmmoInClip > 0 && CurrentState!=EWeaponState::Reloading)
	{
		CurrentState = EWeaponState::Firing;


		ASoldierCharacter* SoldierChar = Cast<ASoldierCharacter>(GetOwner());
		if (SoldierChar->IsZooming == true)
		{
			AActor* MyOwner = GetOwner();
			if (MyOwner)
			{
				FHitResult Hit;
				FVector StartLocation = SkelMeshComp->GetSocketLocation(LineSocket);
				FRotator Rotation = SkelMeshComp->GetSocketRotation(LineSocket);
				FVector ShotDirection = Rotation.Vector();
				float HalfRad = FMath::DegreesToRadians(BulletSpreadZooming);
				ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
				FVector EndLocation = StartLocation + (ShotDirection * 10000);

				FCollisionQueryParams QueryParams;
				QueryParams.AddIgnoredActor(MyOwner);
				QueryParams.AddIgnoredActor(this);

				if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECollisionChannel::ECC_Pawn, QueryParams))
				{

					//DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 1.0f, 0, 1.0f);

					if (ImpactEffect)
					{
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
					}
					//*Applying damage*//
					AActor* HitActor = Hit.GetActor();
					float ActualDamage = BaseDamage;
					UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);
				}
				UseAmmo();
				PlayFireEffects(EndLocation);
			}
		}
		else
		{
			AActor* MyOwner = GetOwner();
			if (MyOwner)
			{
				FHitResult Hit;
				FVector StartLocation = SkelMeshComp->GetSocketLocation(MuzzleSocket);
				FRotator Rotation = SkelMeshComp->GetSocketRotation(MuzzleSocket);
				FVector ShotDirection = Rotation.Vector();
				if (SoldierChar->isGripAttached == true)
				{
					BulletSpread = BulletSpreadGrip;
				}
				float HalfRad = FMath::DegreesToRadians(BulletSpread);
				ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
				FVector EndLocation = StartLocation + (ShotDirection * 10000);
				FCollisionQueryParams QueryParams;
				QueryParams.AddIgnoredActor(MyOwner);
				QueryParams.AddIgnoredActor(this);

				if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECollisionChannel::ECC_Pawn, QueryParams))
				{
					//DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::White, false, 1.0f, 0, 1.0f);

					if (ImpactEffect)
					{
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
					}
					//*Applying damage*//
					AActor* HitActor = Hit.GetActor();
					float ActualDamage = BaseDamage;
					UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage,ShotDirection,Hit,MyOwner->GetInstigatorController(), MyOwner,DamageType);

				}
				UseAmmo();
				PlayFireEffects(EndLocation);
			}
		}

		LastFireTime = GetWorld()->TimeSeconds;
	}

	//*Sound when no ammo in clip*//

	if (CurrentAmmoInClip == 0 && CurrentState!=EWeaponState::Reloading)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), NoAmmoSound, GetActorLocation());
	}
	
}


void AAutomaticRifle::PlayFireEffects(FVector EndLocation)
{
	FVector MuzzleLocation = SkelMeshComp->GetSocketLocation(MuzzleSocket);

	if (TracerEffect)
	{
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter("Target", EndLocation);
		}
	}

	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleEffect, MuzzleLocation);
	}
	if (ShootSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ShootSound, GetActorLocation());
	}
	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
		ASoldierCharacter* SoldierChar = Cast<ASoldierCharacter>(GetOwner());
		if (PC && SoldierChar->IsZooming == true)
		{
			PC->ClientPlayCameraShake(CameShakeZoomClass);
		}
		else
		{
			PC->ClientPlayCameraShake(CameShakeHipClass);
		}
	}
	
}
void AAutomaticRifle::StartReload()
{
	CurrentState = EWeaponState::Reloading;

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ReloadSound, GetActorLocation());

	GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &AAutomaticRifle::ReloadWeapon, 2.5f, false);

}

void AAutomaticRifle::StopReload()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_ReloadWeapon);
}

void AAutomaticRifle::ReloadWeapon()
{

	int32 ClipDelta = FMath::Min(WeaponConfig.AmmoPerClip - CurrentAmmoInClip, CurrentAmmo - CurrentAmmoInClip);
	if (ClipDelta > 0)
	{
		CurrentAmmoInClip += ClipDelta;
	}

	CurrentState = EWeaponState::Idle;

	UE_LOG(LogTemp, Warning, TEXT("Reloaded"));
	//LastReloadTime = GetWorld()->TimeSeconds;
}

