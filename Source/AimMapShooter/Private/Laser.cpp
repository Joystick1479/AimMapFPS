// Fill out your copyright notice in the Description page of Project Settings.
#include "Laser.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

#include "SoldierCharacter.h"
#include "AutomaticRifle.h"

#include "TimerManager.h"

#include "DrawDebugHelpers.h"

#include "Components/PointLightComponent.h"

// Sets default values
ALaser::ALaser()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MeshComp2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp2"));
	MeshComp2->SetupAttachment(MeshComp);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(MeshComp);

	PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLightComp"));
	PointLight->SetupAttachment(MeshComp);

	LaserSocket = "LaserSocket";

	LengthOfLaser = 5;
	ThickOfLaser = 0.05;

	SetReplicates(true);
	
}

// Called when the game starts or when spawned
void ALaser::BeginPlay()
{
	Super::BeginPlay();

	DestroyOnUse();

	
}

void ALaser::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);



	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{
		SoldierCharacter->bLaserPickUp = true;
		SphereComp->ToggleActive();
	}
}

void ALaser::NotifyActorEndOverlap(AActor * OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{
		SoldierCharacter->bLaserPickUp = false;
	}
}

// Called every frame
void ALaser::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}

void ALaser::DestroyOnUse()
{
	StartLaser();

	if (IsPickedUp == true) this->Destroy();

	GetWorldTimerManager().SetTimer(DestroyTimer, this, &ALaser::DestroyOnUse, 0.1f, false);

}


void ALaser::StartLaser()
{
		AAutomaticRifle* Rifle = Cast<AAutomaticRifle>(GetOwner());
		if (Rifle)
		{
			UE_LOG(LogTemp, Warning, TEXT("LOL"));
			FName Socket = Rifle->MuzzleSocket;
			FHitResult Hit;
			FVector StartLocation = MeshComp->GetSocketLocation(LaserSocket);
		//	FRotator Rotation = Rifle->GetRootComponent()->GetSocketRotation(Socket);
			FRotator Rotation = Rifle->SkelMeshComp->GetSocketTransform(Socket).Rotator();
			FVector ShotDirection = Rotation.Vector();
			FVector EndLocation = StartLocation + (ShotDirection * 10000);
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this);
			QueryParams.bTraceComplex = false;

			if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility, QueryParams))
			{

				DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::White, false, 1.0f, 0, 10.0f);
				FVector StartLocation = Hit.TraceStart;
				FVector EndLocation = Hit.Location;
				FVector Laser = StartLocation - EndLocation;
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



