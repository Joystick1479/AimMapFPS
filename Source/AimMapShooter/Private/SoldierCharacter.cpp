// Fill out your copyright notice in the Description page of Project Settings.
#include "SoldierCharacter.h"
#include "Camera/CameraComponent.h"
#include "AutomaticRifle.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HealthComponent.h"
#include "AimMapShooter.h"
#include "HoloScope.h"
#include "Grip.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Helmet.h"
#include "Headset.h"
#include "Laser.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h" 
#include "Sound/SoundCue.h" 
#include "Components/AudioComponent.h"


// Sets default values
ASoldierCharacter::ASoldierCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HeadSocket = "HeadSocket";
	WeaponSocket = "WeaponSocket";
	ArmSocket = "ArmSocket";
	HelmetSocket = "HelmetSocket";
	HeadsetSocket = "HeadsetSocket";

	SpringArm = CreateDefaultSubobject <USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeadSocket);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->AttachToComponent(SpringArm, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));




	ZoomingTime = 0.2f;

	IsSingleFire = false;
	IsReloading = false;

	CharacterState = ECharacterState::Idle;
	HoldingWeaponState = EHoldingWeapon::None;
	HoldingAttachmentState = EHoldingAttachment::None;
	LaserState = ELaserState::Idle;
	MaxUseDistance = 400;

	bRiflePickUp = false;

	SetReplicates(true);

	///// VAULTING /////
	float MaxHeightForVault = 60;
	isAllowClimbing = false;
	isAbleToVault = false;

}


// Called when the game starts or when spawned
void ASoldierCharacter::BeginPlay()
{
	Super::BeginPlay();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	HealthComp->OnHealthChanged.AddDynamic(this, &ASoldierCharacter::OnHealthChanged);

	///***Creating hud displaying***////
	if (IsLocallyControlled())
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			if (wAmmoCount)
			{
				wAmmoCountvar = CreateWidget<UUserWidget>(PC, wAmmoCount);
				if (wAmmoCountvar)
				{
					wAmmoCountvar->AddToViewport();
				}
			}
			if (wHealthIndicator)
			{
				wHealthIndicatorvar = CreateWidget<UUserWidget>(PC, wHealthIndicator);
				if (wHealthIndicatorvar)
				{
					wHealthIndicatorvar->AddToViewport();
				}
			}
		}
	}
}

void ASoldierCharacter::LineTraceItem()
{
		const FVector start_trace = CameraComp->GetComponentLocation();
		const FVector direction = CameraComp->GetComponentRotation().Vector();
		const FVector end_trace = start_trace + (direction* MaxUseDistance);

		FCollisionQueryParams TraceParams(FName(TEXT("")), true, this);
		TraceParams.bReturnPhysicalMaterial = false;
		TraceParams.bTraceComplex = true;
		TraceParams.AddIgnoredActor(this);

		FHitResult Hit;

		if (GetWorld()->LineTraceSingleByChannel(Hit, start_trace, end_trace, COLLISION_ITEMS, TraceParams) && HoldingWeaponState == EHoldingWeapon::None)
		{
			DrawDebugLine(GetWorld(), start_trace, end_trace, FColor::Red, false, 1.0f, 0, 1.0f);
			bRiflePickUp = true;

			AActor* WeaponHit = Hit.GetActor();
		}
		else 
		{
			bRiflePickUp = false;
		}
		if (GetWorld()->LineTraceSingleByChannel(Hit, start_trace, end_trace, COLLISION_HOLO, TraceParams) && (HoldingWeaponState == EHoldingWeapon::A4 && isHoloAttached == false))
		{
			DrawDebugLine(GetWorld(), start_trace, end_trace, FColor::Green, false, 1.0f, 0, 1.0f);
			bHoloPickUp = true;
		}
		else
		{
			bHoloPickUp = false;
		}
		if (GetWorld()->LineTraceSingleByChannel(Hit, start_trace, end_trace, COLLISION_GRIP, TraceParams) && (HoldingWeaponState == EHoldingWeapon::A4 && isGripAttached == false))
		{
			DrawDebugLine(GetWorld(), start_trace, end_trace, FColor::Blue, false, 1.0f, 0, 1.0f);
			bGripPickUp = true;
		}
		else
		{
			bGripPickUp = false;
		}
		if (GetWorld()->LineTraceSingleByChannel(Hit, start_trace, end_trace, COLLISION_HELMET, TraceParams) && isHelmetAttached == false)
		{
			DrawDebugLine(GetWorld(), start_trace, end_trace, FColor::Blue, false, 1.0f, 0, 1.0f);
			bHelmetPickUp = true;
		}
		else
		{
			bHelmetPickUp = false;
		}
		if (GetWorld()->LineTraceSingleByChannel(Hit, start_trace, end_trace, COLLISION_HEADSET, TraceParams) && isHeadsetAttached == false)
		{
			DrawDebugLine(GetWorld(), start_trace, end_trace, FColor::Black, false, 1.0f, 0, 1.0f);
			bHeadsetPickUp = true;
		}
		else
		{
			bHeadsetPickUp = false;
		}
		if (GetWorld()->LineTraceSingleByChannel(Hit, start_trace, end_trace, COLLISION_LASER, TraceParams) && isLaserAttached == false && HoldingWeaponState == EHoldingWeapon::A4)
		{
			DrawDebugLine(GetWorld(), start_trace, end_trace, FColor::Orange, false, 1.0f, 0, 1.0f);
			bLaserPickUp = true;
		}
		else
		{
			bLaserPickUp = false;
		}
	
}

