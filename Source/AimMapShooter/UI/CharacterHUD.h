// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CharacterHUD.generated.h"

class UUserWidget;
/**
 * 
 */
UCLASS()
class AIMMAPSHOOTER_API ACharacterHUD : public AHUD
{
	GENERATED_BODY()

private:

	virtual void BeginPlay() override;

	void PickUpHud();
	void AmmoAndHealthHud();
	///
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> wPickUp;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> PickUpTestWidgetClass;

	UPROPERTY()
	UUserWidget* wPickUpvar;
	///
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<UUserWidget> wAmmoCount;

	UPROPERTY()
	UUserWidget* wAmmoCountvar;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<UUserWidget> wHealthIndicator;

	UPROPERTY()
	UUserWidget* wHealthIndicatorvar;
	///
	FTimerHandle HudTimer;

	bool bDoOnce = true;

	
};
