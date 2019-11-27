// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Headset.h"

// Sets default values
AHeadset::AHeadset()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(MeshComp);
}

// Called when the game starts or when spawned
void AHeadset::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHeadset::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