// Called every frame
void ASoldierCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LineTraceItem();
	ShowingPickUpHud();

	if (AutomaticRifle)
	{
		SoldierCurrentAmmoInClip = AutomaticRifle->CurrentAmmoInClip;
		SoldierCurrentAmmo = AutomaticRifle->CurrentAmmo;
		SoldierCurrentClips = AutomaticRifle->CurrentAmountOfClips;
	}

	///***IF DEAD, DESTROY ACTOR AFTER 2 seconds ***///
	if (bDied == true)
	{
			APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
			if (PC)
			{
				this->DisableInput(PC);
			}
			FTimerHandle DeathTimer;
			GetWorldTimerManager().SetTimer(DeathTimer, this, &ASoldierCharacter::OnDeath, 2.5f, false);
	}

}
void ASoldierCharacter::OnDeath()
{
	this->Destroy();
	UE_LOG(LogTemp, Warning, TEXT("DEAD"));

	APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		this->EnableInput(PC);
	}
}
// Called to bind functionality to input
void ASoldierCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Keyboard movement
	PlayerInputComponent->BindAxis("MoveForward", this, &ASoldierCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASoldierCharacter::MoveRight);

	//Mouse look
	PlayerInputComponent->BindAxis("LookUp", this, &ASoldierCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASoldierCharacter::AddControllerYawInput);

	//Crouching
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASoldierCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASoldierCharacter::EndCrouch);

	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &ASoldierCharacter::ZoomIn);
	PlayerInputComponent->BindAction("ZoomIn", IE_Released, this, &ASoldierCharacter::ZoomOut);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASoldierCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASoldierCharacter::StopFire);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASoldierCharacter::Reload);

	PlayerInputComponent->BindAction("FireMode", IE_Pressed, this, &ASoldierCharacter::FireMode);

	PlayerInputComponent->BindAction("PickUp", IE_Pressed, this, &ASoldierCharacter::PickUp);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASoldierCharacter::SprintOn);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASoldierCharacter::SprintOff);

	PlayerInputComponent->BindAction("ToogleLaser", IE_Pressed, this, &ASoldierCharacter::TurnOnLaser);

	PlayerInputComponent->BindAction("Vault", IE_Pressed, this, &ASoldierCharacter::Vault);


}
void ASoldierCharacter::Vault()
{
	if (Role < ROLE_Authority)
	{
		ServerVault();
	}

		FHitResult Hit;
		FVector StartLocation = GetActorLocation() - FVector(0, 0, 44);
		FVector EndLocation = (GetActorForwardVector() * 100) + StartLocation;
		FCollisionObjectQueryParams QueryParams;
		FCollisionQueryParams CollisionParams;
		CollisionParams.bTraceComplex = true;

		///// CHECKING IF OBJECT IS CLOSE ENOUGH////
		if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, COLLISION_TRACE, CollisionParams))
		{
			WallLocation = Hit.ImpactPoint;
			WallNormal = Hit.Normal;
			DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Green, false, 1.0f, 0, 1.0f);
			isAbleToVault = true;

			///// CHECKING IF OBJECT IS HIGH ENOUGH ////
			FHitResult Hit2;
			FRotator Rotator = UKismetMathLibrary::MakeRotFromX(WallNormal);
			FVector TempStartLocation2 = UKismetMathLibrary::GetForwardVector(Rotator);
			FVector AlmostStartLocation2 = (TempStartLocation2 * (-10)) + WallLocation;
			FVector StartLocation2 = AlmostStartLocation2 + FVector(0, 0, 200);
			FVector EndLocation2 = StartLocation2 - FVector(0, 0, 200);

			if (GetWorld()->LineTraceSingleByChannel(Hit2, StartLocation2, EndLocation2, COLLISION_TRACE, CollisionParams) && isAbleToVault == true)
			{
				DrawDebugLine(GetWorld(), StartLocation2, EndLocation2, FColor::Blue, false, 1.0f, 0, 1.0f);
				WallHight = Hit2.ImpactPoint;
				float Test = (WallHight - WallLocation).Z;
				if (Test < MaxHeightForVault)
				{
					isObjectTooHigh = false;
					UE_LOG(LogTemp, Warning, TEXT("Object is not to high:%f "), Test);

				}
				else
				{
					isObjectTooHigh = true;
					UE_LOG(LogTemp, Warning, TEXT("Object is to high. It is:%f "), Test);
				}
			}

		}
		else
		{
			isAbleToVault = false;
		}

		/// GETTING THIRD LINE TRACE FOR THICKNESS TO DECIDE IF VAULT OR CLIMB ///

		FHitResult Hit3;
		FRotator Rotator2 = UKismetMathLibrary::MakeRotFromX(WallNormal);
		FVector TempStartLocation3 = UKismetMathLibrary::GetForwardVector(Rotator2);
		FVector AlmostStartLocation3 = (TempStartLocation3 * (-50)) + WallLocation;
		FVector StartLocation3 = AlmostStartLocation3 + FVector(0, 0, 250);
		FVector EndLocation3 = StartLocation3 - FVector(0, 0, 300);
		if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation3, EndLocation3, COLLISION_TRACE, CollisionParams) && isAbleToVault == true)
		{
			DrawDebugLine(GetWorld(), StartLocation3, EndLocation3, FColor::Yellow, false, 1.0f, 0, 1.0f);
			NextWallHight = Hit3.ImpactPoint;
			isAllowClimbing = true;
		}
		else
		{
			isAllowClimbing = false;
		}

		//// IF ALL THE TERMS ARE GOOD THEN GO VAULT OR CLIMB ////
		if (isAllowClimbing == true && isAbleToVault == true && isObjectTooHigh == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("Climb"));

			///***Turning off collision when getting on the obstacle***//
			UCapsuleComponent* CapsuleComponent = this->FindComponentByClass<UCapsuleComponent>();
			if (CapsuleComponent)
			{
				CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			}

			//***Setting up flying movement when vaulting/climbing**//
			UCharacterMovementComponent* CharMovement = this->FindComponentByClass<UCharacterMovementComponent>();
			if (CharMovement)
			{
				CharMovement->SetMovementMode(EMovementMode::MOVE_Flying);
			}

		//	this->PlayAnimMontage(ClimbAnim); IMPLENTED IN ANIMATION BP

			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC)
			{
				PC->DisableInput(PC);
			}

			GoClimb = true;
			GoVault = false;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_Vault, this, &ASoldierCharacter::ResetVaultTimer, 0.7f, false);
		}
		else if (isAbleToVault == true && isAllowClimbing == false && isObjectTooHigh == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("Vault"));
			GoClimb = false;
			GoVault = true;

			///***Turning off collision when getting on the obstacle***//
			UCapsuleComponent* CapsuleComponent = this->FindComponentByClass<UCapsuleComponent>();
			if (CapsuleComponent)
			{
				CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			}

			//***Setting up flying movement when vaulting/climbing**//
			UCharacterMovementComponent* CharMovement = this->FindComponentByClass<UCharacterMovementComponent>();
			if (CharMovement)
			{
				CharMovement->SetMovementMode(EMovementMode::MOVE_Flying);
			}

		//	this->PlayAnimMontage(VaultAnim);  IMPLENTED IN ANIMATION_BP

			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC)
			{
				PC->DisableInput(PC);
			}


			GetWorld()->GetTimerManager().SetTimer(TimerHandle_Vault, this, &ASoldierCharacter::ResetVaultTimer, 0.95f, false);
		}
		else
		{
			GoVault = false;
			GoClimb = false;
		}
}

