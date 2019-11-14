// Fill out your copyright notice in the Description page of Project Settings.

#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AutomaticRifle.h"
#include "AMainCharacter.h"

// Sets default values
AAMainCharacter::AAMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SkelMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));

	HeadSocket = "HeadSocket";

	//Socket on Gun//
	CameraSocket = "CameraScope";

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeadSocket);

	WeaponSocket = "WeaponSocket";
}

// Called when the game starts or when spawned
void AAMainCharacter::BeginPlay()
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
void AAMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Keyboard movement
	PlayerInputComponent->BindAxis("MoveForward", this, &AAMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AAMainCharacter::MoveRight);

	//Mouse look
	PlayerInputComponent->BindAxis("LookUp", this, &AAMainCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &AAMainCharacter::AddControllerYawInput);

	//Crouching
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AAMainCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AAMainCharacter::EndCrouch);

	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &AAMainCharacter::ZoomIn);
	PlayerInputComponent->BindAction("ZoomIn", IE_Released, this, &AAMainCharacter::ZoomOut);


}

void AAMainCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector()*Value);
}
void AAMainCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}
void AAMainCharacter::BeginCrouch()
{
	Crouch();
}
void AAMainCharacter::EndCrouch()
{
	UnCrouch();
}
void AAMainCharacter::ZoomIn()
{
	//Attaching camera to ADS//
	//CameraComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	//CameraComp->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, CameraSocket);
}

void AAMainCharacter::ZoomOut()
{
	//CameraComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	//CameraComp->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeadSocket);
}
