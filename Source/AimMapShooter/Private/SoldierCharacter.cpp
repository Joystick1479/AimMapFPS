// Fill out your copyright notice in the Description page of Project Settings.

#include "Camera/CameraComponent.h"
#include "AutomaticRifle.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HealthComponent.h"
#include "AimMapShooter.h"
#include "SoldierCharacter.h"


// Sets default values
ASoldierCharacter::ASoldierCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HeadSocket = "HeadSocket";
	WeaponSocket = "WeaponSocket";


	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeadSocket);

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

	ZoomingTime = 0.2f;

	IsSingleFire = false;

	CharacterState = ECharacterState::Idle;
	MaxUseDistance = 400;

	bFocusItem = false;

}

// Called when the game starts or when spawned
void ASoldierCharacter::BeginPlay()
{
	Super::BeginPlay();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AutomaticRifle = GetWorld()->SpawnActor<AAutomaticRifle>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (AutomaticRifle)
	{
		AutomaticRifle->SetOwner(this);
		AutomaticRifle->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
	}

	HealthComp->OnHealthChanged.AddDynamic(this, &ASoldierCharacter::OnHealthChanged);
	
}

void ASoldierCharacter::LineTraceItem()
{
	//Line trace to pickup object//
	//FVector camLoc;
	//FRotator camRot;

	//if (Controller == NULL)  return;

	//Controller->GetPlayerViewPoint(camLoc, camRot);
	const FVector start_trace = CameraComp->GetComponentLocation();
	const FVector direction = CameraComp->GetComponentRotation().Vector();
	const FVector end_trace = start_trace + (direction* MaxUseDistance);

	FCollisionQueryParams TraceParams(FName(TEXT("")), true, this);
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;
	TraceParams.AddIgnoredActor(this);

		FHitResult Hit;

		if (GetWorld()->LineTraceSingleByChannel(Hit, start_trace, end_trace, COLLISION_ITEMS , TraceParams))
		{
			DrawDebugLine(GetWorld(), start_trace, end_trace, FColor::Red, false, 1.0f, 0, 1.0f);
			bFocusItem = true;
		}
		else
		{
			bFocusItem = false;
		}
	
}

// Called every frame
void ASoldierCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LineTraceItem();

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
	Crouch();
}
void ASoldierCharacter::EndCrouch()
{
	UnCrouch();
}
void ASoldierCharacter::ZoomIn()
{
	IsZooming = true;

	APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		if (AutomaticRifle)
		{
			PC->SetViewTargetWithBlend(AutomaticRifle, ZoomingTime, EViewTargetBlendFunction::VTBlend_Linear);
		}
	}
}

void ASoldierCharacter::ZoomOut()
{
	IsZooming = false;

	APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
			PC->SetViewTargetWithBlend(this, ZoomingTime, EViewTargetBlendFunction::VTBlend_Linear);
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

void ASoldierCharacter::Reload()
{
	if (AutomaticRifle)
	{
		AutomaticRifle->StartReload();
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
}