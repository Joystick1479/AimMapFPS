// Fill out your copyright notice in the Description page of Project Settings.
#include "SoldierCharacter.h"
#include "Camera/CameraComponent.h"

#include "Weapons/AutomaticRifle.h"
#include "Weapons/SniperRifle.h"

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

#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


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

	FPPMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPPMesh"));
	FPPMesh->SetupAttachment(RootComponent);

	SpringArm = CreateDefaultSubobject <USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(FPPMesh);
	

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArm);

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


	ZoomingTime = 0.2f;

	IsSingleFire = false;
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
	isAllowClimbing = false;
	isAbleToVault = false;


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

	if (SpringArm)
	{
		SpringArm->AttachToComponent(FPPMesh, FAttachmentTransformRules::SnapToTargetIncludingScale);
	
	}
	if (CameraComp)
	{
		CameraComp->AttachToComponent(SpringArm, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}

	///Calling these function on begin play with timers to avoid ticking
	ShowingPickUpHud();
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
		CameraManager->ViewPitchMin = -60.0f;
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
		DrawDebugLine(GetWorld(), start_trace, end_trace, FColor::Red, false, 1.0f, 0, 1.0f);

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

	GetWorldTimerManager().SetTimer(RagdollTimer, this, &ASoldierCharacter::RagdollOnDeath, 1.25f, false);
}
void ASoldierCharacter::PutWeaponOnBack()
{
	if (Role < ROLE_Authority)
	{
		ServerPutWeaponOnBack();
	}
	if (HoldingWeaponState == EHoldingWeapon::A4 && bWeaponOnBack == false)
	{
		/// TO DO NIE WOLNO STRZELAC JAK NA PLECACH BRON

		///Put weapon on back for 3rd person and hide weapon for 1st person///
		if (this->CurrentWeapon)
		{
			this->CurrentWeapon->SetActorHiddenInGame(true);
		}
		if (Rifle_3rd)
		{
			Rifle_3rd->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponBackSocket);
		}

		isWeaponAttached = false;
		bWeaponOnBack = true;
	}
	else if (HoldingWeaponState == EHoldingWeapon::A4 || HoldingWeaponState == EHoldingWeapon::Sniper && bWeaponOnBack == true)
	{
		///Put weapon on back//
		if (this->CurrentWeapon)
		{
			this->CurrentWeapon->SetActorHiddenInGame(false);
		}
		if (Rifle_3rd)
		{
			Rifle_3rd->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
		}

		isWeaponAttached = true;
		bWeaponOnBack = false;
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
		Laser->StartLaser();
	}
}
void ASoldierCharacter::PickUp(ABaseWeaponClass* Weapons, ABaseAttachmentClass* Attachments, ABaseSurvivalItemClass* SurvivalItem)
{

		if (Role < ROLE_Authority)
		{
			ServerPickUpItem(Weapons,Attachments,SurvivalItem);
		}

		AAutomaticRifle* AutoRifle = Cast<AAutomaticRifle>(Weapons);
		if (AutoRifle)
		{
			UE_LOG(LogTemp, Warning, TEXT("Tutaj3"));

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
					isWeaponAttached = true;
					AutoRifle->Destroy();

				}

				Rifle_3rd = GetWorld()->SpawnActor<ARifle_3rd>(ThirdWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (Rifle_3rd)
				{
					Rifle_3rd->SetOwner(this);
					Rifle_3rd->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
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
						isWeaponAttached = true;
						SniperWeapon->Destroy();

					}
				}
				UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());

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
						isHoloAttached = true;
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

					isLaserAttached = true;
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
					isHelmetAttached = true;
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
					isHeadsetAttached = true;
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
void ASoldierCharacter::ShowingPickUpHud()
{
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
					if (bWantToPickUp && bDoOnce == true)
					{
						wPickUpvar->AddToViewport();
						bDoOnce = false;
					}
					else
					{
						bRemoveHud = true;
						bDoOnce = true;
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
	
	GetWorldTimerManager().SetTimer(HudTimer, this, &ASoldierCharacter::ShowingPickUpHud, 0.5f, false);

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
		if (!IsSprinting)
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
	///Hide 3rd person mesh
	UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
	if (MoveComp && !IsSprinting && !IsInspecting && !bReloading &&bWeaponOnBack != true)
	{
		MoveComp->MaxWalkSpeed = 250.0f;
		if (!IsSprinting)
		{
			bZooming = true;

			if (IsLocallyControlled())
			{

				APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
				if (PC)
				{
					if (CurrentWeapon)
					{
						PC->SetViewTargetWithBlend(CurrentWeapon, ZoomingTime, EViewTargetBlendFunction::VTBlend_Linear);
					}
				}
			}
		}
	}

	if (IsLocallyControlled())
	{
		TArray<AActor*> ThirdWeapon;
		UGameplayStatics::GetAllActorsOfClass(this, ThirdWeaponClass, ThirdWeapon);
		for (int i = 0; i < ThirdWeapon.Num(); i++)
		{
			ARifle_3rd* HideIt = Cast<ARifle_3rd>(ThirdWeapon[i]);
			if (HideIt)
			{
				if (this == HideIt->GetOwner())
				{
					HideIt->SkelMeshComp->SetHiddenInGame(true, false);
				}
			}
		}
	}
}
void ASoldierCharacter::ZoomOut()
{
	///Make 3rd rifle visible
	if (IsLocallyControlled())
	{
		TArray<AActor*> ThirdWeapon;
		UGameplayStatics::GetAllActorsOfClass(this, ThirdWeaponClass, ThirdWeapon);
		for (int i = 0; i < ThirdWeapon.Num(); i++)
		{
			ARifle_3rd* HideIt = Cast<ARifle_3rd>(ThirdWeapon[i]);
			if (HideIt)
			{
				HideIt->SkelMeshComp->SetHiddenInGame(false, false);
			}
		}
	}

	UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
	if (MoveComp)
	{
		MoveComp->MaxWalkSpeed = 300.0f;
	}

	bZooming = false;

	if (IsLocallyControlled())
	{

		APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
		if (PC)
		{
			if (CurrentWeapon)
			{
				PC->SetViewTargetWithBlend(this, ZoomingTime, EViewTargetBlendFunction::VTBlend_Linear);
			}
		}
	}
}
void ASoldierCharacter::StartFire()
{
	if (!IsSprinting && bWeaponOnBack == false && bDied != true)
	{
		CharacterState = ECharacterState::Firing;

		IsFiring = true;

		if (IsSingleFire == false)
		{
			if (CurrentWeapon)
			{
				if (CurrentWeapon->GetCurrentAmmoInClip() > 0 && CurrentWeapon->CurrentState != EWeaponState::Reloading )
				{
					bFireAnimation = true;
					CurrentWeapon->StartFire();
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
				}
				CurrentWeapon->Fire();
			}
		}
	}
}
void ASoldierCharacter::StopFire()
{
	CharacterState = ECharacterState::Idle;
	IsFiring = false;

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
	else if (stamina > 10 && IsSprinting == false)
	{
		if (bZooming != true && GetVelocity().Size()>0)
		{
			SprintProgressBar();
			IsSprinting = true;
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

	IsSprinting = false;
	UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
	if (MoveComp)
	{
		MoveComp->MaxWalkSpeed = 300.0f;
	}
	
}
void ASoldierCharacter::SprintProgressBar()
{
	if (IsSprinting == true)
	{
		GetWorldTimerManager().SetTimer(SprintTimerHandle, this, &ASoldierCharacter::SprintProgressBar, 0.2f, false);
		stamina--;
		if (stamina == 0) IsSprinting = false;
		//UE_LOG(LogTemp, Warning, TEXT("Sprinting now, bar is: %f"), stamina);
	}
	if (IsSprinting == false)
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
		IsSprinting = false;
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
		if (stamina < 10 && ResetBreath == false)
		{
			UGameplayStatics::PlaySound2D(this, OutOfBreath);
			GetWorldTimerManager().SetTimer(OutOfBreathTimer, this, &ASoldierCharacter::OutOfBreathReset, 9.0f, false);
			ResetBreath = true;
		}
	}

	GetWorldTimerManager().SetTimer(UpdateBreath, this, &ASoldierCharacter::OutOfBreathSound, 1.0f, false);
}
void ASoldierCharacter::OutOfBreathReset()
{
	ResetBreath = false;
}
void ASoldierCharacter::Headbobbing()
{
	if (IsSprinting == true)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(CameraSprintShake, 0.5f);
		}
	}
	if (GetVelocity().Size() > 0)
	{
		if (IsSprinting == false)
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
	if (CharacterState == ECharacterState::Idle && (SoldierCurrentClips > 0) && !bZooming && bDied != true)
	{
		if (CurrentWeapon)
		{
			CharacterState = ECharacterState::Reloading;
			bReloading = true;
			CurrentWeapon->StartReload();
			ServerReloadingSound();
			GetWorldTimerManager().SetTimer(ReloadTimer, this, &ASoldierCharacter::StopReload, 2.167f, false);
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
void ASoldierCharacter::UpdateRifleStatus()
{
	if (CurrentWeapon)
	{
		SoldierCurrentAmmoInClip = CurrentWeapon->GetCurrentAmmoInClip();
		SoldierCurrentAmmo = CurrentWeapon->GetAllAmmo();
		SoldierCurrentClips = CurrentWeapon->GetCurrentAmountOfClips();
	}
	GetWorldTimerManager().SetTimer(UpdateRifleTimer, this, &ASoldierCharacter::UpdateRifleStatus, 0.05f, false);
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
			if (IsFacing == true)
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
	
			IsFacing = UKismetMathLibrary::BooleanOR(DiffRotation >= 90.0f, DiffRotation <= -90.0f);
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
	PickUp(Weapons,Attachments,SurvivalItem);
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
void ASoldierCharacter::ServerShowingPickUpHud_Implementation()
{
	ShowingPickUpHud();
}
bool ASoldierCharacter::ServerShowingPickUpHud_Validate()
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

	DOREPLIFETIME(ASoldierCharacter, bReloading);
	DOREPLIFETIME(ASoldierCharacter, bZooming);
	DOREPLIFETIME(ASoldierCharacter, bDied);

	DOREPLIFETIME(ASoldierCharacter, IsFiring);
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
	DOREPLIFETIME(ASoldierCharacter, IsCrouching);
	DOREPLIFETIME(ASoldierCharacter, wHealthIndicator);
	DOREPLIFETIME(ASoldierCharacter, wHealthIndicatorvar);
	DOREPLIFETIME(ASoldierCharacter, wAmmoCount);
	DOREPLIFETIME(ASoldierCharacter, wAmmoCountvar);
	DOREPLIFETIME(ASoldierCharacter, PlayerName);
	DOREPLIFETIME(ASoldierCharacter, CameraComp);
	DOREPLIFETIME(ASoldierCharacter, STL);
	DOREPLIFETIME(ASoldierCharacter, STR);
	DOREPLIFETIME(ASoldierCharacter, isWeaponAttached);
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