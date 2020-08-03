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



