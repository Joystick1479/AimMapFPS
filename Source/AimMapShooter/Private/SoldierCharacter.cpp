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
#include "SurvivalComponent.h"

#include "AimMapShooter.h"

#include "HoloScope.h"
#include "Grip.h"
#include "Helmet.h"
#include "Headset.h"
#include "Laser.h"
#include "Magazine.h"
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
#include "PayloadCharacter.h"
#include "RedEndgame.h"
#include "BlueEndgame.h"
#include "FlashGrenade.h"
#include "Sound/SoundCue.h"
#include "Drink.h"
#include "Food.h"
#include "Survival/Water.h"
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
	LookUp = "LookUp";

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
	IsReloading = false;

	CharacterState = ECharacterState::Idle;
	HoldingWeaponState = EHoldingWeapon::None;
	//HoldingWeaponState = EHoldingWeapon::A4;
	HoldingAttachmentState = EHoldingAttachment::None;
	LaserEquipState = ELaserAttachment::None;
	MaxUseDistance = 400;

	bRiflePickUp = false;
	bWeaponOnBack = false;

	AmountGrenades = 10;

	SetReplicates(true);
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;

	///// VAULTING /////
	float MaxHeightForVault = 60;
	isAllowClimbing = false;
	isAbleToVault = false;


	/////SURVIVAL/////
	FreQOfDrainingHealthWhenLowFood = 3.0f;
	FreQOfDrainingHealthWhenLowDrink = 5.0f;
	amountOfBoostDrink = 30;
	amountOfBoostFood = 40;
	stamina = 100;


	SmoothAmount = 8.0f;
	LookAmount = 5.0f;
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
	UpdateWeaponRotation();
	UpdateRifleStatus();
	OutOfBreathSound();
	RagdollOnDeath();


	///CLAMP CAMERA
	APlayerCameraManager * CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (CameraManager)
	{
		CameraManager->ViewPitchMax = 37.0f;
		CameraManager->ViewPitchMin = -60.0f;
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

		PlayerInputComponent->BindAction("PickUp", IE_Pressed, this, &ASoldierCharacter::PickUp);

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
		if (this->AutomaticRifle)
		{
			this->AutomaticRifle->SetActorHiddenInGame(true);
		}
		if (Rifle_3rd)
		{
			Rifle_3rd->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponBackSocket);
		}

		isWeaponAttached = false;
		bWeaponOnBack = true;
	}
	else if (HoldingWeaponState == EHoldingWeapon::A4 && bWeaponOnBack == true)
	{
		///Put weapon on back//
		if (this->AutomaticRifle)
		{
			this->AutomaticRifle->SetActorHiddenInGame(false);
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

void ASoldierCharacter::IsTargetFromBack()
{
	/*FVector ActorForwardVector = this->GetActorForwardVector();

	TArray<AActor*> Target;
	UGameplayStatics::GetAllActorsOfClass(this, SoldierChar, Target);
	for (int i = 0; i < Target.Num(); i++)
	{
		ASoldierCharacter* SoldChar = Cast<ASoldierCharacter>(Target[i]);
		if (SoldChar)
		{
			FVector TargetLocation = SoldChar->GetActorLocation();
			FVector DiffVector = (TargetLocation - ActorForwardVector);
			FVector NormalizedDiffVector = UKismetMathLibrary::Normal(DiffVector);
			float DotProduct = UKismetMathLibrary::Dot_VectorVector(NormalizedDiffVector, ActorForwardVector);
			if (DotProduct > 0.55 && DotProduct < 0.999)
			{
				UE_LOG(LogTemp, Warning, TEXT("Staying behind opponent back"));
				MultipleDamage = true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("NOT staying behind opponent back"));
				MultipleDamage = false;
			}
		}
	}*/
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
		
		if (bRiflePickUp == true && HoldingWeaponState==EHoldingWeapon::None)
		{
			UE_LOG(LogTemp, Warning, TEXT("Test"));
			HoldingWeaponState = EHoldingWeapon::A4;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (Role == ROLE_Authority)
			{
				AutomaticRifle = GetWorld()->SpawnActor<AAutomaticRifle>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (AutomaticRifle)
				{
					AutomaticRifle->SetOwner(this);
					AutomaticRifle->AttachToComponent(FPPMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
					AutomaticRifle->SkelMeshComp->bOnlyOwnerSee = true;
					AutomaticRifle->SkelMeshComp->SetAnimInstanceClass(AnimBp);
					isWeaponAttached = true;
					//Weapon Sway//
					InitialWeaponRot = AutomaticRifle->SkelMeshComp->GetRelativeTransform().GetRotation().Rotator();
				}
				
			}
			if (Role == ROLE_Authority)
			{
				Rifle_3rd = GetWorld()->SpawnActor<ARifle_3rd>(ThirdWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (Rifle_3rd)
				{
					Rifle_3rd->SetOwner(this);
					Rifle_3rd->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
				}
			}
			UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());

		}

		if (bHoloPickUp == true && HoldingWeaponState == EHoldingWeapon::A4 && HoloEquipState == EHoloAttachment::None)
		{
			HoldingAttachmentState = EHoldingAttachment::Holo;
			HoloEquipState = EHoloAttachment::Equipped;

			TArray<AActor*> HoloScopes;
			UGameplayStatics::GetAllActorsOfClass(this, HoloClass, HoloScopes);
			for (int i = 0; i < HoloScopes.Num(); i++)
			{
				AHoloScope* HoloItr = Cast<AHoloScope>(HoloScopes[i]);
				if (HoloItr)
				{
					if (this->IsOverlappingActor(HoloItr))
					{
						HoloItr->IsPickedUp = true;
					}
				}
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (IsLocallyControlled())
			{
				HoloScope = GetWorld()->SpawnActor<AHoloScope>(HoloClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (HoloScope)
				{
					HoloScope->SetOwner(this);
					HoloScope->SkelMeshComp->bOnlyOwnerSee = true;
					HoloScope->SkelMeshComp->SetRenderCustomDepth(false);
					FName Socket = AutomaticRifle->ScopeSocket;
					HoloScope->AttachToComponent(AutomaticRifle->SkelMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
					isHoloAttached = true;
				}
				UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
			}
		}
		if (bGripPickUp == true && HoldingWeaponState == EHoldingWeapon::A4 && GripEquipState == EGripAttachment::None)
		{
			HoldingAttachmentState = EHoldingAttachment::Grip;
			GripEquipState = EGripAttachment::Equipped;

			TArray<AActor*> Grips;
			UGameplayStatics::GetAllActorsOfClass(this, GripClass, Grips);
			for (int i = 0; i < Grips.Num(); i++)
			{
				AGrip* GripItr = Cast<AGrip>(Grips[i]);
				if (GripItr)
				{
					if (this->IsOverlappingActor(GripItr))
					{
						GripItr->IsPickedUp = true;
					}
				}
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (IsLocallyControlled())
			{
				Grip = GetWorld()->SpawnActor<AGrip>(GripClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (Grip)
				{
					Grip->SetOwner(this);
					Grip->MeshComp->bOnlyOwnerSee = true;
					Grip->MeshComp->SetRenderCustomDepth(false);
					FName GSocket = AutomaticRifle->GripSocket;
					Grip->AttachToComponent(AutomaticRifle->SkelMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, GSocket);
					isGripAttached = true;
				}
				UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
			}
		}
		if (bHelmetPickUp == true && HelmetEquipState==EHelmetAttachment::None)
		{
			HelmetEquipState = EHelmetAttachment::Equipped;

			TArray<AActor*> Helemts;
			UGameplayStatics::GetAllActorsOfClass(this, HelmetClass, Helemts);
			for (int i = 0; i < Helemts.Num(); i++)
			{
				AHelmet* HelmetItr = Cast<AHelmet>(Helemts[i]);
				if (HelmetItr)
				{
					if (this->IsOverlappingActor(HelmetItr))
					{
						HelmetItr->IsPickedUp = true;
					}
				}
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			Helmet = GetWorld()->SpawnActor<AHelmet>(HelmetClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (Helmet)
			{
				Helmet->SetOwner(this);
				Helmet->MeshComp->SetRenderCustomDepth(false);
				Helmet->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HelmetSocket);
				isHelmetAttached = true;
			}
			UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
		}
		if (bHeadsetPickUp == true && HeadsetEquipState == EHeadsetAttachment::None)
		{
			HeadsetEquipState = EHeadsetAttachment::Equipped;

			TArray<AActor*> Headsets;
			UGameplayStatics::GetAllActorsOfClass(this, HeadsetClass, Headsets);
			for (int i = 0; i < Headsets.Num(); i++)
			{
				AHeadset* HeadsetItr = Cast<AHeadset>(Headsets[i]);
				if (HeadsetItr)
				{
					if (this->IsOverlappingActor(HeadsetItr))
					{
						HeadsetItr->IsPickedUp = true;
					}
				}
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			Headset = GetWorld()->SpawnActor<AHeadset>(HeadsetClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (Headset)
			{
				Headset->SetOwner(this);
				Headset->MeshComp->SetRenderCustomDepth(false);
				Headset->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeadsetSocket);
				Headset->MeshComp->ToggleActive();
				isHeadsetAttached = true;
			}
			///Destroy after picking up

			UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
		}
		if (bLaserPickUp == true && LaserEquipState == ELaserAttachment::None)
		{
			LaserEquipState = ELaserAttachment::Equipped;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			TArray<AActor*> Lasers;
			UGameplayStatics::GetAllActorsOfClass(this, LaserClass, Lasers);
			for (int i = 0; i < Lasers.Num(); i++)
			{
				ALaser* LaserItr = Cast<ALaser>(Lasers[i]);
				if (LaserItr)
				{
					if (this->IsOverlappingActor(LaserItr))
					{
						LaserItr->IsPickedUp = true;
					}
				}
			}

			if (Role<ROLE_Authority)
			{
				Laser = GetWorld()->SpawnActor<ALaser>(LaserClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (Laser)
				{
					Laser->SetOwner(this);
					Laser->MeshComp->bOnlyOwnerSee = true;
					Laser->MeshComp->SetRenderCustomDepth(false);
					FName LSocket = AutomaticRifle->LaserSocket;
					Laser->AttachToComponent(AutomaticRifle->SkelMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, LSocket);
					isLaserAttached = true;
				}
				UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
			}
		}
		if (bMagazinePickUp == true)
		{

			TArray<AActor*> Magazines;
			UGameplayStatics::GetAllActorsOfClass(this, MagazineClass, Magazines);
			for (int i = 0; i < Magazines.Num(); i++)
			{
				AMagazine* MagazineItr = Cast<AMagazine>(Magazines[i]);
				if (MagazineItr)
				{
					if (this->IsOverlappingActor(MagazineItr))
					{
						MagazineItr->IsPickedUp = true;
					}
				}
			}

			//SoldierCurrentClips++;
			TArray<AActor*> Rifles;
			UGameplayStatics::GetAllActorsOfClass(this, StarterWeaponClass, Rifles);
			for (int i = 0; i < Rifles.Num(); i++)
			{
				AAutomaticRifle* RiflesItr = Cast<AAutomaticRifle>(Rifles[i]);
				if (RiflesItr)
				{
					RiflesItr->CurrentAmountOfClips++;
				}
			}


			UGameplayStatics::PlaySoundAtLocation(this, ItemsPickUp, GetActorLocation());
		}

		///SURVIVAL STUFF///
		if (bDrinkPickup == true)
		{
			amountOfDrinks++;

			TArray<AActor*> Drinks;
			UGameplayStatics::GetAllActorsOfClass(this, DrinkClass, Drinks);
			for (int i = 0; i < Drinks.Num(); i++)
			{
				ADrink* Drink = Cast<ADrink>(Drinks[i]);
				if (Drink)
				{
					if (this->IsOverlappingActor(Drink))
					{
						Drink->IsPickedUp = true;
					}
				}
			}

		}
		if (bFoodPickup == true)
		{
			amountOfFood++;

			TArray<AActor*> Foods;
			UGameplayStatics::GetAllActorsOfClass(this, FoodClass, Foods);
			for (int i = 0; i < Foods.Num(); i++)
			{
				AFood* Food = Cast<AFood>(Foods[i]);
				if (Food)
				{
					if (this->IsOverlappingActor(Food))
					{
						Food->IsPickedUp = true;
					}
				}
			}
		}
		if (bDrinkFromPond == true)
		{
			APlayerController* PC = Cast<APlayerController>(GetController());
			DisableInput(PC);
			FTimerDelegate DelegateFunc = FTimerDelegate::CreateUObject(this, &ASoldierCharacter::EndDrinkFromPond, PC);
			GetWorldTimerManager().SetTimer(DrinkFromPondTimer, DelegateFunc, 2.5f, false);

			if (IsLocallyControlled())
			{
				UGameplayStatics::PlaySound2D(this, DrinkFromPondSound);
			}
		}
}
void ASoldierCharacter::EndDrinkFromPond(APlayerController* PC)
{
	EnableInput(PC);
	bDrinkFromPond = false;
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
					if ((bRiflePickUp || bHeadsetPickUp || bLaserPickUp || bHelmetPickUp || bGripPickUp || bHoloPickUp || bFoodPickup || bDrinkPickup || bDrinkFromPond || bMagazinePickUp) == true)
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
	if (MoveComp && !IsSprinting && !IsInspecting && !IsReloading &&bWeaponOnBack != true)
	{
		MoveComp->MaxWalkSpeed = 250.0f;
		if (!IsSprinting)
		{
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
	if (!IsSprinting && bWeaponOnBack == false && bDied != true)
	{
		CharacterState = ECharacterState::Firing;

		IsFiring = true;

		if (IsSingleFire == false)
		{
			if (AutomaticRifle)
			{
				if (AutomaticRifle->CurrentAmmoInClip > 0 && AutomaticRifle->CurrentState != EWeaponState::Reloading)
				{
					bFireAnimation = true;
				}
				AutomaticRifle->StartFire();
			}
		}
		else
		{
			if (AutomaticRifle)
			{
				if (AutomaticRifle->CurrentAmmoInClip > 0 && AutomaticRifle->CurrentState != EWeaponState::Reloading)
				{
					bFireAnimation = true;
				}
				AutomaticRifle->Fire();
			}
		}
	}
}

void ASoldierCharacter::StopFire()
{
	CharacterState = ECharacterState::Idle;
	IsFiring = false;

	bFireAnimation = false;

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
		if (IsZooming != true && GetVelocity().Size()>0)
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
	if (CharacterState == ECharacterState::Idle && (SoldierCurrentClips > 0) && !IsZooming && bDied != true)
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

void ASoldierCharacter::UpdateRifleStatus()
{
	if (AutomaticRifle)
	{
		SoldierCurrentAmmoInClip = AutomaticRifle->CurrentAmmoInClip;
		SoldierCurrentAmmo = AutomaticRifle->CurrentAmmo;
		SoldierCurrentClips = AutomaticRifle->CurrentAmountOfClips;
	}

	GetWorldTimerManager().SetTimer(UpdateRifleTimer, this, &ASoldierCharacter::UpdateRifleStatus, 0.05f, false);
}
void ASoldierCharacter::UpdateWeaponRotation()
{
	//Weapon Sway
	FRotator AlmostFinal = FRotator(temp2*LookAmount, temp1*LookAmount, temp1*LookAmount);
	FRotator TempRotator = FRotator(InitialWeaponRot.Pitch - AlmostFinal.Pitch, AlmostFinal.Yaw + InitialWeaponRot.Yaw, InitialWeaponRot.Roll + AlmostFinal.Roll);
	if (AutomaticRifle)
	{
		float timeWorld = UGameplayStatics::GetWorldDeltaSeconds(GetWorld());
		FinalWeaponRot = UKismetMathLibrary::RInterpTo(AutomaticRifle->SkelMeshComp->GetRelativeTransform().GetRotation().Rotator(), TempRotator, timeWorld, SmoothAmount);
	}

	GetWorldTimerManager().SetTimer(UpdateWRotTimer, this, &ASoldierCharacter::UpdateWeaponRotation, 0.05f, false);
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
		//return;
	}
	if (SurvivalComp)
	{
		if (SurvivalComp->Food == 0)
		{
			UHealthComponent* HealthComp = this->FindComponentByClass<UHealthComponent>();
			if (HealthComp)
			{
				if (HealthComp->Health > 0)
				{
					//UE_LOG(LogTemp, Warning, TEXT("Znalazlem healthcomp"));
					HealthComp->Health = HealthComp->Health - 1;
				}

				if (HealthComp->Health == 0) bDied = true;
			}
			//UE_LOG(LogTemp, Warning, TEXT("Food low!!!"));
			GetWorldTimerManager().SetTimer(FoodLowTimer, this, &ASoldierCharacter::OnFoodLow, FreQOfDrainingHealthWhenLowFood, false);
		}
		if (SurvivalComp->Food > 0)
		{
			SurvivalComp->OnRep_Food();
		}
	}
	
}
void ASoldierCharacter::OnDrinkLow()
{
	if (Role < ROLE_Authority)
	{
		ServerOnDrinkLow();
		return;
	}

	//USurvivalComponent* SurvComp = this->FindComponentByClass<USurvivalComponent>();
	if (SurvivalComp)
	{
		if (SurvivalComp->Drink == 0)
		{
			UHealthComponent* HealthComp = this->FindComponentByClass<UHealthComponent>();
			if (HealthComp)
			{
				if (HealthComp->Health > 0)
				{
					//UE_LOG(LogTemp, Warning, TEXT("Znalazlem healthcomp"));
					HealthComp->Health = HealthComp->Health - 1;
				}

				if (HealthComp->Health == 0) bDied = true;

			}
			//UE_LOG(LogTemp, Warning, TEXT("Drink low!!!"));
			GetWorldTimerManager().SetTimer(DrinkLowTimer, this, &ASoldierCharacter::OnDrinkLow, FreQOfDrainingHealthWhenLowDrink, false);
		}
		if (SurvivalComp->Drink > 0)
		{
			SurvivalComp->OnRep_Drink();
		}
	}

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