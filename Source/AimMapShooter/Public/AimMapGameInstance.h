// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UI/MainMenuInterface.h"
#include "AimMapGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class AIMMAPSHOOTER_API UAimMapGameInstance : public UGameInstance, public IMainMenuInterface
{
	GENERATED_BODY()

	UAimMapGameInstance(const FObjectInitializer & ObjectInitializer);

	virtual void Init();

	UFUNCTION(BlueprintCallable)
	void LoadMenu();

	UFUNCTION(exec)
	void Host();

	UFUNCTION(exec)
	void Join(const FString& Address);

	UFUNCTION(exec)
	void Reset();

private:
	TSubclassOf<class UUserWidget> MenuClass;

	class UMyUserWidget* Menu;
	
};
