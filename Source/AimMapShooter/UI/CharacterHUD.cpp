// Bartosz Jastrzebski


#include "CharacterHUD.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h" 

#include "Character/SoldierCharacter.h"

#include "TimerManager.h"

void ACharacterHUD::BeginPlay()
{
	Super::BeginPlay();

	PickUpHud();

	AmmoAndHealthHud();

}

void ACharacterHUD::PickUpHud()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	ASoldierCharacter* SoldierChar = Cast<ASoldierCharacter>(GetOwningPawn());
	if (PC && SoldierChar)
	{
		if (wPickUp)
		{
			wPickUpvar = CreateWidget<UUserWidget>(PC, wPickUp);
			if (wPickUpvar)
			{
				if (SoldierChar->GetbWantToPickUp() && bDoOnce == true)
				{
					wPickUpvar->AddToViewport();
					bDoOnce = false;
				}
				else
				{
					bDoOnce = true;
					TArray<UUserWidget*> PickupWidgets;
					UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, PickupWidgets, PickUpTestWidgetClass, true);

					if (PickupWidgets.Num() > 0)
					{
						UUserWidget* NewPickupWidget = PickupWidgets[0];
						NewPickupWidget->RemoveFromParent();
					}
				}
			}
		}
	}

	GetWorldTimerManager().SetTimer(HudTimer, this, &ACharacterHUD::PickUpHud, 0.5f, false);
}

void ACharacterHUD::AmmoAndHealthHud()
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