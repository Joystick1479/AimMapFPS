// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseAttachmentClass.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

#include "TimerManager.h"

#include "Character/SoldierCharacter.h"
#include "Weapons/BaseWeaponClass.h"

// Sets default values
ABaseAttachmentClass::ABaseAttachmentClass()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh1P = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh1P"));
	Mesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = Mesh1P;
	

	Mesh3P = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh3P"));
	Mesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh3P->SetupAttachment(Mesh1P);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(Mesh1P);

	SetReplicates(true);

}

ABaseAttachmentClass::~ABaseAttachmentClass()
{
	bAttachmentAttached = false;
}

// Called when the game starts or when spawned
void ABaseAttachmentClass::BeginPlay()
{
	Super::BeginPlay();
	
}
void ABaseAttachmentClass::PickUpAttachment(ABaseWeaponClass* Weapon)
{
	if (PawnOwner)
	{
		if (PawnOwner->IsLocallyControlled() == true)
		{
			USkeletalMeshComponent* PawnMesh1P = PawnOwner->GetFPPMesh();
			USkeletalMeshComponent* PawnMesh3P = PawnOwner->GetMesh();
			FName AttachPoint = PawnOwner->GetWeaponAttachPoint();
			Mesh1P->AttachToComponent(PawnOwner->GetCurrentWeapon()->GetMesh1P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, GetAttachPoint());
			Mesh3P->AttachToComponent(PawnOwner->GetCurrentWeapon()->GetMesh3P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, GetAttachPoint());

			//Turn off collision after picking up object to stop line trace to pick it up
			if (SphereComp)
			{
				SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}

			bAttachmentAttached = true;
		}
	}
	
	
}
void ABaseAttachmentClass::SetOwningPawn(ASoldierCharacter* SoldierCharacter)
{
	if (PawnOwner != SoldierCharacter)
	{
		PawnOwner = SoldierCharacter;
		SetOwner(SoldierCharacter);
	}
}
bool ABaseAttachmentClass::GetIfPickeditem()
{
	return bPickedUp;
}
void ABaseAttachmentClass::SetIfPickedItem(bool PickedUp)
{
	bPickedUp = PickedUp;
}

UStaticMeshComponent* ABaseAttachmentClass::GetMesh1P()
{
	return Mesh1P;
}
UStaticMeshComponent* ABaseAttachmentClass::GetMesh3P()
{
	return Mesh3P;
}
USphereComponent* ABaseAttachmentClass::GetSphereComponent()
{
	return SphereComp;
}
FName ABaseAttachmentClass::GetAttachPoint() const
{
	return "";
}
bool ABaseAttachmentClass::bIsAttachmentAttached() const
{
	return bAttachmentAttached;
}