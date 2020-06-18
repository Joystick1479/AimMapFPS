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
#include "Helmet.h"
#include "Math/Vector.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimSequence.h"

// Sets default values
AAutomaticRifle::AAutomaticRifle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CameraSocket = "CameraSocket";

	SkelMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkelMesh"));
	RootComponent = SkelMeshComp;
	

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SkelMeshComp, CameraSocket);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(SkelMeshComp);

	MuzzleSocket = "MuzzleFlash";
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

	TimeBetweenShots = 60 / RateOfFire;
	
}

USkeletalMeshComponent* AAutomaticRifle::GetSkelMeshComp()
{
	return this->SkelMeshComp;
}

UCameraComponent* AAutomaticRifle::GetCamera()
{
	return this->Camera;
}

FName AAutomaticRifle::GetMuzzleSocketName()
{
	return this->MuzzleSocket;
}

FName AAutomaticRifle::GetScopeSocketName()
{
	return this->ScopeSocket;
}

FName AAutomaticRifle::GetGripSocketName()
{
	return this->GripSocket;
}

FName AAutomaticRifle::GetLaserSocketName()
{
	return this->LaserSocket;
}

int32 AAutomaticRifle::GetCurrentAmmoInClip()
{
	return CurrentAmmoInClip;
}

int32 AAutomaticRifle::GetAllAmmo()
{
	return CurrentAmmo;
}

int32 AAutomaticRifle::GetCurrentAmountOfClips()
{
	return CurrentAmountOfClips;
}



void AAutomaticRifle::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{
		SoldierCharacter->bRiflePickUp = true;
		SphereComp->ToggleActive();
	}
}
void AAutomaticRifle::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{
		SoldierCharacter->bRiflePickUp = false;

	}
}
void AAutomaticRifle::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds,0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AAutomaticRifle::Fire, TimeBetweenShots, true, FirstDelay);
}

