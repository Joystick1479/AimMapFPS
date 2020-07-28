// Fill out your copyright notice in the Description page of Project Settings.
#include "Laser.h"

#include "Components/StaticMeshComponent.h"

#include "Weapons/BaseWeaponClass.h"
#include "Character/SoldierCharacter.h"

#include "DrawDebugHelpers.h"

#include "Components/PointLightComponent.h"

// Sets default values
ALaser::ALaser()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp2"));
	MeshComp2->SetupAttachment(Mesh1P);

	PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLightComp"));
	PointLight->SetupAttachment(Mesh1P);

	LaserSocket = "LaserSocket";

	LengthOfLaser = 5;
	ThickOfLaser = 0.05;

	SetReplicates(true);
	
}

void ALaser::StartLaser(ABaseWeaponClass* WeaponClass)
{
	if (WeaponClass)
	{
		
		FName Socket = WeaponClass->GetMuzzleSocketName();
		FHitResult Hit;
		FVector StartSocketLocation = Mesh1P->GetSocketLocation(LaserSocket);
		FRotator ActorEyes;
		ASoldierCharacter* SoldierChar = Cast<ASoldierCharacter>(GetOwner());
		if (SoldierChar)
		{
			SoldierChar->GetActorEyesViewPoint(StartSocketLocation, ActorEyes);
		}

		FVector EndLocation = StartSocketLocation + ActorEyes.Vector() * 10000;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = false;


		if (GetWorld()->LineTraceSingleByChannel(Hit, StartSocketLocation, EndLocation, ECollisionChannel::ECC_Visibility, QueryParams))
		{
			DrawDebugLine(GetWorld(), StartSocketLocation, EndLocation, FColor::White, false, 1.0f, 0, 1.0f);
			FVector StartLaserLocation = Hit.TraceStart;
			FVector EndLaserLocation = Hit.Location;
			FVector Laser = StartLaserLocation - EndLaserLocation;
			float LaserLentgh = Laser.Size() / LengthOfLaser;
			FVector Last = FVector(LaserLentgh, ThickOfLaser, ThickOfLaser);
			//MeshComp2->SetWorldScale3D(Last);


			FVector LaserImpact = Hit.ImpactPoint;
			FVector LaserForwardVector = PointLight->GetForwardVector();
			FVector LastLaserImpact = LaserImpact - LaserForwardVector;
			PointLight->SetWorldLocation(LastLaserImpact);

		}
	}
	
}

UPointLightComponent* ALaser::GetPointLightComponent()
{
	return this->PointLight;
}
UStaticMeshComponent* ALaser::GetScalableMeshComponent()
{
	return this->MeshComp2;
}

