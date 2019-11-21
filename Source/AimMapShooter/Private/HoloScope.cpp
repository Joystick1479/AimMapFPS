// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/StaticMeshComponent.h"
#include "HoloScope.h"

// Sets default values
AHoloScope::AHoloScope()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

}

// Called when the game starts or when spawned
void AHoloScope::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHoloScope::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

