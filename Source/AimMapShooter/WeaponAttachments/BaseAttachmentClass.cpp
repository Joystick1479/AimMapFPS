// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseAttachmentClass.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

#include "TimerManager.h"

#include "Character/SoldierCharacter.h"

// Sets default values
ABaseAttachmentClass::ABaseAttachmentClass()
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
void ABaseAttachmentClass::BeginPlay()
{
	Super::BeginPlay();
	
}

bool ABaseAttachmentClass::GetIfPickeditem()
{
	return bPickedUp;
}
void ABaseAttachmentClass::SetIfPickedItem(bool PickedUp)
{
	bPickedUp = PickedUp;
}

UStaticMeshComponent* ABaseAttachmentClass::GetMeshComponent()
{
	return MeshComp;
}
USphereComponent* ABaseAttachmentClass::GetSphereComponent()
{
	return SphereComp;
}
