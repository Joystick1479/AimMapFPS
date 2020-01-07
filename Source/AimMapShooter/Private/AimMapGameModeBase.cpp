// Fill out your copyright notice in the Description page of Project Settings.


#include "AimMapGameModeBase.h"

void AAimMapGameModeBase::PostLogin(APlayerController * NewPlayer)
{
	++NumberOfPlayers;
}

void AAimMapGameModeBase::Logout(AController * Exiting)
{
	--NumberOfPlayers;
}
