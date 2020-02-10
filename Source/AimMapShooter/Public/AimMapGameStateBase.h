// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AimMapGameStateBase.generated.h"

/**
 * 
 */

class UTimerWidget;
class UUserWidget;

UCLASS()
class AIMMAPSHOOTER_API AAimMapGameStateBase : public AGameStateBase
{
	GENERATED_UCLASS_BODY()


public:

	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	bool IsTimeOver;

private:
	UPROPERTY(EditAnywhere, Category = "Widgets")
	TSubclassOf<UTimerWidget> TimerWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "RoundTime")
	float RoundTime;
	bool IsTimerActive;

	class UTextBlock* TextBlock;

	UTimerWidget* WidgetRef;
	
	void UpdateTimerText(float Seconds);
	void TimeIsOver();



	
};
