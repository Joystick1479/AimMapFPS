// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CharacterPlayerState.generated.h"

class UUserWidget;

/**
 * 
 */
UCLASS()
class AIMMAPSHOOTER_API ACharacterPlayerState : public APlayerState
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	void StartingHud();

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<UUserWidget> wAmmoCount;

	UPROPERTY()
	UUserWidget* wAmmoCountvar;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<UUserWidget> wHealthIndicator;
	

	UPROPERTY()
	UUserWidget* wHealthIndicatorvar;
};
