// Fill out your copyright notice in the Description page of Project Settings.


#include "AimMapGameModeBase.h"

#include "TimerManager.h"

#include "SoldierCharacter.h"
#include "RedEndgame.h"
#include "BlueEndGame.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Engine/LatentActionManager.h" 

#include "UObject/UObjectGlobals.h"

#include "EngineUtils.h"

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
		GetWorldTimerManager().SetTimer(GameStartTimer, this, &AAimMapGameModeBase::StartGame, 3);
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
	//ResetLevel();

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	bUseSeamlessTravel = true;
	World->ServerTravel("/Game/AbandonedFactoryBuildings/Maps/Warehouse_01_day/Main_Warehouse_01?listen");

}

void AAimMapGameModeBase::CheckIfGameOver()
{
	///CHECKING IF GAME OVER OR COMPLETED OBJECTIVE , IF YES RESTART GAME AFTER DELAY ///

	TArray<AActor*> RedEndGame;
	UGameplayStatics::GetAllActorsOfClass(this, RedEndgameClass, RedEndGame);
	
	for (int i = 0; i < RedEndGame.Num(); i ++)
	{
		ARedEndgame*New = Cast<ARedEndgame>(RedEndGame[i]);
		if (New)
		{
			if (New->RedWins == true)
			{
				FTimerHandle RestartHandle;
				GetWorldTimerManager().SetTimer(RestartHandle, this, &AAimMapGameModeBase::RestartGame, 5.0f);
				UE_LOG(LogTemp, Warning, TEXT("RED IS WINNER"));
				
			}
		}
	}
	TArray<AActor*> BlueEndGame;
	UGameplayStatics::GetAllActorsOfClass(this, BlueEndgameClass, BlueEndGame);

	for (int i = 0; i < BlueEndGame.Num(); i++)
	{
		ABlueEndgame*New2 = Cast<ABlueEndgame>(BlueEndGame[i]);
		if (New2)
		{
			if (New2->BlueWins == true)
			{
				FTimerHandle RestartHandle2;
				GetWorldTimerManager().SetTimer(RestartHandle2, this, &AAimMapGameModeBase::RestartGame, 5.0f);
				UE_LOG(LogTemp, Warning, TEXT("BLUE IS WINNER"));
			}
		}
	}
}
void AAimMapGameModeBase::RespawningPlayer()
{
	for (TActorIterator<ASoldierCharacter> Itr(GetWorld()); Itr; ++Itr)
	{
		ASoldierCharacter* Soldier = *Itr;
		if (Soldier->bDied == true)
		{
			APlayerController* PC = Cast<APlayerController>(Soldier->GetController());
			{
				RestartPlayer(PC);
			}
		}
	}
}




void AAimMapGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckIfGameOver();
	//RespawningPlayer();
	
}

