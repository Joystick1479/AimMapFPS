// Fill out your copyright notice in the Description page of Project Settings.
#include "HoloScope.h"

#include "Weapons/BaseWeaponClass.h"
#include "Character/SoldierCharacter.h"

#include "Components/SphereComponent.h"

#define PrintLog(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Green,text)

// Sets default values
AHoloScope::AHoloScope()
{

	SetReplicates(true);
}

FName AHoloScope::GetAttachPoint() const
{
	return "ScopeSocket";
}

void AHoloScope::PickUpAttachment(ABaseWeaponClass* Weapon)
{
	if (PawnOwner && PawnOwner->GetHoloScope()==nullptr)
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
	/*if (Weapon)
	{
		Weapon->SetupHoloScope(this);
		PrintLog("Find owner");
	}
	else
	{
		PrintLog("Not found owner");
	}*/

}


