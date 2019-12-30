// Fill out your copyright notice in the Description page of Project Settings.


#include "AimMapGameInstance.h"
#include "Engine/Engine.h"

UAimMapGameInstance::UAimMapGameInstance(const FObjectInitializer & ObjectInitializer)
{
}

void UAimMapGameInstance::Init()
{

}
void UAimMapGameInstance::Host()
{
	UEngine* Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;

	Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Hosting"));

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;
	World->ServerTravel("/Game/FirstPersonBP/Maps/Map?listen");
	
}
void UAimMapGameInstance::Join(const FString& Address)
{
	UEngine* Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;

	Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));

	APlayerController* PC = GetFirstLocalPlayerController();
	if (!ensure(PC != nullptr)) return;

	PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);

}

void UAimMapGameInstance::Reset()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;
	World->ServerTravel("/Game/FirstPersonBP/Maps/Map");
}