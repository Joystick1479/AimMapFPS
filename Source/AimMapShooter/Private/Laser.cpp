// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "SoldierCharacter.h"
#include "AutomaticRifle.h"
#include "DrawDebugHelpers.h"
#include "Laser.h"

// Sets default values
ALaser::ALaser()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(MeshComp);

	LaserSocket = "LaserSocket";
	
}

// Called when the game starts or when spawned
void ALaser::BeginPlay()
{
	Super::BeginPlay();


	
}

// Called every frame
void ALaser::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	


	AAutomaticRifle* Rifle = Cast<AAutomaticRifle>(GetOwner());
	if (Rifle)
	{
		FName Socket = Rifle->MuzzleSocket;

		FHitResult Hit;
		FVector StartLocation = MeshComp->GetSocketLocation(LaserSocket);
		FRotator Rotation = Rifle->GetRootComponent()->GetSocketRotation(Socket);
		FVector ShotDirection = Rotation.Vector();
		FVector EndLocation = StartLocation + (ShotDirection * 10000);
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.bReturnPhysicalMaterial = true;
		QueryParams.bTraceComplex = true;


		if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECollisionChannel::ECC_Pawn, QueryParams))
		{
			DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::White, false, 1.0f, 0, 1.0f);
		}
	}
		

}