void ASoldierCharacter::ResetVaultTimer()
{
	if (Role < ROLE_Authority)
	{
		ServerResetTimerVault();
	}

	GoClimb = false;
	GoVault = false;

	UE_LOG(LogTemp, Warning, TEXT("Resetting timer"));

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Vault);

	UCapsuleComponent* CapsuleComponent = this->FindComponentByClass<UCapsuleComponent>();
	if (CapsuleComponent)
	{
		CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	UCharacterMovementComponent* CharMovement = this->FindComponentByClass<UCharacterMovementComponent>();
	if (CharMovement)
	{
		CharMovement->SetMovementMode(EMovementMode::MOVE_Walking);
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->EnableInput(PC);
	}
}

void ASoldierCharacter::TurnOnLaser()
{
	if (Laser && LaserState == ELaserState::Start)
	{
		Laser->MeshComp2->SetVisibility(false);
		LaserState = ELaserState::Idle;
	}
	else 
	{
		Laser->StartLaser();
		Laser->MeshComp2->SetVisibility(true);
		LaserState = ELaserState::Start;
	}
	
}

void ASoldierCharacter::ServerPickUpItem_Implementation()
{
	PickUp();
}
bool ASoldierCharacter::ServerPickUpItem_Validate()
{
	return true;
}
void ASoldierCharacter::PickUp()
{
	if (Role < ROLE_Authority)
	{
		ServerPickUpItem();
		//return;
	}
		if (bRiflePickUp == true)
		{

			HoldingWeaponState = EHoldingWeapon::A4;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AutomaticRifle = GetWorld()->SpawnActor<AAutomaticRifle>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (AutomaticRifle)
			{
				AutomaticRifle->SetOwner(this);
				AutomaticRifle->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
			}
			UGameplayStatics::PlaySoundAtLocation(this, RiflePickUp, GetActorLocation());
		}

		if (bHoloPickUp == true && HoldingWeaponState == EHoldingWeapon::A4)
		{
			HoldingAttachmentState = EHoldingAttachment::Holo;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			HoloScope = GetWorld()->SpawnActor<AHoloScope>(HoloClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (HoloScope)
			{
				HoloScope->SetOwner(AutomaticRifle);
				FName Socket = AutomaticRifle->ScopeSocket;
				HoloScope->AttachToComponent(AutomaticRifle->SkelMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
				isHoloAttached = true;
			}
			UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
		}
		if (bGripPickUp == true && HoldingWeaponState == EHoldingWeapon::A4)
		{
			HoldingAttachmentState = EHoldingAttachment::Grip;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			Grip = GetWorld()->SpawnActor<AGrip>(GripClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (Grip)
			{
				Grip->SetOwner(AutomaticRifle);
				FName GSocket = AutomaticRifle->GripSocket;
				Grip->AttachToComponent(AutomaticRifle->SkelMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, GSocket);
				isGripAttached = true;
			}
			UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
		}
		if (bHelmetPickUp == true)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			Helmet = GetWorld()->SpawnActor<AHelmet>(HelmetClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (Helmet)
			{
				Helmet->SetOwner(this);
				Helmet->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HelmetSocket);
				isHelmetAttached = true;
			}
			UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
		}
		if (bHeadsetPickUp == true)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			Headset = GetWorld()->SpawnActor<AHeadset>(HeadsetClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (Headset)
			{
				Headset->SetOwner(this);
				Headset->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeadsetSocket);
				isHeadsetAttached = true;
			}
			UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
		}
		if (bLaserPickUp == true)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			Laser = GetWorld()->SpawnActor<ALaser>(LaserClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (Laser)
			{
				Laser->SetOwner(AutomaticRifle);
				FName LSocket = AutomaticRifle->LaserSocket;
				Laser->AttachToComponent(AutomaticRifle->SkelMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, LSocket);
				isLaserAttached = true;
			}
			UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
		}
}
void ASoldierCharacter::ShowingPickUpHud()
{
	if (Role < ROLE_Authority)
	{
		ServerShowingPickUpHud();
	}
	if (IsLocallyControlled())
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			if (wPickUp)
			{
				wPickUpvar = CreateWidget<UUserWidget>(PC, wPickUp);
				if (wPickUpvar)
				{
					if ((bRiflePickUp || bHeadsetPickUp || bLaserPickUp || bHelmetPickUp || bGripPickUp || bHoloPickUp) == true)
					{
						wPickUpvar->AddToViewport();
					}
					else
					{
						bRemoveHud = true;
					}
				}
			}
		}
	}
	
}

void ASoldierCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector()*Value);
}
void ASoldierCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}
void ASoldierCharacter::BeginCrouch()
{
	if (Role < ROLE_Authority)
	{
		ServerBeginCrouch();
	}
	IsCrouching = true;
	Crouch();
}
void ASoldierCharacter::EndCrouch()
{
	if (Role < ROLE_Authority)
	{
		ServerEndCrouch();
	}
	IsCrouching = false;
	UnCrouch();

}
void ASoldierCharacter::ZoomIn()
{
	UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
	if (MoveComp)
	{
		///***Checking for character speed*////
		FVector Velocity = MoveComp->GetLastUpdateVelocity();
		float Velocity2 = Velocity.Size();
		if (Velocity2 < 1)
		{
			if (Role < ROLE_Authority)
			{
				ServerZoomIn();
				//return;
			}
			IsZooming = true;

			if (IsLocallyControlled())
			{

				APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
				if (PC)
				{
					if (AutomaticRifle)
					{
						PC->SetViewTargetWithBlend(AutomaticRifle, ZoomingTime, EViewTargetBlendFunction::VTBlend_Linear);
					}
				}
			}
		}
	}
	

}
void ASoldierCharacter::ZoomOut()
{
	if (Role < ROLE_Authority)
	{
		ServerZoomOut();
		//return;
	}
	IsZooming = false;

	if (IsLocallyControlled())
	{

		APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
		if (PC)
		{
			if (AutomaticRifle)
			{
				PC->SetViewTargetWithBlend(this, ZoomingTime, EViewTargetBlendFunction::VTBlend_Linear);
			}
		}
	}

}

