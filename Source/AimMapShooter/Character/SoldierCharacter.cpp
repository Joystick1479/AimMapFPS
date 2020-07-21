// Fill out your copyright notice in the Description page of Project Settings.
#include "SoldierCharacter.h"
#include "Camera/CameraComponent.h"

#include "Weapons/AutomaticRifle.h"
#include "Weapons/SniperRifle.h"
#include "Weapons/ScarH.h"
#include "Weapons/M4Rifle.h"

#include "Components/SkeletalMeshComponent.h"
#include "Animation//AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneComponent.h" 
#include "Components/TimelineComponent.h" 
#include "Containers/EnumAsByte.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"

#include "Character/HealthComponent.h"
#include "Survival/SurvivalComponent.h"

#include "AimMapShooter.h"

#include "WeaponAttachments/HoloScope.h"
#include "WeaponAttachments/Grip.h"
#include "WeaponAttachments/Helmet.h"
#include "WeaponAttachments/Headset.h"
#include "WeaponAttachments/Laser.h"
#include "WeaponAttachments/Magazine.h"

#include "3rdPersonMeshes/Rifle_3rd.h"
#include "3rdPersonMeshes/AK47_3rd.h"
#include "3rdPersonMeshes/M4_3rd.h"

#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "SingleplayerGameMode.h"

#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h" 
#include "Sound/SoundCue.h" 
#include "Components/AudioComponent.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Controller.h"
#include "AI/PayloadCharacter.h"
#include "Grenades/FlashGrenade.h"
#include "Sound/SoundCue.h"
#include "Survival/Drink.h"
#include "Survival/Food.h"
#include "Survival/Water.h"
#include "Survival/BaseSurvivalItemClass.h"
#include "AimMapGameModeBase.h"

// Sets default values
ASoldierCharacter::ASoldierCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	HeadSocket = "HeadSocket";
	WeaponSocket = "WeaponSocket";
	ArmSocket = "ArmSocket";
	HelmetSocket = "HelmetSocket";
	HeadsetSocket = "HeadsetSocket";
	WeaponBackSocket = "WeaponBackSocket";

	RootComponent = this->GetRootComponent();

	SpringArm = CreateDefaultSubobject <USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArm);

	FPPMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPPMesh"));
	FPPMesh->SetupAttachment(CameraComp);

	WeaponSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("WeaponSpring"));
	WeaponSpringArm->SetupAttachment(FPPMesh);


	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

	SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
	AudioComp->SetupAttachment(GetMesh());

	AudioCompReload = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioCompReload"));
	AudioCompReload->SetupAttachment(GetMesh());

	AudioDamageComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioDamageComp"));
	AudioDamageComp->SetupAttachment(GetMesh());

	// TIMELINE//
	static ConstructorHelpers::FObjectFinder<UCurveFloat> Curve(TEXT("/Game/Blueprints/Granates/C_MyCurve"));
	check(Curve.Succeeded());
	FloatCurve = Curve.Object;

	FieldOfView = 75.f;
	ZoomingTime = 0.2f;
	ZoomInterpSpeed = 7.5f;
	HoloScopeFieldOfView = 30.0f;
	NoScopeFieldOfView = 45.0f;

	bIsSingleFire = false;
	bReloading = false;

	CharacterState = ECharacterState::Idle;
	HoldingWeaponState = EHoldingWeapon::None;
	HoldingAttachmentState = EHoldingAttachment::None;
	LaserEquipState = ELaserAttachment::None;
	MaxUseDistance = 400;

	bWeaponOnBack = false;

	AmountGrenades = 10;

	SetReplicates(true);
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;

	///// VAULTING /////
	MaxHeightForVault = 60;
	bIsAllowClimbing = false;
	bIsAbleToVault = false;


	/////SURVIVAL/////
	FreQOfDrainingHealthWhenLowFood = 3.0f;
	FreQOfDrainingHealthWhenLowDrink = 5.0f;
	amountOfBoostDrink = 30;
	amountOfBoostFood = 40;
	stamina = 100;

}
// Called when the game starts or when spawned
void ASoldierCharacter::BeginPlay()
{
	Super::BeginPlay();

	ASingleplayerGameMode* SingleMode = Cast<ASingleplayerGameMode>(GetWorld()->GetAuthGameMode());
	if (SingleMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("Singleplayer game mode "));
		bSinglePlayerMode = true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Multiplayer game mode "));
		bSinglePlayerMode = false;
	}

	WeaponSpringArm->AttachToComponent(FPPMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);

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

	HealthComp->OnHealthChanged.AddDynamic(this, &ASoldierCharacter::OnHealthChanged);

	if (HealthComp)
	{
		HealthComp->Health = 100;
	}

	/*if (SpringArm)
	{
		SpringArm->AttachToComponent(FPPMesh, FAttachmentTransformRules::SnapToTargetIncludingScale);

	}
	if (CameraComp)
	{
		CameraComp->AttachToComponent(SpringArm, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}*/

	///Calling these function on begin play with timers to avoid ticking
	FindingGrenadeTransform();
	Headbobbing();
	GrenadeTimeline();
	SprintSlowDown();
	UpdateRifleStatus();
	OutOfBreathSound();
	RagdollOnDeath();
	OnFoodLow();


	///CLAMP CAMERA
	APlayerCameraManager * CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (CameraManager)
	{
		CameraManager->ViewPitchMax = 45.0f;
		CameraManager->ViewPitchMin = -90.0f;
	}


}
void ASoldierCharacter::LineTraceItem()
{
	if (Role < ROLE_Authority)
	{
		ServerLineTraceItem();
	}

	FVector start_trace = CameraComp->GetComponentLocation();
	FVector direction = CameraComp->GetComponentRotation().Vector();
	FVector end_trace = start_trace + (direction* MaxUseDistance);
	

	FCollisionQueryParams TraceParams(FName(TEXT("")), true, this);
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;
	TraceParams.AddIgnoredActor(this);

	FHitResult Hit;

	if (GetWorld()->LineTraceSingleByChannel(Hit, start_trace, end_trace, COLLISION_ITEMS, TraceParams))
	{
		//DrawDebugLine(GetWorld(), start_trace, end_trace, FColor::Red, false, 1.0f, 0, 1.0f);

		ABaseWeaponClass* WeaponClass = Cast<ABaseWeaponClass>(Hit.GetActor());
		if (WeaponClass)
		{
			bWantToPickUp = true;
			if (GetWorld()->GetFirstPlayerController()->IsInputKeyDown("E"))
			{
				PickUp(WeaponClass, nullptr,nullptr);
			}
		}

		ABaseAttachmentClass* AttachmentClass = Cast<ABaseAttachmentClass>(Hit.GetActor());
		if (AttachmentClass)
		{
			bWantToPickUp = true;
			if (GetWorld()->GetFirstPlayerController()->IsInputKeyDown("E"))
			{
				PickUp(nullptr, AttachmentClass,nullptr);
			}
		}
		ABaseSurvivalItemClass* SurvItem = Cast<ABaseSurvivalItemClass>(Hit.GetActor());
		if (SurvItem)
		{
			bWantToPickUp = true;
			if (GetWorld()->GetFirstPlayerController()->IsInputKeyDown("E"))
			{
				PickUp(nullptr, nullptr, SurvItem);
			}
		}
	}
	else
	{
		bWantToPickUp = false;
	}
	
}

// Called every frame
void ASoldierCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsLocallyControlled())
	{
		LineTraceItem();
	}

	RagdollOnDeath();
}

