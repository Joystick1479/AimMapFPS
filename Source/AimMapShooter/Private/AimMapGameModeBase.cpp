// Fill out your copyright notice in the Description page of Project Settings.


#include "AimMapGameModeBase.h"

#include "TimerManager.h"

#include "Character/SoldierCharacter.h"

#include "PlayerStartActor.h"

#include "Sound/SoundCue.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

#include "Engine/LatentActionManager.h" 

#include "UObject/UObjectGlobals.h"

#include "EngineUtils.h"

void AAimMapGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), PlayerStartClass, PlayerStarts);

}

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
	World->ServerTravel("/Game/Maps/Blockout_P?listen");

	
}

void AAimMapGameModeBase::RestartGame()
{

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	bUseSeamlessTravel = false;
	World->ServerTravel("/Game/Maps/Blockout_P?listen");
}

void AAimMapGameModeBase::RespawningPlayer()
{
	TArray<AActor*> Target;
	UGameplayStatics::GetAllActorsOfClass(this, SoldierCharClass, Target);
	for (int i = 0; i < Target.Num(); i++)
	{
		ASoldierCharacter* SoldChar = Cast<ASoldierCharacter>(Target[i]);
		if (SoldChar)
		{
			if (SoldChar->GetbDied() == true && SoldChar->GetbWantToRespawn() == true)
			{
				AController* PC = Cast<AController>(SoldChar->GetController());
				if(PC)
				{
					SpawnAndPossPawn(PC);
				}
			}
		}
	}
}

void AAimMapGameModeBase::SpawnAndPossPawn(AController* PC)
{
		///Find player start and respawn player
		///TODO random spawns
	if (PC)
	{
		int randomNumber = UKismetMathLibrary::RandomInteger(PlayerStarts.Num());

		AActor* StartSpot = PlayerStarts[randomNumber];

		APlayerStartActor * PlayerStartActor = Cast<APlayerStartActor>(StartSpot);
		if (PlayerStartActor->GetbIsOverlapping())
		{
			SpawnAndPossPawn(PC);
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		UE_LOG(LogTemp, Warning, TEXT("SPAWNING"));
		if (StartSpot)
		{
			Soldiero = GetWorld()->SpawnActor<ASoldierCharacter>(SoldierCharClass, StartSpot->GetActorLocation(), StartSpot->GetActorRotation(), SpawnParams);
			PC->Possess(Soldiero);
		}
	}

}

void AAimMapGameModeBase::RestartPlayer(AController* PC)
{
	Super::RestartPlayer(PC);

}

void AAimMapGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}




