// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UI/MainMenuInterface.h"
#include "OnlineSubsystem.h"
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
	void LoadMenuWidget();

	UFUNCTION(BlueprintCallable)
	void InGameLoadMenu();

	UFUNCTION(exec)
	void Host() override;

	UFUNCTION(exec)
	void Join(const FString& Address) override;

	UFUNCTION(exec)
	void Reset();

	virtual void LoadMainMenu() override;

private:
	TSubclassOf<class UUserWidget> MenuClass;

	TSubclassOf<class UUserWidget> InGameMenuClass;

	class UMyUserWidget* Menu;

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	void OnCreateSessionComplete(FName SessionName, bool Success);
	void OnDestroySessionComplete(FName SessionName, bool Success);
	void OnFindSessionsComplete(bool Success);


	void CreateSession();

};
