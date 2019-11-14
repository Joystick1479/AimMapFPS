// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/SkeletalMeshComponent.h"
#include "AutomaticRifle.h"

// Sets default values
AAutomaticRifle::AAutomaticRifle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SkelMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkelMesh"));
	RootComponent = SkelMeshComp;

}

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

