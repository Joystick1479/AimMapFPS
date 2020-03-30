// Fill out your copyright notice in the Description page of Project Settings.
#include "SoldierCharacter.h"
#include "Camera/CameraComponent.h"
#include "AutomaticRifle.h"

#include "Components/SkeletalMeshComponent.h"
#include "Animation//AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneComponent.h" 
#include "Components/TimelineComponent.h" 

#include "Containers/EnumAsByte.h"

#include "SpriteComponent.h"
#include "PaperSpriteComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"

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
#include "Engine/EngineTypes.h"
#include "GameFramework/Controller.h"
#include "PayloadCharacter.h"
#include "RedEndgame.h"
#include "BlueEndgame.h"
#include "FlashGrenade.h"
#include "Sound/SoundCue.h"

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

	RootComponent = this->GetRootComponent();

	FPPMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPPMesh"));
	FPPMesh->SetupAttachment(RootComponent);

	SpringArm = CreateDefaultSubobject <USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(FPPMesh);
	

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArm);

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
	AudioComp->SetupAttachment(GetMesh());

	AudioCompReload = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioCompReload"));
	AudioCompReload->SetupAttachment(GetMesh());

	AudioDamageComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioDamageComp"));
	AudioDamageComp->SetupAttachment(GetMesh());

	// MINIMAP //
	SpringArmRender2 = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmRender2"));
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComp"));

	GrenadeStartLocation = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	GrenadeStartLocation->SetupAttachment(this->GetRootComponent());

	// TIMELINE//
	static ConstructorHelpers::FObjectFinder<UCurveFloat> Curve(TEXT("/Game/Blueprints/Granates/C_MyCurve"));
	check(Curve.Succeeded());
	FloatCurve = Curve.Object;


	ZoomingTime = 0.2f;

	IsSingleFire = false;
	IsReloading = false;

	CharacterState = ECharacterState::Idle;
	//HoldingWeaponState = EHoldingWeapon::None;
	HoldingWeaponState = EHoldingWeapon::A4;
	HoldingAttachmentState = EHoldingAttachment::None;
	LaserEquipState = ELaserAttachment::None;
	MaxUseDistance = 400;

	bRiflePickUp = false;

	AmountGrenades = 10;

	SetReplicates(true);
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;

	///// VAULTING /////
	float MaxHeightForVault = 60;
	isAllowClimbing = false;
	isAbleToVault = false;
}



