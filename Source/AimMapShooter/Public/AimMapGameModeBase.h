// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AimMapGameModeBase.generated.h"

class ARedEndgame;
class ABlueEndgame;
class ASoldierCharacter;
class UUserWidget;
class USoundCue;

/**
 * 
 */
UCLASS()
class AIMMAPSHOOTER_API AAimMapGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	

	UFUNCTION(BlueprintCallable)
	void PostLogin(APlayerController* NewPlayer) override;

	void Logout(AController* Exiting) override;

	
	UPROPERTY(BlueprintReadWrite)
	FString TeamName;

	UFUNCTION(BlueprintCallable)
	void RestartGame();

	UFUNCTION(BlueprintCallable)
	void CheckIfGameOver();



private:

	uint32 NumberOfPlayers = 0;
	FString Name;

	void StartGame();

	FTimerHandle GameStartTimer;
	FTimerHandle GameOverTimer;
	FTimerHandle RespawnDeadPlayer;

	UFUNCTION(BlueprintCallable)
	void RespawningPlayer();


	bool BlueIsWinner;
	bool RedIsWinner;

	ASoldierCharacter* Soldiero;

	UPROPERTY(EditDefaultsOnly, Category = "EndTarget")
	TSubclassOf<ABlueEndgame> BlueEndgameClass;

	UPROPERTY(EditDefaultsOnly, Category = "EndTarget")
	TSubclassOf<ASoldierCharacter> SoldierCharClass;

	UPROPERTY(EditDefaultsOnly, Category = "EndTarget")
	TSubclassOf<ARedEndgame> RedEndgameClass;


	UPROPERTY(EditDefaultsOnly, Category = "GameOver")
	TSubclassOf<UUserWidget> GameOverWidget;

	UPROPERTY()
	UUserWidget* wGameOverWidget;


	virtual void Tick(float DeltaTime) override;


};
