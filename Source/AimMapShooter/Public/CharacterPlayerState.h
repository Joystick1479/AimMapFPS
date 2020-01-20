// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CharacterPlayerState.generated.h"

class UUserWidget;
class APayloadCharacter;

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

	void PayloadStatus();

	virtual void Tick(float DeltaTime) override;


	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<APayloadCharacter> PayloadCharacterClass;


	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<UUserWidget> wAmmoCount;

	UPROPERTY()
	UUserWidget* wAmmoCountvar;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<UUserWidget> wHealthIndicator;
	
	UPROPERTY()
	UUserWidget* wHealthIndicatorvar;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<UUserWidget> wPayloadObjective;

	UPROPERTY()
	UUserWidget* wPayloadObjectivesvar;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<UUserWidget> wPayloadPushing;

	UPROPERTY()
	UUserWidget* wPayloadPushingvar;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<UUserWidget> wPayloadContested;

	UPROPERTY()
	UUserWidget* wPayloadContestedvar;
};
