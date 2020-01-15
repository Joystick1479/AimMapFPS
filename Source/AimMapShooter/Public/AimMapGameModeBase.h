// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AimMapGameModeBase.generated.h"

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

private:

	uint32 NumberOfPlayers = 0;
	FString Name;

	void StartGame();

	FTimerHandle GameStartTimer;

	UFUNCTION(BlueprintCallable)
	void RestartGame();

	void CheckIfGameOver();

	virtual void Tick(float DeltaTime) override;


};
