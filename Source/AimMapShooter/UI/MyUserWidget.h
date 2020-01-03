// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuInterface.h"
#include "MyUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class AIMMAPSHOOTER_API UMyUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	void SetMenuInterface(IMainMenuInterface* MenuInterface);

	void Setup();
	void TearDown();

protected:
	virtual bool Initialize();

private:

	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* JoinButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* CancelJoinMenuButton;

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* MenuSwitcher;

	UPROPERTY(meta = (BindWidget))
	class UWidget* JoinMenu;

	UPROPERTY(meta = (BindWidget))
	class UWidget* MainMenu;
	
	UFUNCTION()
	void HostServer();

	UFUNCTION()
	void OpenJoinMenu();

	UFUNCTION()
	void OpenMainMenu();

	IMainMenuInterface* MenuInterface;
};
