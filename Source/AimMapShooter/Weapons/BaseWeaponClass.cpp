// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseWeaponClass.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Character/SoldierCharacter.h"
#include "TimerManager.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "AimMapShooter.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "WeaponAttachments/Laser.h"
#include "WeaponAttachments/Helmet.h"
#include "Math/Vector.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimSequence.h"
#include "Math/Rotator.h"

// Sets default values
ABaseWeaponClass::ABaseWeaponClass()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraSocket = "CameraSocket";
	MuzzleSocket = "MuzzleFlash";
	LineSocket = "LineSocket";
	ScopeSocket = "ScopeSocket";
	GripSocket = "GripSocket";
	LaserSocket = "LaserSocket";
	LaserSocketEnd = "LaserSocketEnd";

	SkelMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkelMesh"));
	RootComponent = SkelMeshComp;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SkelMeshComp, CameraSocket);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(SkelMeshComp);

	//
	CurrentState = EWeaponState::Idle;
	//
	CurrentAmmo = 0;
	CurrentAmmoInClip = 0;
	CurrentAmountOfClips = 0;
	//
	SmoothSway1 = 5.0f;
	SmoothSway2 = 3.0f;
	//
	SmoothClipping = 7.0f;
	//
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
void ABaseWeaponClass::BeginPlay()
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


	//Setting up weapon sway with a small delay
	FTimerHandle TimerHandle_SwayTimer;
	bStartWeaponSway = true;
	FTimerDelegate DelegateFunc = FTimerDelegate::CreateUObject(this, &ABaseWeaponClass::SetbWeaponSway, bStartWeaponSway);
	GetWorldTimerManager().SetTimer(TimerHandle_SwayTimer, DelegateFunc, 3.0f, false);

	

}

void ABaseWeaponClass::SetbWeaponSway(bool bSway)
{
	bStartWeaponSway = bSway;
}
USkeletalMeshComponent* ABaseWeaponClass::GetSkelMeshComp()
{
	return this->SkelMeshComp;
}

USphereComponent* ABaseWeaponClass::GetSphereComp()
{
	return this->SphereComp;
}

UCameraComponent* ABaseWeaponClass::GetCamera()
{
	return this->Camera;
}

FName ABaseWeaponClass::GetMuzzleSocketName()
{
	return this->MuzzleSocket;
}

FName ABaseWeaponClass::GetScopeSocketName()
{
	return this->ScopeSocket;
}

FName ABaseWeaponClass::GetGripSocketName()
{
	return this->GripSocket;
}

FName ABaseWeaponClass::GetLaserSocketName()
{
	return this->LaserSocket;
}

int32 ABaseWeaponClass::GetCurrentAmmoInClip()
{
	return CurrentAmmoInClip;
}

float ABaseWeaponClass::GetDistanceToObject()
{
	return DistanceToObject;
}
int32 ABaseWeaponClass::GetAllAmmo()
{
	return CurrentAmmo;
}

int32 ABaseWeaponClass::GetCurrentAmountOfClips()
{
	return CurrentAmountOfClips;
}

void ABaseWeaponClass::AddMagazine()
{
	CurrentAmountOfClips++;
}	

void ABaseWeaponClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bStartWeaponSway)
	{
		CalculateWeaponSway();
	}

	LineTraceWeaponClipping();
}

void ABaseWeaponClass::LineTraceWeaponClipping()
{

	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(GetOwner());
	if (SoldierCharacter)
	{
		FVector SoldierCharacterLocation = SoldierCharacter->GetActorLocation();
		FVector SoldierCharacterRotation = UKismetMathLibrary::GetForwardVector(SoldierCharacter->GetActorRotation());
		FVector SoldierCharacterLength = SoldierCharacterRotation * 150.0f;
		FVector SoldierCharacterLineEnd = SoldierCharacterLocation + SoldierCharacterLength;

		FHitResult Hit;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(SoldierCharacter);
		QueryParams.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(Hit, SoldierCharacterLocation, SoldierCharacterLineEnd, ECollisionChannel::ECC_Visibility, QueryParams))
		{
			float CloseToObstacle;
			CloseToObstacle = 50.0f / Hit.Distance;
			CloseToObstacle = UKismetMathLibrary::FClamp(CloseToObstacle, 0.0f, 1.0f);

			DistanceToObject = UKismetMathLibrary::Lerp(DistanceToObject, CloseToObstacle, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()) * SmoothClipping);

			if (DistanceToObject > 0.34f)
			{
				SoldierCharacter->SetbZooming(false);
			}
		}
		else
		{
			DistanceToObject = UKismetMathLibrary::Lerp(DistanceToObject, 0.0f, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()) * SmoothClipping);
		}
	}
}

