// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseWeaponClass.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Character/SoldierCharacter.h"
#include "TimerManager.h"

#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

#include "Sound/SoundCue.h"

#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h" 
#include "Components/SkeletalMeshComponent.h"

#include "AimMapShooter.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "WeaponAttachments/Laser.h"
#include "WeaponAttachments/Helmet.h"
#include "WeaponAttachments/HoloScope.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "Net/UnrealNetwork.h"
#include "Animation/AnimSequence.h"

#include "Math/Rotator.h"
#include "Math/Vector.h"

#define PrintLog(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Green,text)

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

	SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
	RootComponent = SceneComp;

	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh1P"));
	Mesh1P->CastShadow = false;
	Mesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh1P->SetCollisionProfileName("WorldStatic");
	Mesh1P->SetSimulatePhysics(false);
	Mesh1P->SetupAttachment(SceneComp);

	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh3P"));
	Mesh3P->CastShadow = true;
	Mesh3P->SetupAttachment(Mesh1P);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(Mesh1P);
	//
	CurrentState = EWeaponState::Idle;
	//
	CurrentAmmoInClip = WeaponConfig.AmmoPerClip;
	CurrentAmmo = WeaponConfig.AmmoPerClip * WeaponConfig.InitialClips;
	WeaponConfig.InitialClips--;
	CurrentAmountOfClips = WeaponConfig.InitialClips;
	//
	SmoothSway1 = 5.0f;
	SmoothSway2 = 3.0f;
	ClampSwayDegree = 5.0f;
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
void ABaseWeaponClass::SetupWeapon(int32 LoadedAmmo, int32 NumberofClips)
{
	CurrentAmmoInClip = LoadedAmmo;
	CurrentAmountOfClips = NumberofClips;
}
void ABaseWeaponClass::SetupHoloScope(AHoloScope * HolScope)
{
	HoloScope = HolScope;
}
// Called when the game starts or when spawned
void ABaseWeaponClass::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;

	//Setting up weapon sway with a small delay
	FTimerHandle TimerHandle_SwayTimer;
	bStartWeaponSway = true;
	FTimerDelegate DelegateFunc = FTimerDelegate::CreateUObject(this, &ABaseWeaponClass::SetbWeaponSway, bStartWeaponSway);
	GetWorldTimerManager().SetTimer(TimerHandle_SwayTimer, DelegateFunc, 3.0f, false);



}
void ABaseWeaponClass::PickUpWeapon()
{
	if (PawnOwner)
	{
		PrintLog("Have owner");

		if (PawnOwner->IsLocallyControlled() == true)
		{
			USkeletalMeshComponent* PawnMesh1P = PawnOwner->GetFPPMesh();
			USkeletalMeshComponent* PawnMesh3P = PawnOwner->GetMesh();
			FName AttachPoint = PawnOwner->GetWeaponAttachPoint();
			SceneComp->AttachToComponent(PawnMesh1P, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachPoint);
			Mesh1P->AttachToComponent(PawnOwner->GetWeaponSpringArm(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			Mesh3P->AttachToComponent(PawnMesh3P, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachPoint);

			//Turn off collision after picking up object to stop line trace to pick it up
			if (SphereComp)
			{
				SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}
	
}
void ABaseWeaponClass::SetOwningPawn(ASoldierCharacter* SoldierCharacter)
{
	if (PawnOwner != SoldierCharacter)
	{
		PawnOwner = SoldierCharacter;
		SetOwner(SoldierCharacter);
	}
}
void ABaseWeaponClass::SetbWeaponSway(bool bSway)
{
	bStartWeaponSway = bSway;
}
USkeletalMeshComponent* ABaseWeaponClass::GetMesh1P()const
{
	return this->Mesh1P;
}
USkeletalMeshComponent* ABaseWeaponClass::GetMesh3P()const
{
	return this->Mesh3P;
}

USphereComponent* ABaseWeaponClass::GetSphereComp()const
{
	return this->SphereComp;
}
AHoloScope* ABaseWeaponClass::GetHoloScope() const
{
	if (HoloScope!=nullptr)
	{
		return HoloScope;
	}
	else
	{
		return nullptr;
	}
}

FName ABaseWeaponClass::GetMuzzleSocketName()const
{
	return this->MuzzleSocket;
}

FName ABaseWeaponClass::GetScopeSocketName()const
{
	return this->ScopeSocket;
}

FName ABaseWeaponClass::GetGripSocketName()const
{
	return this->GripSocket;
}

FName ABaseWeaponClass::GetLaserSocketName()const
{
	return this->LaserSocket;
}

int32 ABaseWeaponClass::GetCurrentAmmoInClip() const
{
	return CurrentAmmoInClip;
}

float ABaseWeaponClass::GetDistanceToObject() const
{
	return DistanceToObject;
}
int32 ABaseWeaponClass::GetAllAmmo() const
{
	return CurrentAmmo;
}

int32 ABaseWeaponClass::GetCurrentAmountOfClips() const
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
		FVector SoldierLocation = SoldierCharacter->GetActorLocation() +FVector(0.0f,0.0f,55.f);
		FVector WeaponRotation = UKismetMathLibrary::GetRightVector(SoldierCharacter->GetFPPMesh()->GetComponentRotation());

		float LengthOfLineTrace = 0;

		if (SoldierCharacter->GetbIsCrouching())
		{
			LengthOfLineTrace = 200.f;
		}
		else
		{
			LengthOfLineTrace = 100.f;
		}

		FVector WeaponRotationVector = WeaponRotation * LengthOfLineTrace;
		FVector WeaponDirectionVector = SoldierLocation + WeaponRotationVector;

		FHitResult Hit;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(SoldierCharacter);
		QueryParams.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(Hit, SoldierLocation, WeaponDirectionVector, ECollisionChannel::ECC_Visibility, QueryParams))
		{
			float CloseToObstacle = 0;

			if (SoldierCharacter->GetbIsCrouching())
			{
				CloseToObstacle = 100.0f / Hit.Distance;
			}
			else
			{
				CloseToObstacle = 50.0f / Hit.Distance;
			}
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

		if (InitialWeaponRotation != FinalWeaponRotation)
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
			FRotator CurrentRotation = this->GetMesh1P()->GetRelativeTransform().Rotator();
			FRotator TargetRotation = UKismetMathLibrary::RInterpTo(CurrentRotation, FRotator(0.0f, CurrentRotation.Yaw, CurrentRotation.Roll), UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), SmoothSway1);

			this->GetMesh1P()->SetRelativeRotation(TargetRotation);
		}
	}
}

