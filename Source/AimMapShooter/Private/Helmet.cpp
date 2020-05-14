// Fill out your copyright notice in the Description page of Project Settings.
#include "Helmet.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

#include "TimerManager.h"

#include "Net/UnrealNetwork.h"

#include "SoldierCharacter.h"

// Sets default values
AHelmet::AHelmet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

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

	NumberOfLives = 2;
	
	DestroyOnUse();
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

void AHelmet::DestroyOnUse()
{
	if (IsPickedUp == true)
	{
		this->Destroy();
	}
	if (NumberOfLives <= 0)
	{
		ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(this->GetOwner());
		if (SoldierCharacter)
		{
			SoldierCharacter->HelmetEquipState= EHelmetAttachment::None;
		}
		this->Destroy();
	}

	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AHelmet::DestroyOnUse, 0.5f, false);
}



// Called every frame
void AHelmet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//UE_LOG(LogTemp, Warning, TEXT("Number of lives: %i"), NumberOfLives);

}


void AHelmet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//This function tells us how we want to replicate things//
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(AHelmet, NumberOfLives,COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHelmet, NumberOfLives, COND_OwnerOnly);


}
