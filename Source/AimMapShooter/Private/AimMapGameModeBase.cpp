// Fill out your copyright notice in the Description page of Project Settings.


#include "AimMapGameModeBase.h"

void AAimMapGameModeBase::PostLogin(APlayerController * NewPlayer)
{
	Super::PostLogin(NewPlayer);

	++NumberOfPlayers;

	if (NumberOfPlayers > 1)
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr)) return;

		bUseSeamlessTravel = true;
		World->ServerTravel("/Game/FirstPersonBP/Maps/Map?listen");
	}
}

void AAimMapGameModeBase::Logout(AController * Exiting)
{
	Super::Logout(Exiting);

	--NumberOfPlayers;
}