void ASoldierCharacter::StartFire()
{
	CharacterState = ECharacterState::Firing;

	if (IsSingleFire == false)
	{
		if (AutomaticRifle)
		{
			AutomaticRifle->StartFire();

		}
	}
	else
	{
		if (AutomaticRifle)
		{
			AutomaticRifle->Fire();

		}
	}
}

void ASoldierCharacter::StopFire()
{
	CharacterState = ECharacterState::Idle;

	if (AutomaticRifle)
	{
		AutomaticRifle->StopFire();
	}
}

void ASoldierCharacter::SprintOn()
{
	if (Role < ROLE_Authority)
	{
		ServerSprintOn();
	}

	IsSprinting = true;
	UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
	if (MoveComp)
	{
		MoveComp->MaxWalkSpeed = 270.0f;
	}
}

void ASoldierCharacter::SprintOff()
{
	if (Role < ROLE_Authority)
	{
		ServerSprintOff();
	}
	IsSprinting = false;
	UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
	if (MoveComp)
	{
		MoveComp->MaxWalkSpeed = 149.0f;
	}
}

void ASoldierCharacter::Reload()
{
	if (Role < ROLE_Authority)
	{
		ServerReload();
	}
	if (CharacterState == ECharacterState::Idle && (SoldierCurrentClips > 0))
	{
		if (AutomaticRifle)
		{
			CharacterState = ECharacterState::Reloading;
			IsReloading = true;
			AutomaticRifle->StartReload();
			GetWorldTimerManager().SetTimer(ReloadTimer, this, &ASoldierCharacter::StopReload, 2.167f, false);
		}
	}
}

void ASoldierCharacter::StopReload()
{
	if (CharacterState == ECharacterState::Reloading)
	{
		CharacterState = ECharacterState::Idle;
		IsReloading = false;
		GetWorldTimerManager().ClearTimer(ReloadTimer);
	}
}

