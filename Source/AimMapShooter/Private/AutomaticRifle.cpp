// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "SoldierCharacter.h"
#include "AutomaticRifle.h"

// Sets default values
AAutomaticRifle::AAutomaticRifle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraSocket = "CameraSocket";

	SkelMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkelMesh"));
	RootComponent = SkelMeshComp;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SkelMeshComp, CameraSocket);

	MuzzleSocket = "MuzzleSocket";
	LineSocket = "LineSocket";

}

enum class EAmmoType
{
	EStandardAmmo,
	EBpAmmo,
	EBtAmmo,
};

// Called when the game starts or when spawned
void AAutomaticRifle::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAutomaticRifle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAutomaticRifle::Fire()
{
	ASoldierCharacter* SoldierChar = Cast<ASoldierCharacter>(GetOwner());
	if (SoldierChar->IsZooming == true)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			FHitResult Hit;
			FVector StartLocation = SkelMeshComp->GetSocketLocation(LineSocket);
			FRotator Rotation = SkelMeshComp->GetSocketRotation(LineSocket);
			FVector ShotDirection = Rotation.Vector();
			FVector EndLocation = StartLocation + (ShotDirection * 10000);
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(MyOwner);
			QueryParams.AddIgnoredActor(this);

			if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECollisionChannel::ECC_Pawn, QueryParams))
			{
				DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 1.0f, 0, 1.0f);
			}
		}
	}
	else
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			FHitResult Hit;
			FVector StartLocation = SkelMeshComp->GetSocketLocation(MuzzleSocket);
			FRotator Rotation = SkelMeshComp->GetSocketRotation(MuzzleSocket);
			FVector ShotDirection = Rotation.Vector();
			FVector EndLocation = StartLocation + (ShotDirection * 10000);
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(MyOwner);
			QueryParams.AddIgnoredActor(this);

			if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECollisionChannel::ECC_Pawn, QueryParams))
			{
				DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::White, false, 1.0f, 0, 1.0f);
			}
		}
	}
}

