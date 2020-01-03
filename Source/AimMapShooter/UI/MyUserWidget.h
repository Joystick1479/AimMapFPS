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
	class UButton* Host;

	UPROPERTY(meta = (BindWidget))
	class UButton* Join;
	
	UFUNCTION()
	void HostServer();

	IMainMenuInterface* MenuInterface;
};