// Called to bind functionality to input
void ASoldierCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
		Super::SetupPlayerInputComponent(PlayerInputComponent);
	
		//Keyboard movement
		PlayerInputComponent->BindAxis("MoveForward", this, &ASoldierCharacter::MoveForward);
		PlayerInputComponent->BindAxis("MoveRight", this, &ASoldierCharacter::MoveRight);

		//Mouse look
		PlayerInputComponent->BindAxis("LookUp", this, &ASoldierCharacter::AddPichInput);
		PlayerInputComponent->BindAxis("Turn", this, &ASoldierCharacter::AddYawInput);


		//Crouching
		PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASoldierCharacter::BeginCrouch);
		PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASoldierCharacter::EndCrouch);

		PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &ASoldierCharacter::ZoomIn);
		PlayerInputComponent->BindAction("ZoomIn", IE_Released, this, &ASoldierCharacter::ZoomOut);

		PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASoldierCharacter::StartFire);
		PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASoldierCharacter::StopFire);

		PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASoldierCharacter::Reload);

		PlayerInputComponent->BindAction("FireMode", IE_Pressed, this, &ASoldierCharacter::FireMode);

	//	PlayerInputComponent->BindAction("PickUp", IE_Pressed, this, &ASoldierCharacter::LineTraceItem);

		PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASoldierCharacter::SprintOn);
		PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASoldierCharacter::SprintOff);

		PlayerInputComponent->BindAction("ToogleLaser", IE_Pressed, this, &ASoldierCharacter::TurnOnLaser);

		PlayerInputComponent->BindAction("Vault", IE_Pressed, this, &ASoldierCharacter::Vault);

		PlayerInputComponent->BindAction("Inspect", IE_Pressed, this, &ASoldierCharacter::WeaponInspectionOn);

		PlayerInputComponent->BindAction("Grenade", IE_Pressed, this, &ASoldierCharacter::ThrowGrenade);

		PlayerInputComponent->BindAction("StartDropGun", IE_Pressed, this, &ASoldierCharacter::StartDropGun);


		PlayerInputComponent->BindAction("EatFood", IE_Pressed, this, &ASoldierCharacter::EatFood);
		PlayerInputComponent->BindAction("DrinkWater", IE_Pressed, this, &ASoldierCharacter::DrinkWater);

		PlayerInputComponent->BindAction("TakeOutWeapon", IE_Pressed, this, &ASoldierCharacter::PutWeaponOnBack);

		PlayerInputComponent->BindAction("Respawn", IE_Pressed, this, &ASoldierCharacter::ServerWantToRespawn);
	


}
void ASoldierCharacter::RagdollOnDeath()
{
	if (bDied == true && bDoRagdollOnce == false)
	{
		this->GetMesh()->SetSimulatePhysics(true);
		bDoRagdollOnce = true;
	}
}
void ASoldierCharacter::PutWeaponOnBack()
{
	/*if (Role < ROLE_Authority)
	{
		ServerPutWeaponOnBack();
	}*/

	//TODO Attach 3rd person gun to hands, multipalyer replication
	
	if (CurrentWeapon &&bWeaponOnBack == false)
	{
		CurrentWeapon->SetActorHiddenInGame(true);
		bWeaponOnBack = true;
		bIsWeaponAttached = false;
	}
	else if (CurrentWeapon && bWeaponOnBack == true)
	{
		CurrentWeapon->SetActorHiddenInGame(false);
		bWeaponOnBack = false;
		bIsWeaponAttached = true;
	}
}
void ASoldierCharacter::EatFood()
{

	if (Role < ROLE_Authority)
	{
		ServerEatFood();
	//	return;
	}
	if (amountOfFood > 0)
	{
		if (IsLocallyControlled())
		{
			UGameplayStatics::PlaySound2D(this, EatFoodSound);
		}
		amountOfFood--;

		//USurvivalComponent* SurvivalComp = this->FindComponentByClass<USurvivalComponent>();
		if (SurvivalComp)
		{
			SurvivalComp->Food = SurvivalComp->Food + amountOfBoostFood;
			if (SurvivalComp->Food > 100)
			{
				SurvivalComp->Food = 100;
			}
		}
	}
}
void ASoldierCharacter::DrinkWater()
{

	if (Role < ROLE_Authority)
	{
		ServerDrinkWater();
		//return;
	}
	if (amountOfDrinks > 0)
	{
		if (IsLocallyControlled())
		{
			UGameplayStatics::PlaySound2D(this, DrinkWaterSound);
		}
		amountOfDrinks--;

		//USurvivalComponent* SurvivalComp = this->FindComponentByClass<USurvivalComponent>();
		if (SurvivalComp)
		{
			SurvivalComp->Drink = SurvivalComp->Drink + amountOfBoostDrink;
			if (SurvivalComp->Drink > 100)
			{
				SurvivalComp->Drink = 100;
			}
		}
	}
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
			bIsAbleToVault = true;

			///// CHECKING IF OBJECT IS HIGH ENOUGH ////
			FHitResult Hit2;
			FRotator Rotator = UKismetMathLibrary::MakeRotFromX(WallNormal);
			FVector TempStartLocation2 = UKismetMathLibrary::GetForwardVector(Rotator);
			FVector AlmostStartLocation2 = (TempStartLocation2 * (-10)) + WallLocation;
			FVector StartLocation2 = AlmostStartLocation2 + FVector(0, 0, 200);
			FVector EndLocation2 = StartLocation2 - FVector(0, 0, 200);

			if (GetWorld()->LineTraceSingleByChannel(Hit2, StartLocation2, EndLocation2, COLLISION_TRACE, CollisionParams) && bIsAbleToVault == true)
			{
			//	DrawDebugLine(GetWorld(), StartLocation2, EndLocation2, FColor::Blue, false, 1.0f, 0, 1.0f);
				WallHight = Hit2.ImpactPoint;
				float Test = (WallHight - WallLocation).Z;
				if (Test < MaxHeightForVault)
				{
					bIsObjectTooHigh = false;
					UE_LOG(LogTemp, Warning, TEXT("Object is not to high:%f "), Test);

				}
				else
				{
					bIsObjectTooHigh = true;
					UE_LOG(LogTemp, Warning, TEXT("Object is to high. It is:%f "), Test);
				}
			}

		}
		else
		{
			bIsAbleToVault = false;
		}

		/// GETTING THIRD LINE TRACE FOR THICKNESS TO DECIDE IF VAULT OR CLIMB ///

		FHitResult Hit3;
		FRotator Rotator2 = UKismetMathLibrary::MakeRotFromX(WallNormal);
		FVector TempStartLocation3 = UKismetMathLibrary::GetForwardVector(Rotator2);
		FVector AlmostStartLocation3 = (TempStartLocation3 * (-50)) + WallLocation;
		FVector StartLocation3 = AlmostStartLocation3 + FVector(0, 0, 250);
		FVector EndLocation3 = StartLocation3 - FVector(0, 0, 300);
		if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation3, EndLocation3, COLLISION_TRACE, CollisionParams) && bIsAbleToVault == true)
		{
			//DrawDebugLine(GetWorld(), StartLocation3, EndLocation3, FColor::Yellow, false, 1.0f, 0, 1.0f);
			NextWallHight = Hit3.ImpactPoint;
			bIsAllowClimbing = true;
		}
		else
		{
			bIsAllowClimbing = false;
		}

		//// IF ALL THE TERMS ARE GOOD THEN GO VAULT OR CLIMB ////
		if (bIsAllowClimbing == true && bIsAbleToVault == true && bIsObjectTooHigh == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("Climb"));

			///***Turning off collision when getting on the obstacle***//
			UCapsuleComponent* CharCapsuleComponent = this->FindComponentByClass<UCapsuleComponent>();
			if (CharCapsuleComponent)
			{
				CharCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

			bGoClimb = true;
			bGoVault = false;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_Vault, this, &ASoldierCharacter::ResetVaultTimer, 0.7f, false);
		}
		else if (bIsAbleToVault == true && bIsAllowClimbing == false && bIsObjectTooHigh == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("Vault"));
			bGoClimb = false;
			bGoVault = true;

			///***Turning off collision when getting on the obstacle***//
			UCapsuleComponent* CharCapsuleComponent = this->FindComponentByClass<UCapsuleComponent>();
			if (CharCapsuleComponent)
			{
				CharCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
			bGoVault = false;
			bGoClimb = false;
		}
}
void ASoldierCharacter::ResetVaultTimer()
{
	if (Role < ROLE_Authority)
	{
		ServerResetTimerVault();
	}

	bGoClimb = false;
	bGoVault = false;

	UE_LOG(LogTemp, Warning, TEXT("Resetting timer"));

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Vault);

	UCapsuleComponent* CharCapsuleComponent = this->FindComponentByClass<UCapsuleComponent>();
	if (CharCapsuleComponent)
	{
		CharCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
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
		Laser->GetScalableMeshComponent()->ToggleVisibility();
		Laser->GetPointLightComponent()->ToggleVisibility();
		if (CurrentWeapon)
		{
			Laser->StartLaser(CurrentWeapon);
		}
	}
}
void ASoldierCharacter::PickUp(ABaseWeaponClass* Weapons, ABaseAttachmentClass* Attachments, ABaseSurvivalItemClass* SurvivalItem)
{

	if (Role < ROLE_Authority && !bSinglePlayerMode)
	{
		ServerPickUpItem(Weapons, Attachments, SurvivalItem);
		UE_LOG(LogTemp, Warning, TEXT("Server pick up "));

		return;
	}
	if (IsLocallyControlled())
	{
		AAutomaticRifle* AutoRifle = Cast<AAutomaticRifle>(Weapons);
		if (AutoRifle)
		{
			if (HoldingWeaponState == EHoldingWeapon::None)
			{
				HoldingWeaponState = EHoldingWeapon::A4;

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				
				AutomaticRifle = GetWorld()->SpawnActor<AAutomaticRifle>(AutoRifleClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (AutomaticRifle)
				{
					AutomaticRifle->SetOwner(this);
					AutomaticRifle->AttachToComponent(FPPMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
					AutomaticRifle->GetSkelMeshComp()->bOnlyOwnerSee = true;
					AutomaticRifle->GetSkelMeshComp()->SetAnimInstanceClass(AnimBp);
					AutomaticRifle->GetSphereComp()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					CurrentWeapon = AutomaticRifle;
					bIsWeaponAttached = true;
					AutoRifle->Destroy();

				}

				Rifle_3rd = GetWorld()->SpawnActor<ARifle_3rd>(ThirdWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (Rifle_3rd)
				{
					Rifle_3rd->SetOwner(this);
					Rifle_3rd->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
					Rifle_3rd->GetSkelMeshComp()->bOwnerNoSee = true;
				}
			}

		}
		ASniperRifle* SniperWeapon = Cast<ASniperRifle>(Weapons);
		if (SniperWeapon)
		{
			if (HoldingWeaponState == EHoldingWeapon::None)
			{
				HoldingWeaponState = EHoldingWeapon::Sniper;

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				if (Role == ROLE_Authority)
				{
					SniperRifle = GetWorld()->SpawnActor<ASniperRifle>(SniperRifleClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
					if (SniperRifle)
					{
						SniperRifle->SetOwner(this);
						SniperRifle->AttachToComponent(FPPMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
						SniperRifle->GetSkelMeshComp()->bOnlyOwnerSee = true;
						SniperRifle->GetSkelMeshComp()->SetAnimInstanceClass(AnimBp);
						SniperRifle->GetSphereComp()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
						CurrentWeapon = SniperRifle;
						bIsWeaponAttached = true;
						SniperWeapon->Destroy();

					}
				}
				UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());

			}
		}
		AScarH* ScarH = Cast<AScarH>(Weapons);
		if (ScarH)
		{
			if (HoldingWeaponState == EHoldingWeapon::None)
			{
				HoldingWeaponState = EHoldingWeapon::A4;

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				//if (Role == ROLE_Authority)
				//{
				Scar = GetWorld()->SpawnActor<AScarH>(ScarHRifleClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (Scar)
				{
					Scar->SetOwner(this);
					Scar->AttachToComponent(WeaponSpringArm, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
					Scar->GetSkelMeshComp()->bOnlyOwnerSee = true;
					Scar->GetSphereComp()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					//Turn out postprocess outline
					Scar->GetSkelMeshComp()->SetRenderCustomDepth(false);

					CurrentWeapon = Scar;
					bIsWeaponAttached = true;
					Scar->SetupWeapon(ScarH->GetCurrentAmmoInClip(), ScarH->GetCurrentAmountOfClips());

					ScarH->Destroy();
				}
				
				AK_3rd = GetWorld()->SpawnActor<AAK47_3rd>(AKThirdWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (AK_3rd)
				{
					AK_3rd->SetOwner(this);
					AK_3rd->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
					AK_3rd->GetSkelMeshComp()->bOwnerNoSee = true;
				}
			}
		}
		AM4Rifle* M4Rifle = Cast<AM4Rifle>(Weapons);
		if (M4Rifle)
		{
			if (HoldingWeaponState == EHoldingWeapon::None)
			{
				HoldingWeaponState = EHoldingWeapon::A4;

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				//if (Role == ROLE_Authority)
				//{
				M4 = GetWorld()->SpawnActor<AM4Rifle>(M4RifleClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (M4)
				{
					M4->SetOwner(this);
					M4->AttachToComponent(WeaponSpringArm, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
					M4->GetSkelMeshComp()->bOnlyOwnerSee = true;
					M4->GetSphereComp()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					//Turn out postprocess outline
					M4->GetSkelMeshComp()->SetRenderCustomDepth(false);

					CurrentWeapon = M4;
					bIsWeaponAttached = true;

					M4->SetupWeapon(M4Rifle->GetCurrentAmmoInClip(), M4Rifle->GetCurrentAmountOfClips());

					M4Rifle->Destroy();
				}
				
				M4_3rd = GetWorld()->SpawnActor<AM4_3rd>(M4ThirdWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (M4_3rd)
				{
					M4_3rd->SetOwner(this);
					M4_3rd->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
					M4_3rd->GetSkelMeshComp()->bOwnerNoSee = true;
				}
				
				
			}
		}
		AHoloScope * HoloAttachment = Cast<AHoloScope>(Attachments);
		if (HoloAttachment)
		{
			UE_LOG(LogTemp, Warning, TEXT("Tutaj4"));

			if ((HoldingWeaponState == EHoldingWeapon::A4 || HoldingWeaponState == EHoldingWeapon::Sniper) && HoloEquipState == EHoloAttachment::None)
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
						HoloScope->SetOwner(this);
						HoloScope->GetMeshComponent()->bOnlyOwnerSee = true;
						HoloScope->GetMeshComponent()->SetRenderCustomDepth(false);
						if (CurrentWeapon)
						{
							FName Socket = CurrentWeapon->GetScopeSocketName();
							HoloScope->AttachToComponent(CurrentWeapon->GetSkelMeshComp(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
							CurrentWeapon->SetupHoloScope(HoloScope);
						}
						HoloScope->GetSphereComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
						bIsHoloAttached = true;
						HoloAttachment->Destroy();
					}
				}
				UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
			}
		}
		AGrip* GripAttachment = Cast<AGrip>(Attachments);
		if (GripAttachment)
		{
			if ((HoldingWeaponState == EHoldingWeapon::A4 || HoldingWeaponState == EHoldingWeapon::Sniper) && GripEquipState == EGripAttachment::None)
			{
				HoldingAttachmentState = EHoldingAttachment::Grip;
				GripEquipState = EGripAttachment::Equipped;

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				Grip = GetWorld()->SpawnActor<AGrip>(GripClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (Grip)
				{
					Grip->SetOwner(this);
					Grip->GetMeshComponent()->bOnlyOwnerSee = true;
					Grip->GetMeshComponent()->SetRenderCustomDepth(false);
					if (CurrentWeapon)
					{
						FName GSocket = CurrentWeapon->GetGripSocketName();
						Grip->AttachToComponent(CurrentWeapon->GetSkelMeshComp(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, GSocket);
					}
					Grip->GetSphereComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					bGripAttached = true;
					GripAttachment->Destroy();
				}
				UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
			}
		}
		ALaser* LaserAttachment = Cast<ALaser>(Attachments);
		if (LaserAttachment)
		{
			if (LaserEquipState == ELaserAttachment::None)
			{
				LaserEquipState = ELaserAttachment::Equipped;

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				Laser = GetWorld()->SpawnActor<ALaser>(LaserClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (Laser)
				{
					Laser->SetOwner(this);
					Laser->GetMeshComponent()->bOnlyOwnerSee = true;
					Laser->GetMeshComponent()->SetRenderCustomDepth(false);
					if (CurrentWeapon)
					{
						FName LSocket = CurrentWeapon->GetLaserSocketName();
						Laser->AttachToComponent(CurrentWeapon->GetSkelMeshComp(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, LSocket);
					}
					Laser->GetSphereComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

					bIsLaserAttached = true;
					LaserAttachment->Destroy();
				}
				UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
			}
		}
		AHelmet* HelmetAttachment = Cast<AHelmet>(Attachments);
		if (HelmetAttachment)
		{
			if (HelmetEquipState == EHelmetAttachment::None)
			{
				HelmetEquipState = EHelmetAttachment::Equipped;

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				Helmet = GetWorld()->SpawnActor<AHelmet>(HelmetClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (Helmet)
				{
					Helmet->SetOwner(this);
					Helmet->GetMeshComponent()->SetRenderCustomDepth(false);
					Helmet->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HelmetSocket);
					Helmet->GetSphereComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					bIsHelmetAttached = true;
					HelmetAttachment->Destroy();
				}
				UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
			}

		}
		AHeadset* HeadsetAttachment = Cast<AHeadset>(Attachments);
		if (HeadsetAttachment)
		{
			if (HeadsetEquipState == EHeadsetAttachment::None)
			{
				HeadsetEquipState = EHeadsetAttachment::Equipped;

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				Headset = GetWorld()->SpawnActor<AHeadset>(HeadsetClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (Headset)
				{
					Headset->SetOwner(this);
					Headset->GetMeshComponent()->SetRenderCustomDepth(false);
					Headset->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeadsetSocket);
					Headset->GetMeshComponent()->ToggleActive();
					Headset->GetSphereComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					bIsHeadsetAttached = true;
					HeadsetAttachment->Destroy();
				}
				UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
			}
		}
		AMagazine* Magazine = Cast<AMagazine>(Attachments);
		if (Magazine)
		{
			if (CurrentWeapon)
			{
				CurrentWeapon->AddMagazine();
			}
			Magazine->Destroy();
		}


		//	if (Role == ROLE_Authority)
		//	{
		//		Rifle_3rd = GetWorld()->SpawnActor<ARifle_3rd>(ThirdWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		//		if (Rifle_3rd)
		//		{
		//			Rifle_3rd->SetOwner(this);
		//			Rifle_3rd->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
		//		}
		//	}
		//	UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());

		//	/*if (Role == ROLE_Authority)
		//	{
		//		Rifle_3rd = GetWorld()->SpawnActor<ARifle_3rd>(ThirdWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		//		if (Rifle_3rd)
		//		{
		//			Rifle_3rd->SetOwner(this);
		//			Rifle_3rd->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
		//		}
		//	}*/
		//	UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());


		///SURVIVAL STUFF///
		ADrink* DrinkActor = Cast<ADrink>(SurvivalItem);
		if (DrinkActor)
		{
			amountOfDrinks++;

			DrinkActor->Destroy();
		}
		AFood* FoodActor = Cast<AFood>(SurvivalItem);
		if (FoodActor)
		{
			amountOfFood++;

			FoodActor->Destroy();
		}

		AWater* WaterActor = Cast<AWater>(SurvivalItem);
		if (WaterActor)
		{

			APlayerController* PC = Cast<APlayerController>(GetController());
			if (PC)
			{
				DisableInput(PC);
				FTimerDelegate DelegateFunc = FTimerDelegate::CreateUObject(this, &ASoldierCharacter::EndDrinkFromPond, PC);
				GetWorldTimerManager().SetTimer(DrinkFromPondTimer, DelegateFunc, 2.5f, false);
			}

			if (IsLocallyControlled())
			{
				UGameplayStatics::PlaySound2D(this, DrinkFromPondSound);
			}

		}
	}

	
		
}
void ASoldierCharacter::EndDrinkFromPond(APlayerController* PC)
{
	EnableInput(PC);
	if (SurvivalComp)
	{
		SurvivalComp->Drink = SurvivalComp->Drink + amountOfBoostDrink;
		if (SurvivalComp->Drink > 100)
		{
			SurvivalComp->Drink = 100;
		}
	}
}
void ASoldierCharacter::MoveForward(float Value)
{
	if (bDied != true)
	{
		AddMovementInput(GetActorForwardVector()*Value);
	}
}
void ASoldierCharacter::MoveRight(float Value)
{
	if (bDied != true)
	{
		if (!bIsSprinting)
		{
			AddMovementInput(GetActorRightVector() * Value);
		}
	}
}
void ASoldierCharacter::AddPichInput(float Value)
{
	if (bDied != true)
	{
		AddControllerPitchInput(Value);
	}
}
void ASoldierCharacter::AddYawInput(float Value)
{
	if (bDied != true)
	{
		AddControllerYawInput(Value);
	}
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
			bIsCrouching = true;
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
	bIsCrouching = false;
	UnCrouch();

}
void ASoldierCharacter::CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult)
{
	UCameraComponent* FirstPersonCamera = FindComponentByClass<UCameraComponent>();
	if (FirstPersonCamera && FirstPersonCamera->IsActive())
	{
		FirstPersonCamera->GetCameraView(DeltaTime, OutResult);

		// Get the camera location
		FVector CameraLocation = FirstPersonCamera->GetComponentLocation();

		// Get the weapon sight transform
		FTransform SightTransform;
		if (HoloScope && bIsHoloAttached)
		{
			SightTransform = HoloScope->GetMeshComponent()->GetSocketTransform(FName(TEXT("LineSocket")));
		}
		else if(CurrentWeapon)
		{
			SightTransform = CurrentWeapon->GetSkelMeshComp()->GetSocketTransform(FName(TEXT("LineSocket")));

		}
		FVector SightLocation = SightTransform.GetLocation();
		FRotator SightRotation = SightTransform.GetRotation().Rotator();

		// Get the sight location
		FVector SightDirection = FRotationMatrix(SightRotation).GetScaledAxis(EAxis::X);
		FVector DirectionToSight = SightLocation - CameraLocation;
		float DirectionToSightDot = FVector::DotProduct(DirectionToSight.GetSafeNormal(), SightDirection);
		float DirectionToSightLen = 58.f;
		//float DirectionToSightLen = DirectionToSight.Size();
		float DirectionToSightDotLen = DirectionToSightDot * DirectionToSightLen;
		DirectionToSightDotLen = UKismetMathLibrary::Clamp(DirectionToSightDotLen, 56.f, 56.f);
		FVector SightDirectionDotLen = SightDirection * DirectionToSightDotLen;
		FVector SightTargetLocation = SightLocation - SightDirectionDotLen;


		//AimAlpha = bZooming ? 1.0f : 0.0f;
		
		if (bZooming)
		{
			AimAlpha = UKismetMathLibrary::FInterpTo(AimAlpha, 1.0f, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), ZoomInterpSpeed);

			if (HoloScope && bIsHoloAttached)
			{
				
				FieldOfView = UKismetMathLibrary::FInterpTo(FieldOfView, HoloScopeFieldOfView, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), ZoomInterpSpeed);

				FirstPersonCamera->SetFieldOfView(FieldOfView);
			}
			else
			{
				FieldOfView = UKismetMathLibrary::FInterpTo(FieldOfView, NoScopeFieldOfView, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), ZoomInterpSpeed);

				FirstPersonCamera->SetFieldOfView(FieldOfView);
			}

			OutResult.Location = CameraLocation + AimAlpha * (SightTargetLocation - CameraLocation);
		}
		else
		{
			AimAlpha = UKismetMathLibrary::FInterpTo(AimAlpha, 0.0f, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), ZoomInterpSpeed);

			FieldOfView = UKismetMathLibrary::FInterpTo(FieldOfView, 75.0f, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), ZoomInterpSpeed);

			FirstPersonCamera->SetFieldOfView(FieldOfView);

			OutResult.Location = CameraLocation + AimAlpha * (SightTargetLocation - CameraLocation);
		}
	}
	else
	{
		GetActorEyesViewPoint(OutResult.Location, OutResult.Rotation);
	}
}
void ASoldierCharacter::ZoomIn()
{

	UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
	if (CurrentWeapon)
	{
		if (CurrentWeapon->GetDistanceToObject() < 0.35f && bIsWeaponAttached && MoveComp && !bIsInspecting && bWeaponOnBack != true)
		{
			MoveComp->MaxWalkSpeed = 250.0f;

			bZooming = true;
		}
	}
	if (WeaponSpringArm)
	{
		WeaponSpringArm->bEnableCameraRotationLag = false;
	}
}
void ASoldierCharacter::ZoomOut()
{
	UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
	if (MoveComp && bIsWeaponAttached)
	{
		MoveComp->MaxWalkSpeed = 300.0f;

		bZooming = false;
	}
	if (WeaponSpringArm)
	{
		WeaponSpringArm->bEnableCameraRotationLag = true;
	}
}
void ASoldierCharacter::StartFire()
{
	if (!bIsSprinting && bWeaponOnBack == false && bDied != true)
	{
		CharacterState = ECharacterState::Firing;

		bIsFiring = true;

		if (bIsSingleFire == false)
		{
			if (CurrentWeapon)
			{
				if (CurrentWeapon->GetCurrentAmmoInClip() > 0 && CurrentWeapon->CurrentState != EWeaponState::Reloading )
				{
					bFireAnimation = true;

					CurrentWeapon->StartFire();

					APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
					if (PC && bZooming == true)
					{
						PC->ClientPlayCameraShake(CameShakeZoomClass);
					}
					else
					{
						PC->ClientPlayCameraShake(CameShakeHipClass);
					}
				}
				else if (CurrentWeapon->GetCurrentAmmoInClip() == 0)
				{
					if (IsLocallyControlled())
					{
						UGameplayStatics::PlaySound2D(this, NoAmmoSound);
					}
				}
			}
		}
		else
		{
			if (CurrentWeapon)
			{
				if (CurrentWeapon->GetCurrentAmmoInClip() > 0 && CurrentWeapon->CurrentState != EWeaponState::Reloading)
				{
					bFireAnimation = true;
					CurrentWeapon->Fire();
					APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
					if (PC && bZooming == true)
					{
						PC->ClientPlayCameraShake(CameShakeZoomClass);
					}
					else
					{
						PC->ClientPlayCameraShake(CameShakeHipClass);
					}
				}
				else if (CurrentWeapon->GetCurrentAmmoInClip() == 0)
				{
					if (IsLocallyControlled())
					{
						UGameplayStatics::PlaySound2D(this, NoAmmoSound);
					}
				}

			}
		}
	}
}
void ASoldierCharacter::StopFire()
{
	CharacterState = ECharacterState::Idle;
	bIsFiring = false;

	bFireAnimation = false;

	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}
void ASoldierCharacter::SprintOn()
{
	if (Role < ROLE_Authority)
	{
		ServerSprintOn();
	}
	if (stamina == 0) SprintOff();
	if (stamina < 10)
	{
		UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
		if (MoveComp)
		{
			MoveComp->MaxWalkSpeed = 300.0f;
		}
	}
	else if (stamina > 10 && bIsSprinting == false)
	{
		if (GetVelocity().Size()>0)
		{
			SprintProgressBar();
			bIsSprinting = true;
			UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
			if (MoveComp)
			{
				MoveComp->MaxWalkSpeed = 400.0f;
			}
		}
	}

}
void ASoldierCharacter::SprintOff()
{

	if (Role < ROLE_Authority)
	{
		ServerSprintOff();
	}

	SprintProgressBar();

	bIsSprinting = false;
	UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
	if (MoveComp)
	{
		MoveComp->MaxWalkSpeed = 300.0f;
	}
	
}
void ASoldierCharacter::SprintProgressBar()
{
	if (bIsSprinting == true)
	{
		GetWorldTimerManager().SetTimer(SprintTimerHandle, this, &ASoldierCharacter::SprintProgressBar, 0.2f, false);
		stamina--;
		if (stamina == 0) bIsSprinting = false;
		//UE_LOG(LogTemp, Warning, TEXT("Sprinting now, bar is: %f"), stamina);
	}
	if (bIsSprinting == false)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Resting spring now, bar is: %f"), stamina);

		if (stamina < 100)
		{
			stamina++;
		}
		else if (stamina == 100) GetWorldTimerManager().ClearTimer(SprintRestingTimerHandle);

		GetWorldTimerManager().ClearTimer(SprintTimerHandle);

		GetWorldTimerManager().SetTimer(SprintRestingTimerHandle, this, &ASoldierCharacter::SprintProgressBar, 0.5f, false);
		
	}

}
void ASoldierCharacter::SprintSlowDown()
{
	////SLOW DOWN WHEN OUT OF STAMINA
	if (stamina == 1)
	{
		bIsSprinting = false;
		UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
		if (MoveComp)
		{
			MoveComp->MaxWalkSpeed = 300.0f;
		}
	}

	GetWorldTimerManager().SetTimer(SlowDownSprintTimer, this, &ASoldierCharacter::SprintSlowDown, 0.5f, false);
}
void ASoldierCharacter::OutOfBreathSound()
{
	
	if (IsLocallyControlled())
	{
		if (stamina < 10 && bResetBreath == false)
		{
			UGameplayStatics::PlaySound2D(this, OutOfBreath);
			GetWorldTimerManager().SetTimer(OutOfBreathTimer, this, &ASoldierCharacter::OutOfBreathReset, 9.0f, false);
			bResetBreath = true;
		}
	}

	GetWorldTimerManager().SetTimer(UpdateBreath, this, &ASoldierCharacter::OutOfBreathSound, 1.0f, false);
}
void ASoldierCharacter::OutOfBreathReset()
{
	bResetBreath = false;
}
void ASoldierCharacter::Headbobbing()
{
	if (bIsSprinting == true)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(CameraSprintShake, 0.5f);
		}
	}
	if (GetVelocity().Size() > 0)
	{
		if (bIsSprinting == false)
		{
			APlayerController* PC = Cast<APlayerController>(GetController());
			if (PC)
			{
				PC->ClientPlayCameraShake(CameraSprintShake, 0.25f);
			}
		}
	}

	GetWorldTimerManager().SetTimer(HBobingTimer, this, &ASoldierCharacter::Headbobbing, 0.1f, false);

}
void ASoldierCharacter::Reload()
{
	if (Role < ROLE_Authority)
	{
		ServerReload();
	}
	if (CharacterState == ECharacterState::Idle && (SoldierCurrentClips > 0) && bDied != true)
	{
		if (CurrentWeapon)
		{
			CharacterState = ECharacterState::Reloading;
			bReloading = true;
			CurrentWeapon->StartReload();
			//ServerReloadingSound();
			GetWorldTimerManager().SetTimer(ReloadTimer, this, &ASoldierCharacter::StopReload, 2.167f, false);

			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ReloadSound, this->GetActorLocation());
		}
	}
}
void ASoldierCharacter::StopReload()
{
	if (CharacterState == ECharacterState::Reloading)
	{
		CharacterState = ECharacterState::Idle;
		bReloading = false;
		GetWorldTimerManager().ClearTimer(ReloadTimer);
	}
}
void ASoldierCharacter::FireMode()
{
	//* NO SWITCHING FIRE MODES WHEN FIRING *//
	if (CharacterState != ECharacterState::Firing)
	{
		if (bIsSingleFire == false)
		{
			bIsSingleFire = true;
		}
		else
		{
			bIsSingleFire = false;
		}
	}
	
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), FiremodeSwitch, this->GetActorLocation());
	
}
void ASoldierCharacter::UpdateRifleStatus()
{
	if (CurrentWeapon)
	{
		SoldierCurrentAmmoInClip = CurrentWeapon->GetCurrentAmmoInClip();
		SoldierCurrentAmmo = CurrentWeapon->GetAllAmmo();
		SoldierCurrentClips = CurrentWeapon->GetCurrentAmountOfClips();
	}
	if(!bIsWeaponAttached)
	{
		SoldierCurrentAmmoInClip = 0;
		SoldierCurrentAmmo = 0;
		SoldierCurrentClips = 0;
	}
	GetWorldTimerManager().SetTimer(UpdateRifleTimer, this, &ASoldierCharacter::UpdateRifleStatus, 0.05f, false);
}
void ASoldierCharacter::WeaponInspectionOn()
{
	if (bIsInspecting != true)
	{
		bIsInspecting = true;
		GetWorld()->GetTimerManager().SetTimer(InspectionTimer, this, &ASoldierCharacter::WeaponInspectionOff, 0.733f);
	}
}
void ASoldierCharacter::WeaponInspectionOff()
{
	bIsInspecting = false;
}
void ASoldierCharacter::FindingGrenadeTransform()
{
	if (GrenadeStartLocation)
	{
		STL = GrenadeStartLocation->GetComponentLocation();
		STR = GrenadeStartLocation->GetComponentRotation();
	}

	GetWorldTimerManager().SetTimer(TransformHandle, this, &ASoldierCharacter::FindingGrenadeTransform, 0.5f, false);
}
void ASoldierCharacter::GrenadeTimeline()
{
	if (MyTimeline != NULL)
	{
		float DeltaTime = UGameplayStatics::GetWorldDeltaSeconds(this);
		MyTimeline->TickComponent(DeltaTime, ELevelTick::LEVELTICK_TimeOnly, NULL);
	}

	GetWorldTimerManager().SetTimer(TimelineHandle, this, &ASoldierCharacter::GrenadeTimeline, 0.5f, false);
}
void ASoldierCharacter::ThrowGrenade()
{
	if (AmountGrenades > 0 && bDied != true)
	{
		AmountGrenades--;
		if (IsLocallyControlled())
		{
			SpawnGrenade(STL, STR);
		}
	}
}
void ASoldierCharacter::SpawnGrenade(FVector StartingLocation, FRotator StartingRotation)
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
void ASoldierCharacter::Flashbang(float ThrowDistance, FVector PlayerFacingAngle)
{
	if (IsLocallyControlled())
	{
		FlashAmount = (UKismetMathLibrary::NormalizeToRange(Distance, 20.0f, 100.0f) / 10.0f) * (-1.0f);
		if (Distance < 2000.0f)
		{
			AngleFromFlash(FacingAngle);
			if (bIsFacing == true)
			{
				FlashAmount = 0.5f;
			}
			UKismetMaterialLibrary::SetScalarParameterValue(this, MaterialCollection, "Flash_Value", FlashAmount);
			GetWorldTimerManager().SetTimer(Timer_Flash, this, &ASoldierCharacter::PlayTimeline, 1.0f);
		}
	}
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
	GetWorldTimerManager().ClearTimer(Timer_Flash);

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
	
			bIsFacing = UKismetMathLibrary::BooleanOR(DiffRotation >= 90.0f, DiffRotation <= -90.0f);
		}
	}
}
void ASoldierCharacter::OnHealthChanged(UHealthComponent * OwningHealthComp, float Health, float HealthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	///TODO Destroy heartbeat sound death for client

	if (Health <= 0.0f && !bDied)
	{
		if (IsLocallyControlled())
		{
			if (AudioComp)
			{
				AudioComp->DestroyComponent();
			}
		}

		bDied = true;
	}
	if (HealthComp->Health <= 40.0f && bDied == false)
	{
		if (AudioComp)
		{
			AudioComp->Play();
		}
	}

	AudioDamageComp->Play(0.0f);
}
void ASoldierCharacter::OnFoodLow()
{
	if (Role < ROLE_Authority)
	{
		ServerOnFoodLow();
	}
	if (SurvivalComp)
	{
		if (SurvivalComp->Food == 0)
		{
			UHealthComponent* HealthComponent = this->FindComponentByClass<UHealthComponent>();
			if (HealthComponent)
			{
				if (HealthComponent->Health > 0)
				{
					HealthComponent->Health = HealthComp->Health - 1;
				}
				if (HealthComponent->Health == 0) bDied = true;
			}
			GetWorldTimerManager().SetTimer(FoodLowTimer, this, &ASoldierCharacter::OnFoodLow, FreQOfDrainingHealthWhenLowFood, false);
		}
		if (SurvivalComp->Food > 0)
		{
			SurvivalComp->OnRep_Food();
		}
	}

	GetWorldTimerManager().SetTimer(UpdateSurvivalComponent, this, &ASoldierCharacter::OnFoodLow, 10.0f, false);
	
}
void ASoldierCharacter::OnDrinkLow()
{
	if (Role < ROLE_Authority)
	{
		ServerOnDrinkLow();
		return;
	}

	if (SurvivalComp)
	{
		if (SurvivalComp->Drink == 0)
		{
			UHealthComponent* HealthComponent = this->FindComponentByClass<UHealthComponent>();
			if (HealthComponent)
			{
				if (HealthComponent->Health > 0)
				{
					HealthComponent->Health = HealthComp->Health - 1;
				}
				if (HealthComponent->Health == 0) bDied = true;
			}
			GetWorldTimerManager().SetTimer(DrinkLowTimer, this, &ASoldierCharacter::OnDrinkLow, FreQOfDrainingHealthWhenLowDrink, false);
		}
		if (SurvivalComp->Drink > 0)
		{
			SurvivalComp->OnRep_Drink();
		}
	}

	GetWorldTimerManager().SetTimer(UpdateSurvivalComponent, this, &ASoldierCharacter::OnDrinkLow, 8.0f, false);
}
void ASoldierCharacter::PlayHitSound(FName SurfaceHit)
{
	if (IsLocallyControlled())
	{
		if (SurfaceHit == "Normal")
		{
			UGameplayStatics::PlaySound2D(this, HitSound);
		}
		else if (SurfaceHit == "Head")
		{
			UGameplayStatics::PlaySound2D(this, HeadshotSound);
		}
		else if (SurfaceHit == "Helmet")
		{
			UGameplayStatics::PlaySound2D(this, HelmetSound);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No surface detected"));
		}
	}
}
void ASoldierCharacter::StartDropGun()
{
	if (CurrentWeapon)
	{
		bDropGun = true;

		GetWorldTimerManager().SetTimer(DropTimer, this, &ASoldierCharacter::EndDropGun, 0.667f);
	}

}
void ASoldierCharacter::EndDropGun()
{
	bDropGun = false;

	if (CurrentWeapon)
	{
		bIsWeaponAttached = false;
		AScarH* ScarH = Cast<AScarH>(CurrentWeapon);
		if (ScarH)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Scar = GetWorld()->SpawnActor<AScarH>(ScarHRifleClass, this->GetActorLocation(), this->GetActorRotation(), SpawnParams);
			if (Scar)
			{
				FVector LocationVector = CameraComp->GetComponentLocation();
				FVector RotationVector = CameraComp->GetComponentRotation().Vector();
				FVector DirectionVector = LocationVector + RotationVector;
				Scar->GetSkelMeshComp()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
				Scar->GetSkelMeshComp()->SetCollisionProfileName("Pawn");
				Scar->GetSkelMeshComp()->SetSimulatePhysics(true);
				Scar->GetSkelMeshComp()->AddImpulse(DirectionVector* SpawnImpulse, NAME_None, true);
				Scar->SetupWeapon(SoldierCurrentAmmoInClip, SoldierCurrentClips);
			}
		}
		AM4Rifle*  M4Rifle = Cast<AM4Rifle>(CurrentWeapon);
		if (M4Rifle)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			M4 = GetWorld()->SpawnActor<AM4Rifle>(M4RifleClass, this->GetActorLocation(), this->GetActorRotation(), SpawnParams);
			if (M4)
			{
				FVector LocationVector = CameraComp->GetComponentLocation();
				FVector RotationVector = CameraComp->GetComponentRotation().Vector();
				FVector DirectionVector = LocationVector + RotationVector;
				M4->GetSkelMeshComp()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
				M4->GetSkelMeshComp()->SetCollisionProfileName("Pawn");
				M4->GetSkelMeshComp()->SetSimulatePhysics(true);
				M4->GetSkelMeshComp()->AddImpulse(DirectionVector* SpawnImpulse, NAME_None, true);
				M4->SetupWeapon(SoldierCurrentAmmoInClip, SoldierCurrentClips);
			}
		}
		if (Grip)
		{
			bGripAttached = false;
			GripEquipState = EGripAttachment::None;
			Grip->Destroy();
		}
		if (HoloScope)
		{
			bIsHoloAttached = false;
			HoloEquipState = EHoloAttachment::None;
			HoloScope->Destroy();
		}
		
		CurrentWeapon->Destroy();

		HoldingWeaponState = EHoldingWeapon::None;

	}
	if (AK_3rd)
	{
		AK_3rd->Destroy();
	}
	if(M4_3rd)
	{
		M4_3rd->Destroy();
	}

	GetWorldTimerManager().ClearTimer(DropTimer);
}
bool ASoldierCharacter::GetbDied()
{
	return bDied;
}
bool ASoldierCharacter::GetbZooming()
{
	return bZooming;
}
bool ASoldierCharacter::GetbGripAttached()
{
	return bGripAttached;
}
bool ASoldierCharacter::GetbWantToRespawn()
{
	return bWantsToRepawn;
}
bool ASoldierCharacter::GetbWantToPickUp()
{
	return bWantToPickUp;
}
bool ASoldierCharacter::GetbIsCrouching()
{
	return bIsCrouching;
}
void ASoldierCharacter::SetbZooming(bool SetZoom)
{
	bZooming = SetZoom;
}
USkeletalMeshComponent* ASoldierCharacter::GetFPPMesh()
{
	return FPPMesh;
}
void ASoldierCharacter::ServerDrinkWater_Implementation()
{
	DrinkWater();
}
bool ASoldierCharacter::ServerDrinkWater_Validate()
{
	return true;
}
void ASoldierCharacter::ServerEatFood_Implementation()
{
	EatFood();
}
bool ASoldierCharacter::ServerEatFood_Validate()
{
	return true;
}
void ASoldierCharacter::ServerWantToRespawn_Implementation()
{
	///ASK SERVER TO RESPAWN

	UE_LOG(LogTemp, Warning, TEXT("Trying to respawn1"));
	if (bDied == true)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trying to respawn2"));

		bWantsToRepawn = true;
		if (bWantsToRepawn == true)
		{
			UE_LOG(LogTemp, Warning, TEXT("Trying to respawn3"));
			AAimMapGameModeBase* GM = Cast<AAimMapGameModeBase>(GetWorld()->GetAuthGameMode());
			if (GM)
			{
				GM->RespawningPlayer();
				this->Destroy();
			}
		}
	}
}
void ASoldierCharacter::ServerPickUpItem_Implementation(ABaseWeaponClass* Weapons, ABaseAttachmentClass* Attachments, ABaseSurvivalItemClass* SurvivalItem)
{
	AAutomaticRifle* AutoRifle = Cast<AAutomaticRifle>(Weapons);
	if (AutoRifle)
	{
		if (HoldingWeaponState == EHoldingWeapon::None)
		{
			HoldingWeaponState = EHoldingWeapon::A4;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			//if (Role == ROLE_Authority)
			//{
			AutomaticRifle = GetWorld()->SpawnActor<AAutomaticRifle>(AutoRifleClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (AutomaticRifle)
			{
				AutomaticRifle->SetOwner(this);
				AutomaticRifle->AttachToComponent(FPPMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
				AutomaticRifle->GetSkelMeshComp()->bOnlyOwnerSee = true;
				AutomaticRifle->GetSkelMeshComp()->SetAnimInstanceClass(AnimBp);
				AutomaticRifle->GetSphereComp()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				CurrentWeapon = AutomaticRifle;
				bIsWeaponAttached = true;
				AutoRifle->Destroy();

			}

			Rifle_3rd = GetWorld()->SpawnActor<ARifle_3rd>(ThirdWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (Rifle_3rd)
			{
				Rifle_3rd->SetOwner(this);
				Rifle_3rd->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
				Rifle_3rd->GetSkelMeshComp()->bOwnerNoSee = true;
			}
		}

	}
	ASniperRifle* SniperWeapon = Cast<ASniperRifle>(Weapons);
	if (SniperWeapon)
	{
		if (HoldingWeaponState == EHoldingWeapon::None)
		{
			HoldingWeaponState = EHoldingWeapon::Sniper;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (Role == ROLE_Authority)
			{
				SniperRifle = GetWorld()->SpawnActor<ASniperRifle>(SniperRifleClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (SniperRifle)
				{
					SniperRifle->SetOwner(this);
					SniperRifle->AttachToComponent(FPPMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
					SniperRifle->GetSkelMeshComp()->bOnlyOwnerSee = true;
					SniperRifle->GetSkelMeshComp()->SetAnimInstanceClass(AnimBp);
					SniperRifle->GetSphereComp()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					CurrentWeapon = SniperRifle;
					bIsWeaponAttached = true;
					SniperWeapon->Destroy();

				}
			}
			UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());

		}
	}
	AScarH* ScarH = Cast<AScarH>(Weapons);
	if (ScarH)
	{
		if (HoldingWeaponState == EHoldingWeapon::None)
		{
			HoldingWeaponState = EHoldingWeapon::A4;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			//if (Role == ROLE_Authority)
			//{
			Scar = GetWorld()->SpawnActor<AScarH>(ScarHRifleClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (Scar)
			{
				Scar->SetOwner(this);
				Scar->AttachToComponent(FPPMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
				Scar->GetSkelMeshComp()->bOnlyOwnerSee = true;
				//Scar->GetSkelMeshComp()->SetAnimInstanceClass(AnimBp);
				Scar->GetSphereComp()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				CurrentWeapon = Scar;
				bIsWeaponAttached = true;
				ScarH->Destroy();
			}

			AK_3rd = GetWorld()->SpawnActor<AAK47_3rd>(AKThirdWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (AK_3rd)
			{
				AK_3rd->SetOwner(this);
				AK_3rd->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
				AK_3rd->GetSkelMeshComp()->bOwnerNoSee = true;
			}
		}
	}
	AM4Rifle* M4Rifle = Cast<AM4Rifle>(Weapons);
	if (M4Rifle)
	{
		if (HoldingWeaponState == EHoldingWeapon::None)
		{
			HoldingWeaponState = EHoldingWeapon::A4;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			//if (Role == ROLE_Authority)
			//{
			M4 = GetWorld()->SpawnActor<AM4Rifle>(M4RifleClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (M4)
			{
				M4->SetOwner(this);
				M4->AttachToComponent(FPPMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
				M4->GetSkelMeshComp()->bOnlyOwnerSee = true;
				//Scar->GetSkelMeshComp()->SetAnimInstanceClass(AnimBp);
				M4->GetSphereComp()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				CurrentWeapon = M4;
				bIsWeaponAttached = true;
				M4Rifle->Destroy();
			}
		}
	}
	AHoloScope * HoloAttachment = Cast<AHoloScope>(Attachments);
	if (HoloAttachment)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tutaj4"));

		if ((HoldingWeaponState == EHoldingWeapon::A4 || HoldingWeaponState == EHoldingWeapon::Sniper) && HoloEquipState == EHoloAttachment::None)
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
					HoloScope->SetOwner(this);
					HoloScope->GetMeshComponent()->bOnlyOwnerSee = true;
					HoloScope->GetMeshComponent()->SetRenderCustomDepth(false);
					if (CurrentWeapon)
					{
						FName Socket = CurrentWeapon->GetScopeSocketName();
						HoloScope->AttachToComponent(CurrentWeapon->GetSkelMeshComp(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
					}
					HoloScope->GetSphereComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					bIsHoloAttached = true;
					HoloAttachment->Destroy();
				}
			}
			UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
		}
	}
	AGrip* GripAttachment = Cast<AGrip>(Attachments);
	if (GripAttachment)
	{
		if ((HoldingWeaponState == EHoldingWeapon::A4 || HoldingWeaponState == EHoldingWeapon::Sniper) && GripEquipState == EGripAttachment::None)
		{
			HoldingAttachmentState = EHoldingAttachment::Grip;
			GripEquipState = EGripAttachment::Equipped;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			Grip = GetWorld()->SpawnActor<AGrip>(GripClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (Grip)
			{
				Grip->SetOwner(this);
				Grip->GetMeshComponent()->bOnlyOwnerSee = true;
				Grip->GetMeshComponent()->SetRenderCustomDepth(false);
				if (CurrentWeapon)
				{
					FName GSocket = CurrentWeapon->GetGripSocketName();
					Grip->AttachToComponent(CurrentWeapon->GetSkelMeshComp(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, GSocket);
				}
				Grip->GetSphereComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				bGripAttached = true;
				GripAttachment->Destroy();
			}
			UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
		}
	}
	ALaser* LaserAttachment = Cast<ALaser>(Attachments);
	if (LaserAttachment)
	{
		if (LaserEquipState == ELaserAttachment::None)
		{
			LaserEquipState = ELaserAttachment::Equipped;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			Laser = GetWorld()->SpawnActor<ALaser>(LaserClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (Laser)
			{
				Laser->SetOwner(this);
				Laser->GetMeshComponent()->bOnlyOwnerSee = true;
				Laser->GetMeshComponent()->SetRenderCustomDepth(false);
				if (CurrentWeapon)
				{
					FName LSocket = CurrentWeapon->GetLaserSocketName();
					Laser->AttachToComponent(CurrentWeapon->GetSkelMeshComp(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, LSocket);
				}
				Laser->GetSphereComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

				bIsLaserAttached = true;
				LaserAttachment->Destroy();
			}
			UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
		}
	}
	AHelmet* HelmetAttachment = Cast<AHelmet>(Attachments);
	if (HelmetAttachment)
	{
		if (HelmetEquipState == EHelmetAttachment::None)
		{
			HelmetEquipState = EHelmetAttachment::Equipped;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			Helmet = GetWorld()->SpawnActor<AHelmet>(HelmetClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (Helmet)
			{
				Helmet->SetOwner(this);
				Helmet->GetMeshComponent()->SetRenderCustomDepth(false);
				Helmet->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HelmetSocket);
				Helmet->GetSphereComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				bIsHelmetAttached = true;
				HelmetAttachment->Destroy();
			}
			UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
		}

	}
	AHeadset* HeadsetAttachment = Cast<AHeadset>(Attachments);
	if (HeadsetAttachment)
	{
		if (HeadsetEquipState == EHeadsetAttachment::None)
		{
			HeadsetEquipState = EHeadsetAttachment::Equipped;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			Headset = GetWorld()->SpawnActor<AHeadset>(HeadsetClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (Headset)
			{
				Headset->SetOwner(this);
				Headset->GetMeshComponent()->SetRenderCustomDepth(false);
				Headset->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeadsetSocket);
				Headset->GetMeshComponent()->ToggleActive();
				Headset->GetSphereComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				bIsHeadsetAttached = true;
				HeadsetAttachment->Destroy();
			}
			UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
		}
	}
	AMagazine* Magazine = Cast<AMagazine>(Attachments);
	if (Magazine)
	{
		if (CurrentWeapon)
		{
			CurrentWeapon->AddMagazine();
		}
		Magazine->Destroy();
	}


	//	if (Role == ROLE_Authority)
	//	{
	//		Rifle_3rd = GetWorld()->SpawnActor<ARifle_3rd>(ThirdWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	//		if (Rifle_3rd)
	//		{
	//			Rifle_3rd->SetOwner(this);
	//			Rifle_3rd->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
	//		}
	//	}
	//	UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());

	//	/*if (Role == ROLE_Authority)
	//	{
	//		Rifle_3rd = GetWorld()->SpawnActor<ARifle_3rd>(ThirdWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	//		if (Rifle_3rd)
	//		{
	//			Rifle_3rd->SetOwner(this);
	//			Rifle_3rd->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
	//		}
	//	}*/
	//	UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());


	///SURVIVAL STUFF///
	ADrink* DrinkActor = Cast<ADrink>(SurvivalItem);
	if (DrinkActor)
	{
		amountOfDrinks++;

		DrinkActor->Destroy();
	}
	AFood* FoodActor = Cast<AFood>(SurvivalItem);
	if (FoodActor)
	{
		amountOfFood++;

		FoodActor->Destroy();
	}

	AWater* WaterActor = Cast<AWater>(SurvivalItem);
	if (WaterActor)
	{

		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			DisableInput(PC);
			FTimerDelegate DelegateFunc = FTimerDelegate::CreateUObject(this, &ASoldierCharacter::EndDrinkFromPond, PC);
			GetWorldTimerManager().SetTimer(DrinkFromPondTimer, DelegateFunc, 2.5f, false);
		}

		if (IsLocallyControlled())
		{
			UGameplayStatics::PlaySound2D(this, DrinkFromPondSound);
		}

	}

	//PickUp(Weapons,Attachments,SurvivalItem);
}
bool ASoldierCharacter::ServerPickUpItem_Validate(ABaseWeaponClass* Weapons, ABaseAttachmentClass* Attachments, ABaseSurvivalItemClass* SurvivalItem)
{
	return true;
}
void ASoldierCharacter::ServerPutWeaponOnBack_Implementation()
{
	PutWeaponOnBack();
}
bool ASoldierCharacter::ServerPutWeaponOnBack_Validate()
{
	return true;
}
void ASoldierCharacter::ServerOnFoodLow_Implementation()
{
	OnFoodLow();
}
bool ASoldierCharacter::ServerOnFoodLow_Validate()
{
	return true;
}
void ASoldierCharacter::ServerOnDrinkLow_Implementation()
{
	OnDrinkLow();
}
bool ASoldierCharacter::ServerOnDrinkLow_Validate()
{
	return true;
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
		if (bIsFacing == true)
		{
			FlashAmount = 0.5f;
		}
		UKismetMaterialLibrary::SetScalarParameterValue(this, MaterialCollection, "Flash_Value", FlashAmount);
		GetWorldTimerManager().SetTimer(Timer_Flash, this, &ASoldierCharacter::PlayTimeline, 1.0f);
	}
}
void ASoldierCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//This function tells us how we want to replicate things//
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASoldierCharacter, AutomaticRifle);
	DOREPLIFETIME(ASoldierCharacter, SniperRifle);
	DOREPLIFETIME(ASoldierCharacter, CurrentWeapon);
	DOREPLIFETIME(ASoldierCharacter, Scar);

	DOREPLIFETIME(ASoldierCharacter, bReloading);
	DOREPLIFETIME(ASoldierCharacter, bZooming);
	DOREPLIFETIME(ASoldierCharacter, bDied);
	

	DOREPLIFETIME(ASoldierCharacter, bIsFiring);
	DOREPLIFETIME(ASoldierCharacter, ClimbAnim);
	DOREPLIFETIME(ASoldierCharacter, VaultAnim);
	DOREPLIFETIME(ASoldierCharacter, bIsAllowClimbing);
	DOREPLIFETIME(ASoldierCharacter, bIsWallThick);
	DOREPLIFETIME(ASoldierCharacter, bIsAbleToVault);
	DOREPLIFETIME(ASoldierCharacter, bIsObjectTooHigh);
	DOREPLIFETIME(ASoldierCharacter, bGoClimb);
	DOREPLIFETIME(ASoldierCharacter, bGoVault);
	DOREPLIFETIME(ASoldierCharacter, MaxHeightForVault);
	DOREPLIFETIME(ASoldierCharacter, TimerHandle_Vault);
	DOREPLIFETIME(ASoldierCharacter, bIsCrouching);
	DOREPLIFETIME(ASoldierCharacter, wHealthIndicator);
	DOREPLIFETIME(ASoldierCharacter, wHealthIndicatorvar);
	DOREPLIFETIME(ASoldierCharacter, wAmmoCount);
	DOREPLIFETIME(ASoldierCharacter, wAmmoCountvar);
	DOREPLIFETIME(ASoldierCharacter, PlayerName);
	DOREPLIFETIME(ASoldierCharacter, CameraComp);
	DOREPLIFETIME(ASoldierCharacter, STL);
	DOREPLIFETIME(ASoldierCharacter, STR);
	DOREPLIFETIME(ASoldierCharacter, bIsWeaponAttached);
	DOREPLIFETIME(ASoldierCharacter, bWantsToRepawn);


	
}
void ASoldierCharacter::CalculatePrimeNumbers()
{
	ThreadingTest::CalculatePrimeNumbers(MaxPrime);

	GLog->Log("--------------------------------------------------------------------");
	GLog->Log("End of prime numbers calculation on game thread");
	GLog->Log("--------------------------------------------------------------------");
}
void ASoldierCharacter::CalculatePrimeNumbersAsync()
{
	(new FAutoDeleteAsyncTask<PrimeCalculationAsyncTask>(MaxPrime))->StartBackgroundTask();
}