void ABaseWeaponClass::SetWeaponSway(float SwayDirection)
{
	FRotator CurrentRotation = this->GetMesh1P()->GetRelativeTransform().Rotator();
	FRotator FinalRotation = FRotator(CurrentRotation.Pitch + SwayDirection, CurrentRotation.Yaw, CurrentRotation.Roll);

	FinalRotation.Pitch = UKismetMathLibrary::Clamp(FinalRotation.Pitch, -ClampSwayDegree, ClampSwayDegree);

	FRotator SwayRotation = UKismetMathLibrary::RInterpTo(CurrentRotation, FinalRotation, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), SmoothSway2);

	this->GetMesh1P()->SetRelativeRotation(SwayRotation);
}
void ABaseWeaponClass::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
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
			///RECOIL
			float PitchRandomVal = UKismetMathLibrary::RandomFloatInRange(-0.1, -0.5);
			SoldierCharacter->AddControllerPitchInput(PitchRandomVal);
			float YawRandomVal = UKismetMathLibrary::RandomFloatInRange(-0.2, 0.2);
			SoldierCharacter->AddControllerYawInput(YawRandomVal);

			AActor* MyOwner = GetOwner();
			if (MyOwner)
			{
				FHitResult Hit;
				
				FVector StartLocation = Mesh1P->GetSocketLocation(ScopeSocket);
				FRotator Rotation = Mesh1P->GetSocketRotation(ScopeSocket);
				if (HoloScope)
				{
					StartLocation = HoloScope->GetMesh1P()->GetSocketLocation("LineSocket");
					Rotation = HoloScope->GetMesh1P()->GetSocketRotation("LineSocket");
				}
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
					//DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 1.0f, 0, 1.0f);

					PlayImpactEffects(Hit.ImpactPoint);

					Mesh1P->PlayAnimation(AnimFire, false);

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

						SoldierCharacter->PlayHitSound("Head");
					}
					if (SurfaceType == SURFACE_CHEST)
					{
						ActualDamage *= 2.0f;

						SoldierCharacter->PlayHitSound("Normal");
					}
					if (SurfaceType == SURFACE_LEG)
					{
						ActualDamage *= 0.5f;

						SoldierCharacter->PlayHitSound("Normal");
					}
					if (SurfaceType == SURFACE_ARM)
					{
						ActualDamage *= 1.5f;

						SoldierCharacter->PlayHitSound("Normal");
					}
					if (SurfaceType == SURFACE_HELMET)
					{
						SoldierCharacter->PlayHitSound("Helmet");
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
		else if (SoldierCharacter->GetbZooming() == false || DistanceToObject > 0.35f)
		{
			UE_LOG(LogTemp, Warning, TEXT("No zooming"));
			///RECOIL
			float randomval = UKismetMathLibrary::RandomFloatInRange(-0.1, -0.5);
			SoldierCharacter->AddControllerPitchInput(randomval);
			float YawRandomVal = UKismetMathLibrary::RandomFloatInRange(-0.2, 0.2);
			SoldierCharacter->AddControllerYawInput(YawRandomVal);

			AActor* MyOwner = GetOwner();
			if (MyOwner)
			{
				FHitResult Hit;
				FVector StartLocation = Mesh1P->GetSocketLocation(MuzzleSocket);
				FRotator temp = Mesh1P->GetSocketRotation(MuzzleSocket);

				FVector RotationCamera = temp.Vector();
				FVector ShotDirection = RotationCamera;

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
					//DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::White, false, 1.0f, 0, 1.0f);

					PlayImpactEffects(Hit.ImpactPoint);

					Mesh1P->PlayAnimation(AnimFire, false);

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

						SoldierCharacter->PlayHitSound("Head");
					}
					if (SurfaceType == SURFACE_CHEST)
					{
						ActualDamage *= 2.0f;

						SoldierCharacter->PlayHitSound("Normal");
					}
					if (SurfaceType == SURFACE_LEG)
					{
						ActualDamage *= 0.5f;

						SoldierCharacter->PlayHitSound("Normal");
					}
					if (SurfaceType == SURFACE_ARM)
					{
						ActualDamage *= 1.5f;

						SoldierCharacter->PlayHitSound("Normal");
					}
					if (SurfaceType == SURFACE_HELMET)
					{
						SoldierCharacter->PlayHitSound("Helmet");
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

	if (CurrentAmmoInClip == 0 && CurrentState != EWeaponState::Reloading)
	{
		ASoldierCharacter* SoldierCharacterAnimation = Cast<ASoldierCharacter>(GetOwner());
		if (SoldierCharacterAnimation)
		{
			SoldierCharacterAnimation->bFireAnimation = false;
		}
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
		FVector MuzzleLocation = Mesh1P->GetSocketLocation(MuzzleSocket);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, ImpactPoint, ShotDirection.Rotation());
		UGameplayStatics::PlaySoundAtLocation(this, HitImpactSound, ImpactPoint);
	}
}

void ABaseWeaponClass::PlayFireEffects(FVector EndLocation)
{
	FVector MuzzleLocation = Mesh1P->GetSocketLocation(MuzzleSocket);

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

}
void ABaseWeaponClass::StartReload()
{
	if (Role < ROLE_Authority)
	{
		ServerStartReload();
		//return;
	}

	if (ReloadingState == EReloadingState::None && CurrentAmountOfClips > 0)
	{
		ReloadingState = EReloadingState::Reloading;
		CurrentState = EWeaponState::Reloading;
		
		this->Mesh1P->PlayAnimation(AnimSeqReload, false);
		
		//UGameplayStatics::PlaySoundAtLocation(GetWorld(), ReloadSound, GetActorLocation());

		GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &ABaseWeaponClass::ReloadWeapon, 2.167f, false);
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

	DOREPLIFETIME_CONDITION(ABaseWeaponClass, HitScanTrace, COND_SkipOwner);
	DOREPLIFETIME(ABaseWeaponClass, Mesh1P);


}

