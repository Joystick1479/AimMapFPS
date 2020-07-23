// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AimMapGameModeBase.generated.h"

class ASoldierCharacter;
class UUserWidget;
class USoundCue;
class APlayerStartActor;

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

	bool ShouldTimerOn;

	UFUNCTION(BlueprintCallable)
	void RespawningPlayer();

	virtual void Tick(float DeltaTime) override;

	virtual void BeginPlay() override;

private:

	uint32 NumberOfPlayers = 0;
	FString Name;

	void StartGame();

	FTimerHandle GameStartTimer;
	FTimerHandle GameOverTimer;
	FTimerHandle RespawnDeadPlayer;


	bool BlueIsWinner;
	bool RedIsWinner;

	ASoldierCharacter* Soldiero;

	UPROPERTY(EditDefaultsOnly, Category = "EndTarget")
	TSubclassOf<ASoldierCharacter> SoldierCharClass;

	UPROPERTY(EditDefaultsOnly, Category = "GameOver")
	TSubclassOf<UUserWidget> GameOverWidget;


	UPROPERTY(EditDefaultsOnly, Category = "ScoreActors")
	TSubclassOf<APlayerStartActor> PlayerStartClass;

	TArray<AActor*> PlayerStarts;

	UPROPERTY()
	UUserWidget* wGameOverWidget;

	void SpawnAndPossPawn(AController* PC);

	virtual void RestartPlayer(AController* PC) override;




};
