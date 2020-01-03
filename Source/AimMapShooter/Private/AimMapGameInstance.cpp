// Fill out your copyright notice in the Description page of Project Settings.


#include "AimMapGameInstance.h"

#include "Engine/Engine.h"
#include "UObject//ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "UI/MyUserWidget.h"
#include "UI/MenuWidget.h"
#include "OnlineSubsystem.h"

UAimMapGameInstance::UAimMapGameInstance(const FObjectInitializer & ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UUserWidget> MenuBPClass(TEXT("/Game/UI/MainMenu/WBP_MainMenu"));
	if (!ensure(MenuBPClass.Class != nullptr)) return;

	MenuClass = MenuBPClass.Class;

	ConstructorHelpers::FClassFinder<UUserWidget> InGameMenuBPClass(TEXT("/Game/UI/MainMenu/WBP_InGameMenu"));
	if (!ensure(InGameMenuBPClass.Class != nullptr)) return;

	InGameMenuClass = InGameMenuBPClass.Class;
}

void UAimMapGameInstance::Init()
{
	UE_LOG(LogTemp, Warning, TEXT("Found class %s"), *MenuClass->GetName());

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found subsystem %s"), *Subsystem->GetSubsystemName().ToString());
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Found session interface"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Found no subsystem "));
	}

}
void UAimMapGameInstance::LoadMenuWidget()
{
	if (!ensure(MenuClass != nullptr)) return;

	Menu = CreateWidget<UMyUserWidget>(this, MenuClass);
	
	Menu->Setup();
	Menu->SetMenuInterface(this);

}
void UAimMapGameInstance::InGameLoadMenu()
{
	if (!ensure(InGameMenuClass != nullptr)) return;

	UMenuWidget* Menu = CreateWidget<UMenuWidget>(this, InGameMenuClass);

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

void UAimMapGameInstance::LoadMainMenu()
{
	APlayerController* PC = GetFirstLocalPlayerController();
	if (!ensure(PC != nullptr)) return;

	PC->ClientTravel("/Game/UI/MainMenu/MainMenu", ETravelType::TRAVEL_Absolute);
}
