// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuInterface.h"
#include "MenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class AIMMAPSHOOTER_API UMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	void Setup();
	void TearDown();

	void SetMenuInterface(IMainMenuInterface* MenuInterface);

protected:
	IMainMenuInterface* MenuInterface;

	
};
