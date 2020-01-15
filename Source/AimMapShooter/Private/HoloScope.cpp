// Fill out your copyright notice in the Description page of Project Settings.
#include "HoloScope.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "SoldierCharacter.h"

// Sets default values
AHoloScope::AHoloScope()
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
void AHoloScope::BeginPlay()
{
	Super::BeginPlay();

}

void AHoloScope::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{
		SoldierCharacter->bHoloPickUp = true;
		SphereComp->ToggleActive();

	}
}

void AHoloScope::NotifyActorEndOverlap(AActor * OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{
		SoldierCharacter->bHoloPickUp = false;

		if (SoldierCharacter->isHoloAttached == true)
		{
			this->Destroy();
		}
	}
}

// Called every frame
void AHoloScope::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

