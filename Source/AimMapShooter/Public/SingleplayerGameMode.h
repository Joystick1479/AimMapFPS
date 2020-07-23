// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "AimMapGameModeBase.h"
#include "SingleplayerGameMode.generated.h"


/**
 * 
 */
UCLASS()
class AIMMAPSHOOTER_API ASingleplayerGameMode : public AAimMapGameModeBase
{
	GENERATED_BODY()

protected:

	FTimerHandle TimerHandle_BotSpawner;
	FTimerHandle TimerHandle_NextWaveStart;
	int32 NrOfBotsToSpawn = 0;
	int32 WaveCount = 0;

	UPROPERTY(EditDefaultsOnly, Category =  "GameMode")
	float TimeBetweenWaves = 0;

	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();

	void SpawnBotTimerElapsed();

	void StartWave();

	void EndWave();

	void PrepareForNextWave();

	void CheckWaveState();

public:

	ASingleplayerGameMode();

	virtual void Tick(float DeltaSeconds) override;

	virtual void StartPlay() override;

	
};
