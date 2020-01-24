// Fill out your copyright notice in the Description page of Project Settings.
#include "AutomaticRifle.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "SoldierCharacter.h"
#include "TimerManager.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "AimMapShooter.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Laser.h"
#include "Math/Vector.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

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
	LaserSocket = "LaserSocket";
	LaserSocketEnd = "LaserSocketEnd";

	CurrentState = EWeaponState::Idle;

	CurrentAmmo = 0;
	CurrentAmmoInClip = 0;
	CurrentAmountOfClips = 0;

	RateOfFire = 600;
	BaseDamage = 20.0f;
	BulletSpread = 2.0f;
	BulletSpreadZooming = 0.5f;
	BulletSpreadGrip = 1.0f;

	//*Multiplayer repliaction **//
	SetReplicates(true);
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
	


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
		WeaponConfig.InitialClips--; ///* Minus one clip when we pick up the gun because it's already loaded *///
		CurrentAmountOfClips = WeaponConfig.InitialClips;
	}

	WeaponConfig.TimeBetweenShots = 60 / RateOfFire;
	
}

// Called every frame
void AAutomaticRifle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//void AAutomaticRifle::NotifyActorBeginOverlap(AActor* OtherActor)
//{
//	Super::NotifyActorBeginOverlap(OtherActor);
//
//	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
//	if (SoldierCharacter)
//	{
//		SoldierCharacter->bRiflePickUp = true;
//		SphereComp->ToggleActive();
//	}
//}
//void AAutomaticRifle::NotifyActorEndOverlap(AActor* OtherActor)
//{
//	Super::NotifyActorEndOverlap(OtherActor);
//
//	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
//	if (SoldierCharacter)
//	{
//		SoldierCharacter->bRiflePickUp = false;
//
//	}
//}
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

void AAutomaticRifle::ServerFire_Implementation()
{
	Fire();
}
bool AAutomaticRifle::ServerFire_Validate()
{
	return true;
}
void AAutomaticRifle::ServerStartReload_Implementation()
{
	StartReload();
}
bool AAutomaticRifle::ServerStartReload_Validate()
{
	return true;
}
void AAutomaticRifle::OnRep_HitScanTrace()
{
	// Play cosmetic FX//
	PlayFireEffects(HitScanTrace.TraceTo);

	PlayImpactEffects(HitScanTrace.TraceTo);
}

void AAutomaticRifle::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}
	if (CurrentAmmoInClip > 0 && CurrentState != EWeaponState::Reloading)
	{
		CurrentState = EWeaponState::Firing;


		ASoldierCharacter* SoldierChar = Cast<ASoldierCharacter>(GetOwner());
		ALaser* Laser = Cast<ALaser>(GetOwner());
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
				QueryParams.bReturnPhysicalMaterial = true;
				QueryParams.bTraceComplex = true;

				if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECollisionChannel::ECC_Pawn, QueryParams))
				{

					//DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 1.0f, 0, 1.0f);

					PlayImpactEffects(Hit.ImpactPoint);

					//*Applying damage*//
					AActor* HitActor = Hit.GetActor();
					EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
					float ActualDamage = BaseDamage;
					if (SurfaceType == SURFACE_HEAD)
					{
						UGameplayStatics::PlaySoundAtLocation(GetWorld(),HeadshotSound, GetActorLocation());
						ActualDamage *= 4.0f;
					}
					if (SurfaceType == SURFACE_CHEST)
					{
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, GetActorLocation());
						ActualDamage *= 2.0f;
					}
					if (SurfaceType == SURFACE_LEG)
					{
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, GetActorLocation());
						ActualDamage *= 0.5f;
					}
					if (SurfaceType == SURFACE_ARM)
					{
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, GetActorLocation());
						ActualDamage *= 1.5f;
					}
					if (SurfaceType == SURFACE_HELMET)
					{
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), HeadshotSound, GetActorLocation());
						ActualDamage *= 1.5f;
					}
					UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);
				}
				UseAmmo();
				PlayFireEffects(EndLocation);
				if (Role == ROLE_Authority)
				{
					HitScanTrace.TraceTo = EndLocation;
				}
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
				QueryParams.bReturnPhysicalMaterial = true;
				QueryParams.bTraceComplex = true;


				if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECollisionChannel::ECC_Pawn, QueryParams))
				{
					//DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::White, false, 1.0f, 0, 1.0f);

					PlayImpactEffects(Hit.ImpactPoint);

					//*Applying damage*//
					AActor* HitActor = Hit.GetActor();
					EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
					float ActualDamage = BaseDamage;
					if (SurfaceType == SURFACE_HEAD)
					{
						ActualDamage *= 4.0f;
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), HeadshotSound, GetActorLocation());
					}
					if (SurfaceType == SURFACE_CHEST)
					{
						ActualDamage *= 2.0f;
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, GetActorLocation());
					}
					if (SurfaceType == SURFACE_LEG)
					{
						ActualDamage *= 0.5f;
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, GetActorLocation());
					}
					if (SurfaceType == SURFACE_ARM)
					{
						ActualDamage *= 1.5f;
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, GetActorLocation());
					}
					if (SurfaceType == SURFACE_HELMET)
					{
						ActualDamage *= 1.5f;
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), HeadshotSound, GetActorLocation());
					}
					UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);

				}
				UseAmmo();
				PlayFireEffects(EndLocation);
				if (Role == ROLE_Authority)
				{
					HitScanTrace.TraceTo = EndLocation;
				}
			}
		}
	}
		LastFireTime = GetWorld()->TimeSeconds;

	//*Sound when no ammo in clip*//

	if (CurrentAmmoInClip == 0 && CurrentState!=EWeaponState::Reloading)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), NoAmmoSound, GetActorLocation());
	}
	
}
void AAutomaticRifle::PlayImpactEffects(FVector ImpactPoint)
{
	if (ImpactEffect)
	{
		FVector MuzzleLocation = SkelMeshComp->GetSocketLocation(MuzzleSocket);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, ImpactPoint, ShotDirection.Rotation());
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
		if (MyOwner->IsLocallyControlled())
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
	
}
void AAutomaticRifle::StartReload()
{
	if (Role < ROLE_Authority)
	{
		ServerStartReload();
		//return;
	}

	if (ReloadingState == EReloadingState::None && CurrentAmountOfClips>0)
	{
		ReloadingState = EReloadingState::Reloading;
		CurrentState = EWeaponState::Reloading;

		//UGameplayStatics::PlaySoundAtLocation(GetWorld(), ReloadSound, GetActorLocation());

		GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &AAutomaticRifle::ReloadWeapon, 2.5f, false);
	}

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
			CurrentAmountOfClips--;
		}

		CurrentState = EWeaponState::Idle;
		ReloadingState = EReloadingState::None;

		UE_LOG(LogTemp, Warning, TEXT("Reloaded"));
		//LastReloadTime = GetWorld()->TimeSeconds;
}
void AAutomaticRifle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//This function tells us how we want to replicate things//
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AAutomaticRifle, HitScanTrace,COND_SkipOwner);
}

