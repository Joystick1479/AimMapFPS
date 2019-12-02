// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Grip.h"

// Sets default values
AGrip::AGrip()
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
void AGrip::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGrip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

