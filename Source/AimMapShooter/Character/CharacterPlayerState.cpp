// Bartosz Jastrzebski


#include "CharacterPlayerState.h"

#include "Blueprint/UserWidget.h"

#include "AI/PayloadCharacter.h"

#include "Kismet/GameplayStatics.h"

void ACharacterPlayerState::BeginPlay()
{
	Super::BeginPlay();
	StartingHud();
}

void ACharacterPlayerState::StartingHud()
{
	UE_LOG(LogTemp, Warning, TEXT("Player State begin play"));
		APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
		if (PC)
		{
			if (wAmmoCount)
			{
				wAmmoCountvar = CreateWidget<UUserWidget>(PC, wAmmoCount);
				if (wAmmoCountvar)
				{
					wAmmoCountvar->AddToViewport();
				}
			}
			if (wHealthIndicator)
			{
				wHealthIndicatorvar = CreateWidget<UUserWidget>(PC, wHealthIndicator);
				if (wHealthIndicatorvar)
				{
					wHealthIndicatorvar->AddToViewport();
				}
			}
		}
}

void ACharacterPlayerState::Tick(float DeltaTime)
{
	

}
