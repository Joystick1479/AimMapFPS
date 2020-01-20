// Bartosz Jastrzebski


#include "CharacterPlayerState.h"

#include "Blueprint/UserWidget.h"

void ACharacterPlayerState::BeginPlay()
{
	Super::BeginPlay();
	StartingHud();
}

void ACharacterPlayerState::StartingHud()
{
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