void ASoldierCharacter::FireMode()
{
	//* NO SWITCHING FIRE MODES WHEN FIRING *//
	if (CharacterState != ECharacterState::Firing)
	{
		if (IsSingleFire == false)
		{
			IsSingleFire = true;
		}
		else
		{
			IsSingleFire = false;
		}
	}
}

void ASoldierCharacter::OnHealthChanged(UHealthComponent * OwningHealthComp, float Health, float HealthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{

	if (Health <= 0.0f && !bDied)
	{
		bDied = true;

	}
	if (Health == 40.0f && bDied == false)
	{
		AudioComp->Play();
	
	}
	if (bDied == true)
	{
		AudioComp->Stop();
	}
}

void ASoldierCharacter::ServerResetTimerVault_Implementation()
{
	ResetVaultTimer();
}
bool ASoldierCharacter::ServerResetTimerVault_Validate()
{
	return true;
}
void ASoldierCharacter::ServerVault_Implementation()
{
	Vault();
}
bool ASoldierCharacter::ServerVault_Validate()
{
	return true;
}
void ASoldierCharacter::ServerReload_Implementation()
{
	Reload();
}
bool ASoldierCharacter::ServerReload_Validate()
{
	return true;
}
void ASoldierCharacter::ServerZoomIn_Implementation()
{
	ZoomIn();
}
bool ASoldierCharacter::ServerZoomIn_Validate()
{
	return true;
}
void ASoldierCharacter::ServerZoomOut_Implementation()
{
	ZoomOut();
}
bool ASoldierCharacter::ServerZoomOut_Validate()
{
	return true;
}
void ASoldierCharacter::ServerSprintOn_Implementation()
{
	SprintOn();
}
bool ASoldierCharacter::ServerSprintOn_Validate()
{
	return true;
}
void ASoldierCharacter::ServerSprintOff_Implementation()
{
	SprintOff();
}
bool ASoldierCharacter::ServerSprintOff_Validate()
{
	return true;
}
void ASoldierCharacter::ServerBeginCrouch_Implementation()
{
	BeginCrouch();
}
bool ASoldierCharacter::ServerBeginCrouch_Validate()
{
	return true;
}
void ASoldierCharacter::ServerEndCrouch_Implementation()
{
	EndCrouch();
}
bool ASoldierCharacter::ServerEndCrouch_Validate()
{
	return true;
}
void ASoldierCharacter::ServerShowingPickUpHud_Implementation()
{
	ShowingPickUpHud();
}
bool ASoldierCharacter::ServerShowingPickUpHud_Validate()
{
	return true;
}
void ASoldierCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//This function tells us how we want to replicate things//
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASoldierCharacter, AutomaticRifle);

	///GAME CHANGER!!!///
	DOREPLIFETIME(ASoldierCharacter,IsReloading);
	DOREPLIFETIME(ASoldierCharacter, IsZooming);
	DOREPLIFETIME(ASoldierCharacter, ClimbAnim);
	DOREPLIFETIME(ASoldierCharacter, VaultAnim);
	DOREPLIFETIME(ASoldierCharacter, isAllowClimbing);
	DOREPLIFETIME(ASoldierCharacter, isWallThick);
	DOREPLIFETIME(ASoldierCharacter, isAbleToVault);
	DOREPLIFETIME(ASoldierCharacter, isObjectTooHigh);
	DOREPLIFETIME(ASoldierCharacter, GoClimb);
	DOREPLIFETIME(ASoldierCharacter, GoVault);
	DOREPLIFETIME(ASoldierCharacter, MaxHeightForVault);
	DOREPLIFETIME(ASoldierCharacter, TimerHandle_Vault);
	DOREPLIFETIME(ASoldierCharacter, bDied);
	DOREPLIFETIME(ASoldierCharacter, IsCrouching);
	DOREPLIFETIME(ASoldierCharacter, bRemoveHud);
	DOREPLIFETIME(ASoldierCharacter, wPickUpvar);
	DOREPLIFETIME(ASoldierCharacter, wPickUp);
	DOREPLIFETIME(ASoldierCharacter, wHealthIndicator);
	DOREPLIFETIME(ASoldierCharacter, wHealthIndicatorvar);
	DOREPLIFETIME(ASoldierCharacter, wAmmoCount);
	DOREPLIFETIME(ASoldierCharacter, wAmmoCountvar);


}