// Fill out your copyright notice in the Description page of Project Settings.
#include "Laser.h"

#include "Components/StaticMeshComponent.h"

#include "Weapons/AutomaticRifle.h"

#include "DrawDebugHelpers.h"

#include "Components/PointLightComponent.h"

// Sets default values
ALaser::ALaser()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MeshComp2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp2"));
	MeshComp2->SetupAttachment(MeshComp);

	PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLightComp"));
	PointLight->SetupAttachment(MeshComp);

	LaserSocket = "LaserSocket";

	LengthOfLaser = 5;
	ThickOfLaser = 0.05;

	SetReplicates(true);
	
}

void ALaser::StartLaser()
{
		AAutomaticRifle* Rifle = Cast<AAutomaticRifle>(GetOwner());
		if (Rifle)
		{
			UE_LOG(LogTemp, Warning, TEXT("LOL"));
			FName Socket = Rifle->GetMuzzleSocketName();
			FHitResult Hit;
			FVector StartSocketLocation = MeshComp->GetSocketLocation(LaserSocket);
			FRotator Rotation = Rifle->GetSkelMeshComp()->GetSocketTransform(Socket).Rotator();
			FVector ShotDirection = Rotation.Vector();
			FVector EndSocketLocation = StartSocketLocation + (ShotDirection * 10000);
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this);
			QueryParams.bTraceComplex = false;

			if (GetWorld()->LineTraceSingleByChannel(Hit, StartSocketLocation, EndSocketLocation, ECollisionChannel::ECC_Visibility, QueryParams))
			{

				DrawDebugLine(GetWorld(), StartSocketLocation, EndSocketLocation, FColor::White, false, 1.0f, 0, 10.0f);
				FVector StartLaserLocation = Hit.TraceStart;
				FVector EndLaserLocation = Hit.Location;
				FVector Laser = StartLaserLocation - EndLaserLocation;
				float LaserLentgh = Laser.Size() / LengthOfLaser;
				FVector Last = FVector(LaserLentgh, ThickOfLaser, ThickOfLaser);
				MeshComp2->SetWorldScale3D(Last);


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

