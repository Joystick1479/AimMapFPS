// Fill out your copyright notice in the Description page of Project Settings.


#include "AimMapGameInstance.h"

#include "Engine/Engine.h"
#include "UObject//ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "UI/MyUserWidget.h"

UAimMapGameInstance::UAimMapGameInstance(const FObjectInitializer & ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UUserWidget> MenuBPClass(TEXT("/Game/UI/MainMenu/WBP_MainMenu"));
	if (!ensure(MenuBPClass.Class != nullptr)) return;

	MenuClass = MenuBPClass.Class;
}

void UAimMapGameInstance::Init()
{
	UE_LOG(LogTemp, Warning, TEXT("Found class %s"), *MenuClass->GetName());

}
void UAimMapGameInstance::LoadMenu()
{
	if (!ensure(MenuClass != nullptr)) return;

	Menu = CreateWidget<UMyUserWidget>(this, MenuClass);
	
	Menu->Setup();
	Menu->SetMenuInterface(this);

}
void UAimMapGameInstance::Host()
{
	if (Menu != nullptr)
	{
		Menu->TearDown();
	}

	UEngine* Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;

	Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Hosting"));

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;
	World->ServerTravel("/Game/FirstPersonBP/Maps/Map?listen");
	
}
void UAimMapGameInstance::Join(const FString& Address)
{
	if (Menu != nullptr)
	{
		Menu->TearDown();
	}

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