// Called when the game starts or when spawned
void ASoldierCharacter::BeginPlay()
{
	Super::BeginPlay();

	/// TIMELINE///
	FOnTimelineFloat onTimelineCallback;
	FOnTimelineEventStatic onTimelineFinishedCallback;

	if (FloatCurve != NULL)
	{
		MyTimeline = NewObject<UTimelineComponent>(this, FName("TimelineAnimation"));
		MyTimeline->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		this->BlueprintCreatedComponents.Add(MyTimeline);
		MyTimeline->SetNetAddressable();
		MyTimeline->SetPropertySetObject(this);
		MyTimeline->SetDirectionPropertyName(FName("TimelineDirection"));

		MyTimeline->SetLooping(false);
		MyTimeline->SetTimelineLength(1.0f);
		MyTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);

		MyTimeline->SetPlaybackPosition(0.0f, false);

		onTimelineCallback.BindUFunction(this, FName{ TEXT("TimelineCallback") });
		onTimelineFinishedCallback.BindUFunction(this, FName{ TEXT("TimelineFinishedCallback") });
		MyTimeline->AddInterpFloat(FloatCurve, onTimelineCallback);
		MyTimeline->SetTimelineFinishedFunc(onTimelineFinishedCallback);
		

		MyTimeline->RegisterComponent();
	}

	/// MINIMAP ///
	if (IsLocallyControlled())
	{
		if (SpringArmRender2)
		{
			SpringArmRender2->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform);
		}
		UPaperSpriteComponent* SpriteComp = this->FindComponentByClass<UPaperSpriteComponent>();
		if (SpriteComp)
		{
			FRotator Rotator = FRotator(0, 90.0f, 0);
			FVector Vector = FVector(0, 45.0f, 0);
			SpriteComp->SetRelativeRotation(Rotator);
			SpriteComp->SetRelativeLocation(Vector);
		}
		if (SceneCapture)
		{
			SceneCapture->AttachToComponent(SpringArmRender2, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}

	///**Getting weapon on begin play *//
	///**Spawn weapmon for first person animation ///**
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if(Role==ROLE_Authority)
	{
		AutomaticRifle = GetWorld()->SpawnActor<AAutomaticRifle>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (AutomaticRifle)
		{
			AutomaticRifle->SetOwner(this);
			AutomaticRifle->AttachToComponent(FPPMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
			AutomaticRifle->SkelMeshComp->bOnlyOwnerSee = true;
			AutomaticRifle->SkelMeshComp->SetAnimInstanceClass(AnimBp);
		}
	}

	HealthComp->OnHealthChanged.AddDynamic(this, &ASoldierCharacter::OnHealthChanged);

	if (HealthComp)
	{
		HealthComp->Health = 100;
	}

	if (SpringArm)
	{
		SpringArm->AttachToComponent(FPPMesh, FAttachmentTransformRules::SnapToTargetIncludingScale);
	
	}
	if (CameraComp)
	{
		CameraComp->AttachToComponent(SpringArm, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
}

void ASoldierCharacter::LineTraceItem()
{
	//if (Role < ROLE_Authority)
	//{
	//	ServerLineTraceItem();
	//	//return;
	//}

	//	 FVector start_trace = CameraComp->GetComponentLocation();
	//	 FVector direction = CameraComp->GetComponentRotation().Vector();
	//	 FVector end_trace = start_trace + (direction* MaxUseDistance);

	//	FCollisionQueryParams TraceParams(FName(TEXT("")), true, this);
	//	TraceParams.bReturnPhysicalMaterial = false;
	//	TraceParams.bTraceComplex = true;
	//	TraceParams.AddIgnoredActor(this);

	//	FHitResult Hit;

		//	if (GetWorld()->LineTraceSingleByChannel(Hit, start_trace, end_trace, COLLISION_ITEMS, TraceParams) && HoldingWeaponState == EHoldingWeapon::None)
		//	{
		//		DrawDebugLine(GetWorld(), start_trace, end_trace, FColor::Red, false, 1.0f, 0, 1.0f);
		//		bRiflePickUp = true;
		//
		//		AActor* WeaponHit = Hit.GetActor();
		//	}
			//else
			//{
			//	bRiflePickUp = false;
		//	}
		//	if (GetWorld()->LineTraceSingleByChannel(Hit, start_trace, end_trace, COLLISION_HOLO, TraceParams) && (HoldingWeaponState == EHoldingWeapon::A4 && isHoloAttached == false))
		//	{
		//		DrawDebugLine(GetWorld(), start_trace, end_trace, FColor::Green, false, 1.0f, 0, 1.0f);
		//		bHoloPickUp = true;
		//	}
		//	else
		//	{
		//		bHoloPickUp = false;
		//	}
		//	if (GetWorld()->LineTraceSingleByChannel(Hit, start_trace, end_trace, COLLISION_GRIP, TraceParams) && (HoldingWeaponState == EHoldingWeapon::A4 && isGripAttached == false))
		//	{
		//		DrawDebugLine(GetWorld(), start_trace, end_trace, FColor::Blue, false, 1.0f, 0, 1.0f);
		//		bGripPickUp = true;
		//	}
		//	else
		//	{
		//		bGripPickUp = false;
		//	}
		//	if (GetWorld()->LineTraceSingleByChannel(Hit, start_trace, end_trace, COLLISION_HELMET, TraceParams) && isHelmetAttached == false)
		//	{
		//		DrawDebugLine(GetWorld(), start_trace, end_trace, FColor::Blue, false, 1.0f, 0, 1.0f);
		//		bHelmetPickUp = true;
		//	}
		//	else
		//	{
		//		bHelmetPickUp = false;
		//	}
		//	if (GetWorld()->LineTraceSingleByChannel(Hit, start_trace, end_trace, COLLISION_HEADSET, TraceParams) && isHeadsetAttached == false)
		//	{
		//		DrawDebugLine(GetWorld(), start_trace, end_trace, FColor::Black, false, 1.0f, 0, 1.0f);
		//		bHeadsetPickUp = true;
		//	}
		//	else
		//	{
		//		bHeadsetPickUp = false;
		//	}
		//	if (GetWorld()->LineTraceSingleByChannel(Hit, start_trace, end_trace, COLLISION_LASER, TraceParams) && isLaserAttached == false && HoldingWeaponState == EHoldingWeapon::A4)
		//	{
		//		DrawDebugLine(GetWorld(), start_trace, end_trace, FColor::Orange, false, 1.0f, 0, 1.0f);
		//		bLaserPickUp = true;
		//	}
		//	else
		//	{
		//		bLaserPickUp = false;
		//	}
		
	
}

// Called every frame
void ASoldierCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (AutomaticRifle)
	{
		SoldierCurrentAmmoInClip = AutomaticRifle->CurrentAmmoInClip;
		SoldierCurrentAmmo = AutomaticRifle->CurrentAmmo;
		SoldierCurrentClips = AutomaticRifle->CurrentAmountOfClips;
	}

	///***SHOWING/HIDING PICKUP HUD UI////
	ShowingPickUpHud();

	ClearingHudAfterDeath();

	DyingAudioTrigger();

	//DefendObjectiveSound();

	GameOverSound();

	FindingGrenadeTransform();

	///TIMELINE///
	if (MyTimeline != NULL)
	{
		MyTimeline->TickComponent(DeltaTime, ELevelTick::LEVELTICK_TimeOnly, NULL);
	}

}
void ASoldierCharacter::OnDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("DEAD"));

	this->Destroy();

	if (IsLocallyControlled())
	{
		APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
		if (PC)
		{
			this->EnableInput(PC);
		}
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

	PlayerInputComponent->BindAction("Inspect", IE_Pressed, this, &ASoldierCharacter::WeaponInspectionOn);

	PlayerInputComponent->BindAction("Grenade", IE_Pressed, this, &ASoldierCharacter::ThrowGrenade);



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
			//DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Green, false, 1.0f, 0, 1.0f);
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
			//	DrawDebugLine(GetWorld(), StartLocation2, EndLocation2, FColor::Blue, false, 1.0f, 0, 1.0f);
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
			//DrawDebugLine(GetWorld(), StartLocation3, EndLocation3, FColor::Yellow, false, 1.0f, 0, 1.0f);
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
	if (Role < ROLE_Authority)
	{
		ServerTurnOnLaser();
	}
	if (Laser)
	{
		Laser->MeshComp2->ToggleVisibility();
		Laser->PointLight->ToggleVisibility();
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
		/*if (bRiflePickUp == true && HoldingWeaponState==EHoldingWeapon::None)
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
		}*/

		if (bHoloPickUp == true && HoldingWeaponState == EHoldingWeapon::A4 && HoloEquipState == EHoloAttachment::None)
		{
			HoldingAttachmentState = EHoldingAttachment::Holo;
			HoloEquipState = EHoloAttachment::Equipped;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (Role == ROLE_Authority)
			{
				HoloScope = GetWorld()->SpawnActor<AHoloScope>(HoloClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (HoloScope)
				{
					HoloScope->SetOwner(AutomaticRifle);
					FName Socket = AutomaticRifle->ScopeSocket;
					HoloScope->AttachToComponent(AutomaticRifle->SkelMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
					HoloScope->SkelMeshComp->bOnlyOwnerSee = true;
					isHoloAttached = true;
				}
			}
			UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
		}
		if (bGripPickUp == true && HoldingWeaponState == EHoldingWeapon::A4 && GripEquipState == EGripAttachment::None)
		{
			HoldingAttachmentState = EHoldingAttachment::Grip;
			GripEquipState = EGripAttachment::Equipped;

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
		if (bHelmetPickUp == true && HelmetEquipState==EHelmetAttachment::None)
		{
			HelmetEquipState = EHelmetAttachment::Equipped;

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
		if (bHeadsetPickUp == true && HeadsetEquipState == EHeadsetAttachment::None)
		{
			HeadsetEquipState = EHeadsetAttachment::Equipped;

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
		if (bLaserPickUp == true && LaserEquipState == ELaserAttachment::None)
		{
			LaserEquipState = ELaserAttachment::Equipped;

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

void ASoldierCharacter::StartingHud()
{
	//if (Role < ROLE_Authority)
	//{
	//	ServerStartingHud();
	//	//return;
	//}
	/*if (IsLocallyControlled())
	{
		APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());

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
	}*/
}

void ASoldierCharacter::DyingAudioTrigger()
{
	///** PLAYING/STOPPING SOUND WHEN LOW HEALTH/DEAD**//

	if (IsLocallyControlled())
	{
		if (bDied == true)
		{
			if (AudioComp)
			{
				AudioComp->DestroyComponent();
			}
		}
	}
}

void ASoldierCharacter::ClearingHudAfterDeath()
{
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
	
void ASoldierCharacter::ShowingPickUpHud()
{
	//if (Role < ROLE_Authority)
	//{
	//	ServerShowingPickUpHud();
	//}
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
						TArray<UUserWidget*> PickupWidgets;
						UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, PickupWidgets, PickUpTestWidgetClass, true);

						if (PickupWidgets.Num() > 0)
						{
							UUserWidget* NewPickupWidget = PickupWidgets[0];
							NewPickupWidget->RemoveFromParent();
						}
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
	//UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
	//if (MoveComp)
	//{
	//	if (MoveComp->GetLastUpdateVelocity().Size() < 10)
	//	{
			IsCrouching = true;
			Crouch();
	//	}
//	}

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
	if (MoveComp && !IsSprinting &&!IsInspecting &&!IsReloading)
	{
		//MoveComp->MaxWalkSpeed = 78.0f;
		MoveComp->MaxWalkSpeed = 250.0f;

		if (!IsSprinting)
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
	UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
	if (MoveComp)
	{
		MoveComp->MaxWalkSpeed = 300.0f;
	}
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

	IsFiring = true;

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
	IsFiring = false;

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
	if (IsZooming != true)
	{
		IsSprinting = true;
		UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
		if (MoveComp)
		{
			//MoveComp->MaxWalkSpeed = 270.0f;
			MoveComp->MaxWalkSpeed = 400.0f;
		}
	}
	
	//CameraComp->ToggleActive();
	//CameraSprintComp->ToggleActive();
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
		//MoveComp->MaxWalkSpeed = 149.0f;
		MoveComp->MaxWalkSpeed = 300.0f;
	}
	//CameraComp->ToggleActive();
	//CameraSprintComp->ToggleActive();
}

void ASoldierCharacter::Reload()
{
	if (Role < ROLE_Authority)
	{
		ServerReload();
	}
	if (CharacterState == ECharacterState::Idle && (SoldierCurrentClips > 0) && !IsZooming)
	{
		if (AutomaticRifle)
		{
			CharacterState = ECharacterState::Reloading;
			IsReloading = true;
			AutomaticRifle->StartReload();
		//	AudioCompReload->Activate(true);
		//	ServerReloadingSound();
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
		//AudioCompReload->Deactivate();
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
	
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), FiremodeSwitch, this->GetActorLocation());
	
}

void ASoldierCharacter::WeaponInspectionOn()
{
	if (IsInspecting != true)
	{
		IsInspecting = true;
		GetWorld()->GetTimerManager().SetTimer(InspectionTimer, this, &ASoldierCharacter::WeaponInspectionOff, 3.704f);
	}
}
void ASoldierCharacter::WeaponInspectionOff()
{
	IsInspecting = false;
}

void ASoldierCharacter::DefendObjectiveSound()
{
	TArray<AActor*> Payload;
	UGameplayStatics::GetAllActorsOfClass(this, PayloadCharacterClass, Payload);
	for (int i = 0; i < Payload.Num(); i++)
	{
		APayloadCharacter* PayloadChar = Cast<APayloadCharacter>(Payload[i]);
		if (PayloadChar)
		{
			if (IsLocallyControlled())
			{
				if (this->IsOverlappingActor(PayloadChar) == false)
				{
					if (PayloadChar->ShouldPush == true)
					{
						UGameplayStatics::PlaySound2D(this, DefendObjective);
					}
				}
			}
		}
	}
}
void ASoldierCharacter::GameOverSound()
{
	TArray<AActor*> HostEndGame;
	UGameplayStatics::GetAllActorsOfClass(this, HostEndgameClass, HostEndGame);

	for (int i = 0; i < HostEndGame.Num(); i++)
	{
		ABlueEndgame*New = Cast<ABlueEndgame>(HostEndGame[i]);
		if (New)
		{
			if (New->BlueWins == true)
			{
				if (bStopSound == false)
				{
					bStopSound = true;
					UGameplayStatics::PlaySound2D(this, GameOverAudio);
					UE_LOG(LogTemp, Warning, TEXT("Tescik"));
				}
			}
		}
	}

	TArray<AActor*> ClientEndGame;
	UGameplayStatics::GetAllActorsOfClass(this, ClientEndgameClass, ClientEndGame);

	for (int i = 0; i < ClientEndGame.Num(); i++)
	{
		ARedEndgame*New = Cast<ARedEndgame>(ClientEndGame[i]);
		if (New)
		{
			if (New->RedWins == true)
			{
				if (bStopSound == false)
				{
					bStopSound = true;
					UGameplayStatics::PlaySound2D(this, GameOverAudio);
					UE_LOG(LogTemp, Warning, TEXT("Tescik"));
				}
			}
		}
	}
}

void ASoldierCharacter::NotifyActorBeginOverlap(AActor * OtherActor)
{
	bool DoOnce;
	DoOnce = false;
	APayloadCharacter* Payload = Cast<APayloadCharacter>(OtherActor);
	if (Payload)
	{
		if (IsLocallyControlled() && DoOnce == false)
		{
			DoOnce = true;
			UGameplayStatics::PlaySound2D(this, EscortVehicle);
		}
	}
}

void ASoldierCharacter::FindingGrenadeTransform()
{
	if (GrenadeStartLocation)
	{
		STL = GrenadeStartLocation->GetComponentLocation();
		STR = GrenadeStartLocation->GetComponentRotation();
	}
}
void ASoldierCharacter::ThrowGrenade()
{
	if (AmountGrenades > 0)
	{
		AmountGrenades--;
		if (IsLocallyControlled())
		{
			SpawnGrenade(STL, STR);
		}
	}
}
void ASoldierCharacter::SpawnGrenade(FVector STL, FRotator STR)
{
	if (Role < ROLE_Authority)
	{
		ServerSpawnGrenade();
		return;
	}
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FlashGrenade = GetWorld()->SpawnActor<AFlashGrenade>(FlashGrenadeClass, STL, STR, SpawnParams);
	


}
void ASoldierCharacter::Flashbang(float Distance, FVector FacingAngle)
{
	if (IsLocallyControlled())
	{
		FlashAmount = (UKismetMathLibrary::NormalizeToRange(Distance, 20.0f, 100.0f) / 10.0f) * (-1.0f);
		if (Distance < 2000.0f)
		{
			AngleFromFlash(FacingAngle);
			if (IsFacing == true)
			{
				FlashAmount = 0.5f;
			}
			UKismetMaterialLibrary::SetScalarParameterValue(this, MaterialCollection, "Flash_Value", FlashAmount);
			GetWorldTimerManager().SetTimer(Timer_Flash, this, &ASoldierCharacter::FlashTimeline, 1.0f);
		}
	}
}
void ASoldierCharacter::FlashTimeline()
{
	PlayTimeline();
	GetWorldTimerManager().ClearTimer(Timer_Flash);
}
void ASoldierCharacter::TimelineCallback(float interpolatedVal)
{
	float LerpFloat = UKismetMathLibrary::Lerp(FlashAmount, 1.0f, interpolatedVal);
	UKismetMaterialLibrary::SetScalarParameterValue(this, MaterialCollection, "Flash_Value", LerpFloat);

}
void ASoldierCharacter::TimelineFinishedCallback()
{

}
void ASoldierCharacter::PlayTimeline()
{
	if (MyTimeline != NULL)
	{
		MyTimeline->PlayFromStart();
	}
}

void ASoldierCharacter::AngleFromFlash(FVector GrenadeLoc)
{
	if (IsLocallyControlled())
	{
		if (CameraComp)
		{
			FVector CameraLocationVector = CameraComp->GetComponentLocation();
			float LookAtRotaion = UKismetMathLibrary::FindLookAtRotation(CameraLocationVector, GrenadeLoc).Yaw;
			float CameraRotation = CameraComp->GetComponentRotation().Yaw;
			float DiffRotation = LookAtRotaion - CameraRotation;
	
			IsFacing = UKismetMathLibrary::BooleanOR(DiffRotation >= 90.0f, DiffRotation <= -90.0f);
		}
	}
}

void ASoldierCharacter::OnHealthChanged(UHealthComponent * OwningHealthComp, float Health, float HealthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{

	if (Health <= 0.0f && !bDied)
	{
		bDied = true;
	}
	if (HealthComp->Health <= 40.0f && bDied == false)
	{
		AudioComp->Play();
	}

	AudioDamageComp->Play(0.0f);
	
}

void ASoldierCharacter::ServerLineTraceItem_Implementation()
{
	LineTraceItem();
}
bool ASoldierCharacter::ServerLineTraceItem_Validate()
{
	return true;
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
void ASoldierCharacter::ServerStartingHud_Implementation()
{
	StartingHud();
}
bool ASoldierCharacter::ServerStartingHud_Validate()
{
	return true;
}
void ASoldierCharacter::ServerTurnOnLaser_Implementation()
{
	TurnOnLaser();
}
bool ASoldierCharacter::ServerTurnOnLaser_Validate()
{
	return true;
}
void ASoldierCharacter::ServerReloadingSound_Implementation()
{
	MulticastReloadingSound();
}
bool ASoldierCharacter::ServerReloadingSound_Validate()
{
	return true;
}
void ASoldierCharacter::MulticastReloadingSound_Implementation()
{
	AudioCompReload->Activate(true);
}
void ASoldierCharacter::ServerSpawnGrenade_Implementation()
{
	SpawnGrenade(STL,STR);
}
bool ASoldierCharacter::ServerSpawnGrenade_Validate()
{
	return true;
}
void ASoldierCharacter::ServerFlashbang_Implementation(FVector Facing)
{
	MulticastFlashbang(Facing);
}
void ASoldierCharacter::MulticastFlashbang_Implementation(FVector Facing)
{
	FlashAmount = (UKismetMathLibrary::NormalizeToRange(Distance, 20.0f, 100.0f) / 10.0f) * (-1.0f);
	if (Distance < 2000.0f)
	{
		AngleFromFlash(Facing);
		if (IsFacing == true)
		{
			FlashAmount = 0.5f;
		}
		UKismetMaterialLibrary::SetScalarParameterValue(this, MaterialCollection, "Flash_Value", FlashAmount);
		GetWorldTimerManager().SetTimer(Timer_Flash, this, &ASoldierCharacter::FlashTimeline, 1.0f);
	}
}

void ASoldierCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//This function tells us how we want to replicate things//
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASoldierCharacter, AutomaticRifle);

	DOREPLIFETIME(ASoldierCharacter,IsReloading);
	DOREPLIFETIME(ASoldierCharacter, IsFiring);
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
	DOREPLIFETIME(ASoldierCharacter, wHealthIndicator);
	DOREPLIFETIME(ASoldierCharacter, wHealthIndicatorvar);
	DOREPLIFETIME(ASoldierCharacter, wAmmoCount);
	DOREPLIFETIME(ASoldierCharacter, wAmmoCountvar);
	DOREPLIFETIME(ASoldierCharacter, bRiflePickUp);
	DOREPLIFETIME(ASoldierCharacter, bHoloPickUp);
	DOREPLIFETIME(ASoldierCharacter, bGripPickUp);
	DOREPLIFETIME(ASoldierCharacter, bHeadsetPickUp);
	DOREPLIFETIME(ASoldierCharacter, bLaserPickUp);
	DOREPLIFETIME(ASoldierCharacter, bHelmetPickUp);
	DOREPLIFETIME(ASoldierCharacter, PlayerName);
	DOREPLIFETIME(ASoldierCharacter, CameraComp);
	DOREPLIFETIME(ASoldierCharacter, STL);
	DOREPLIFETIME(ASoldierCharacter, STR);






	
}