void ABaseWeaponClass::CalculateWeaponSway()
{
	ASoldierCharacter* SoldierCharOwner = Cast<ASoldierCharacter>(GetOwner());
	if (SoldierCharOwner)
	{
		InitialWeaponRotation = SoldierCharOwner->GetActorRotation();

		if (!InitialWeaponRotation.Equals(FinalWeaponRotation))
		{
			if (InitialWeaponRotation.Yaw > FinalWeaponRotation.Yaw)
			{
				FinalWeaponRotation = InitialWeaponRotation;

				DirectionSway = 10;

				SetWeaponSway(DirectionSway);
			}
			else
			{
				FinalWeaponRotation = InitialWeaponRotation;

				DirectionSway = -10;

				SetWeaponSway(DirectionSway);
			}
		}
		else
		{
			FRotator CurrentRotation = this->GetSkelMeshComp()->GetRelativeTransform().Rotator();
			FRotator TargetRotation = UKismetMathLibrary::RInterpTo(CurrentRotation, FRotator(0.0f, CurrentRotation.Yaw, CurrentRotation.Roll), UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), SmoothSway1);

			this->GetSkelMeshComp()->SetRelativeRotation(TargetRotation);
		}
	}
}

void ABaseWeaponClass::SetWeaponSway(float SwayDirection)
{
	FRotator CurrentRotation = this->GetSkelMeshComp()->GetRelativeTransform().Rotator();
	FRotator FinalRotation = FRotator(CurrentRotation.Pitch + SwayDirection, CurrentRotation.Yaw, CurrentRotation.Roll);

	FinalRotation.Pitch = UKismetMathLibrary::Clamp(FinalRotation.Pitch, -5.f, 5.f);

	FRotator SwayRotation = UKismetMathLibrary::RInterpTo(CurrentRotation, FinalRotation, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), SmoothSway2);

	this->GetSkelMeshComp()->SetRelativeRotation(SwayRotation);
}
void ABaseWeaponClass::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds,0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ABaseWeaponClass::Fire, TimeBetweenShots, true, FirstDelay);
}

