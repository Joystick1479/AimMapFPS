// Bartosz Jastrzebski


#include "SingleplayerGameMode.h"
#include "TimerManager.h"
#include "Character/HealthComponent.h"




ASingleplayerGameMode::ASingleplayerGameMode()
{
	TimeBetweenWaves = 2.0f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;
}

void ASingleplayerGameMode::StartWave()
{
	WaveCount++;

	NrOfBotsToSpawn = MultiplyNumberOfBots * WaveCount;

	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASingleplayerGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);
}

void ASingleplayerGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
}

void ASingleplayerGameMode::PrepareForNextWave()
{

	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASingleplayerGameMode::StartWave, TimeBetweenWaves, false);
}

void ASingleplayerGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckWaveState();
}

void ASingleplayerGameMode::CheckWaveState()
{
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	if (NrOfBotsToSpawn > 0 || bIsPreparingForWave)
	{
		return;
	}

	bool bIsAnyBotsAlive = false;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			continue;
		}

		UHealthComponent* HealthComp = Cast<UHealthComponent>(TestPawn->GetComponentByClass(UHealthComponent::StaticClass()));
		if (HealthComp && HealthComp->Health>0.0f)
		{
			bIsAnyBotsAlive = true;
			break;
		}
	}
	
	if (!bIsAnyBotsAlive)
	{
		PrepareForNextWave();
	}
}

	void ASingleplayerGameMode::StartPlay()
	{
		Super::StartPlay();

		PrepareForNextWave();
	}

void ASingleplayerGameMode::SpawnBotTimerElapsed()
{
	SpawnNewBot();

	NrOfBotsToSpawn--;

	if (NrOfBotsToSpawn <= 0)
	{
		EndWave();
	}
}

