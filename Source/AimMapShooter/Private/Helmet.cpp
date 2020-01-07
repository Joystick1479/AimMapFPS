// Fill out your copyright notice in the Description page of Project Settings.
#include "Helmet.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "SoldierCharacter.h"

// Sets default values
AHelmet::AHelmet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(MeshComp);

	SetReplicates(true);

}

// Called when the game starts or when spawned
void AHelmet::BeginPlay()
{
	Super::BeginPlay();
	
}

void AHelmet::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{
		SoldierCharacter->bHelmetPickUp = true;
		SphereComp->ToggleActive();
	}
}

void AHelmet::NotifyActorEndOverlap(AActor * OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{
		SoldierCharacter->bHelmetPickUp = false;

	}
}

// Called every frame
void AHelmet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

