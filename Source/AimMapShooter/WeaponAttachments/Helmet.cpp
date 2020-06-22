// Fill out your copyright notice in the Description page of Project Settings.
#include "Helmet.h"

#include "Net/UnrealNetwork.h"

// Sets default values
AHelmet::AHelmet()
{
	SetReplicates(true);
}

// Called when the game starts or when spawned
void AHelmet::BeginPlay()
{
	Super::BeginPlay();

	NumberOfHits = 2;

}



int32 AHelmet::GetNumberOfHits()
{
	return this->NumberOfHits;
}


void AHelmet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//This function tells us how we want to replicate things//
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(AHelmet, NumberOfLives,COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHelmet, NumberOfHits, COND_OwnerOnly);
}