void AAutomaticRifle::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
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
			///RECOIL
			float PitchRandomVal = UKismetMathLibrary::RandomFloatInRange(-0.1, -0.5);
			SoldierChar->AddControllerPitchInput(PitchRandomVal);
			float YawRandomVal = UKismetMathLibrary::RandomFloatInRange(-0.2, 0.2);
			SoldierChar->AddControllerYawInput(YawRandomVal);


			AActor* MyOwner = GetOwner();
			if (MyOwner)
			{
				FHitResult Hit;
				FVector StartLocation = SkelMeshComp->GetSocketLocation(LineSocket);
				FRotator Rotation = SkelMeshComp->GetSocketRotation(LineSocket);
				FVector ShotDirection = Rotation.Vector();
				float HalfRad = FMath::DegreesToRadians(BulletSpreadZooming);
				ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
				FVector EndLocation = StartLocation + (ShotDirection * 100000);

				FCollisionQueryParams QueryParams;
				QueryParams.AddIgnoredActor(MyOwner);
				QueryParams.AddIgnoredActor(this);
				QueryParams.bReturnPhysicalMaterial = true;
				QueryParams.bTraceComplex = true;

				if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECollisionChannel::ECC_Destructible, QueryParams))
				{

					PlayImpactEffects(Hit.ImpactPoint);

					//*Applying damage*//
					AActor* HitActor = Hit.GetActor();
					EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
					float ActualDamage = BaseDamage;

					HelmetTest = Cast<AHelmet>(HitActor);
					if (HitActor == HelmetTest)
					{
						{
							ServerHelmetHit(HelmetTest);
						}
					}
					if (SurfaceType == SURFACE_HEAD)
					{
						ActualDamage *= 4.0f;
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), HeadshotSound, GetActorLocation());
					}
					if (SurfaceType == SURFACE_CHEST)
					{
						if (SoldierChar->MultipleDamage == true)
						{
							ActualDamage *= 4.0f;
						}
						else
						{
							ActualDamage *= 2.0f;
						}
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, GetActorLocation());
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
						//NO DAMAGE, HELMET BLOCKED THE DAMAGED AND PLAYED OTHER SOUND
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), HelmetSound, GetActorLocation());
					}
					UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);
					UseAmmo();
					PlayFireEffects(Hit.ImpactPoint);
				}
				else
				{
					UseAmmo();
					PlayFireEffects(EndLocation);
				}

				if (Role == ROLE_Authority)
				{
					HitScanTrace.TraceTo = Hit.ImpactPoint;
				}
			}
		}
		else if(SoldierChar->IsZooming == false)
		{
			///CHECK IF SHOULD MULTIPLE DAMAGE FROM BEHIND
			SoldierChar->IsTargetFromBack();

			///RECOILD
			float randomval = UKismetMathLibrary::RandomFloatInRange(-0.1, -0.5);
			SoldierChar->AddControllerPitchInput(randomval);
			float YawRandomVal = UKismetMathLibrary::RandomFloatInRange(-0.2, 0.2);
			SoldierChar->AddControllerYawInput(YawRandomVal);


			AActor* MyOwner = GetOwner();
			if (MyOwner)
			{
				FHitResult Hit;
				FVector StartLocation = SkelMeshComp->GetSocketLocation(MuzzleSocket);
				FRotator temp;
				MyOwner->GetActorEyesViewPoint(StartLocation, temp);

				FRotator RotationCamera = temp;
				FVector ShotDirection = RotationCamera.Vector();

				if (SoldierChar->isGripAttached == true)
				{
					BulletSpread = BulletSpreadGrip;
				}
				float HalfRad = FMath::DegreesToRadians(BulletSpread);
				ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
				FVector EndLocation = StartLocation + (ShotDirection * 100000);

				FCollisionQueryParams QueryParams;
				QueryParams.AddIgnoredActor(MyOwner);
				QueryParams.AddIgnoredActor(this);
				QueryParams.bReturnPhysicalMaterial = true;
				QueryParams.bTraceComplex = true;


				if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECollisionChannel::ECC_Destructible, QueryParams))
				{
					//DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::White, false, 1.0f, 0, 1.0f);

					PlayImpactEffects(Hit.ImpactPoint);

					//*Applying damage*//
					AActor* HitActor = Hit.GetActor();
					EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
					float ActualDamage = BaseDamage;

					HelmetTest = Cast<AHelmet>(HitActor);
					if (HitActor == HelmetTest)
					{
						{
							ServerHelmetHit(HelmetTest);
						}
					}
					if (SurfaceType == SURFACE_HEAD)
					{
						ActualDamage *= 4.0f;
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), HeadshotSound, GetActorLocation());
					}
					if (SurfaceType == SURFACE_CHEST)
					{
						if (SoldierChar->MultipleDamage == true)
						{
							ActualDamage *= 4.0f;
						}
						else
						{
							ActualDamage *= 2.0f;
						}
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
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), HelmetSound, GetActorLocation());
					}
					UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);
					UseAmmo();
					PlayFireEffects(Hit.ImpactPoint);
				}
				else
				{
					UseAmmo();
					PlayFireEffects(EndLocation);

				}
				if (Role == ROLE_Authority)
				{
					HitScanTrace.TraceTo = Hit.ImpactPoint;
				}
			}
		}
	}


		LastFireTime = GetWorld()->TimeSeconds;

	//*Sound when no ammo in clip*//

	if (CurrentAmmoInClip == 0 && CurrentState!=EWeaponState::Reloading)
	{
		ASoldierCharacter* SoldierChar = Cast<ASoldierCharacter>(GetOwner());
		if (SoldierChar)
		{
			SoldierChar->bFireAnimation = false;
		}
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), NoAmmoSound, GetActorLocation());
	}
	
}
void AAutomaticRifle::UseAmmo()
{
	CurrentAmmoInClip--;
	CurrentAmmo--;
}
void AAutomaticRifle::PlayImpactEffects(FVector ImpactPoint)
{
	if (ImpactEffect)
	{
		FVector MuzzleLocation = SkelMeshComp->GetSocketLocation(MuzzleSocket);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, ImpactPoint, ShotDirection.Rotation());
		UGameplayStatics::PlaySoundAtLocation(this, HitImpactSound, ImpactPoint);
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

		SkelMeshComp->PlayAnimation(AnimSeqReload, false);
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
		CurrentAmmoInClip = 20;
		CurrentAmountOfClips--;

		CurrentState = EWeaponState::Idle;
		ReloadingState = EReloadingState::None;

		UE_LOG(LogTemp, Warning, TEXT("Reloaded"));
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
void AAutomaticRifle::ServerHelmetHit_Implementation(AHelmet* HitActor)
{
	if (HitActor)
	{
		HitActor->NumberOfLives--;
	}
}
void AAutomaticRifle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//This function tells us how we want to replicate things//
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AAutomaticRifle, HitScanTrace,COND_SkipOwner);
	DOREPLIFETIME(AAutomaticRifle, SkelMeshComp);
	DOREPLIFETIME(AAutomaticRifle, SoldierChar);


}