void ABaseWeaponClass::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ABaseWeaponClass::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}
	
	if (CurrentAmmoInClip > 0 && CurrentState != EWeaponState::Reloading)
	{
		CurrentState = EWeaponState::Firing;


		ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(GetOwner());
		ALaser* Laser = Cast<ALaser>(GetOwner());
		if (SoldierCharacter->GetbZooming() && DistanceToObject < 0.35f)
		{
			UE_LOG(LogTemp, Warning, TEXT("Zooming"));

			///RECOIL
			float PitchRandomVal = UKismetMathLibrary::RandomFloatInRange(-0.1, -0.5);
			SoldierCharacter->AddControllerPitchInput(PitchRandomVal);
			float YawRandomVal = UKismetMathLibrary::RandomFloatInRange(-0.2, 0.2);
			SoldierCharacter->AddControllerYawInput(YawRandomVal);


			AActor* MyOwner = GetOwner();
			if (MyOwner)
			{
				FHitResult Hit;
				FVector StartLocation = SkelMeshComp->GetSocketLocation(ScopeSocket);
				FRotator Rotation = SkelMeshComp->GetSocketRotation(ScopeSocket);
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
					DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 1.0f, 0, 1.0f);


					PlayImpactEffects(Hit.ImpactPoint);

					//*Applying damage*//
					AActor* HitActor = Hit.GetActor();
					EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
					float ActualDamage = BaseDamage;

					Helmet = Cast<AHelmet>(HitActor);
					if (HitActor == Helmet)
					{
						{
							ServerHelmetHit(Helmet);
						}
					}
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
		else if(SoldierCharacter->GetbZooming() == false || DistanceToObject > 0.35f)
		{
			///RECOIL
			float randomval = UKismetMathLibrary::RandomFloatInRange(-0.1, -0.5);
			SoldierCharacter->AddControllerPitchInput(randomval);
			float YawRandomVal = UKismetMathLibrary::RandomFloatInRange(-0.2, 0.2);
			SoldierCharacter->AddControllerYawInput(YawRandomVal);

			UE_LOG(LogTemp, Warning, TEXT("NotZooming"));

			AActor* MyOwner = GetOwner();
			if (MyOwner)
			{
				FHitResult Hit;
				FVector StartLocation = SkelMeshComp->GetSocketLocation(MuzzleSocket);
				FRotator temp;
				MyOwner->GetActorEyesViewPoint(StartLocation, temp);

				FRotator RotationCamera = temp;
				FVector ShotDirection = RotationCamera.Vector();

				if (SoldierCharacter->GetbGripAttached() == true)
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
					DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::White, false, 1.0f, 0, 1.0f);

					PlayImpactEffects(Hit.ImpactPoint);

					//*Applying damage*//
					AActor* HitActor = Hit.GetActor();
					EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
					float ActualDamage = BaseDamage;

					Helmet = Cast<AHelmet>(HitActor);
					if (HitActor == Helmet)
					{
						{
							ServerHelmetHit(Helmet);
						}
					}
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
		ASoldierCharacter* SoldierCharacterAnimation = Cast<ASoldierCharacter>(GetOwner());
		if (SoldierCharacterAnimation)
		{
			SoldierCharacterAnimation->bFireAnimation = false;
		}
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), NoAmmoSound, GetActorLocation());
	}
	
}
void ABaseWeaponClass::UseAmmo()
{
	CurrentAmmoInClip--;
	CurrentAmmo--;
}
void ABaseWeaponClass::PlayImpactEffects(FVector ImpactPoint)
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

void ABaseWeaponClass::PlayFireEffects(FVector EndLocation)
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
			ASoldierCharacter* SoldierCharacterOwner = Cast<ASoldierCharacter>(GetOwner());
			if (PC && SoldierCharacterOwner->GetbZooming() == true)
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
void ABaseWeaponClass::StartReload()
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

		GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &ABaseWeaponClass::ReloadWeapon, 2.5f, false);
	}

}

void ABaseWeaponClass::StopReload()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_ReloadWeapon);
}

void ABaseWeaponClass::ReloadWeapon()
{
		CurrentAmmoInClip = 20;
		CurrentAmountOfClips--;

		CurrentState = EWeaponState::Idle;
		ReloadingState = EReloadingState::None;

		UE_LOG(LogTemp, Warning, TEXT("Reloaded"));
}
void ABaseWeaponClass::ServerFire_Implementation()
{
	Fire();
}
bool ABaseWeaponClass::ServerFire_Validate()
{
	return true;
}
void ABaseWeaponClass::ServerStartReload_Implementation()
{
	StartReload();
}
bool ABaseWeaponClass::ServerStartReload_Validate()
{
	return true;
}
void ABaseWeaponClass::OnRep_HitScanTrace()
{
	// Play cosmetic FX//
	PlayFireEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.TraceTo);
}
void ABaseWeaponClass::ServerHelmetHit_Implementation(AHelmet* HitActor)
{
	//TODO
	if (HitActor)
	{
		//HitActor->NumberOfLives--;
	}
}
void ABaseWeaponClass::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//This function tells us how we want to replicate things//
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABaseWeaponClass, HitScanTrace,COND_SkipOwner);
	DOREPLIFETIME(ABaseWeaponClass, SkelMeshComp);


}

