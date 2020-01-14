// Fill out your copyright notice in the Description page of Project Settings.


#include "AimMapGameModeBase.h"

#include "TimerManager.h"

#include "SoldierCharacter.h"

void AAimMapGameModeBase::PostLogin(APlayerController * NewPlayer)
{
	Super::PostLogin(NewPlayer);

	++NumberOfPlayers;
	
	NewPlayer->SetName(Name);
	if (NewPlayer->GetName() == "PlayerController_0")
	{
		NewPlayer->SetName("Blue");
	}
	else
	{
		NewPlayer->SetName("Red");
	}

	UE_LOG(LogTemp, Warning, TEXT("Name is: %s"), *NewPlayer->GetName());
	UE_LOG(LogTemp, Warning, TEXT("TeamName is: %s"), *TeamName);



	if (NumberOfPlayers > 1)
	{
		//GetWorldTimerManager().SetTimer(GameStartTimer, this, &AAimMapGameModeBase::StartGame, 7);
	}
}

void AAimMapGameModeBase::Logout(AController * Exiting)
{
	Super::Logout(Exiting);

	--NumberOfPlayers;
}

void AAimMapGameModeBase::StartGame()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	bUseSeamlessTravel = true;
	//World->ServerTravel("/Game/FirstPersonBP/Maps/Map?listen");
	World->ServerTravel("/Game/AbandonedFactoryBuildings/Maps/Warehouse_01_day/Main_Warehouse_01?listen");
}

void AAimMapGameModeBase::RestartGame()
{

	ResetLevel();

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	bUseSeamlessTravel = true;
	World->ServerTravel("/Game/FirstPersonBP/Maps/Map?listen");
}

void AAimMapGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

