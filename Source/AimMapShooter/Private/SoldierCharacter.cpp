// Fill out your copyright notice in the Description page of Project Settings.

#include "Camera/CameraComponent.h"
#include "AutomaticRifle.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
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
			PC->SetViewTargetWithBlend(AutomaticRifle, 2.0f, EViewTargetBlendFunction::VTBlend_Cubic);
		}
	}
}

void ASoldierCharacter::ZoomOut()
{
	IsZooming = false;

	APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
			PC->SetViewTargetWithBlend(this, 2.0f, EViewTargetBlendFunction::VTBlend_Cubic);
	}
}

void ASoldierCharacter::StartFire()
{
	if (AutomaticRifle)
	{
		AutomaticRifle->StartFire();
	}
}

void ASoldierCharacter::StopFire()
{
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
