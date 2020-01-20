// Bartosz Jastrzebski


#include "CharacterPlayerState.h"

#include "Blueprint/UserWidget.h"

#include "PayloadCharacter.h"

#include "Kismet/GameplayStatics.h"

void ACharacterPlayerState::BeginPlay()
{
	Super::BeginPlay();
	StartingHud();
	PayloadStatus();
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

void ACharacterPlayerState::PayloadStatus()
{

	APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		if (wPayloadObjective)
		{
			wPayloadObjectivesvar = CreateWidget<UUserWidget>(PC, wPayloadObjective);
			if (wPayloadObjectivesvar)
			{
				wPayloadObjectivesvar->AddToViewport();
			}
		}
		if (wPayloadContested)
		{
			wPayloadContestedvar = CreateWidget<UUserWidget>(PC, wPayloadContested);
			if (wPayloadContestedvar)
			{
				wPayloadContestedvar->AddToViewport();
				wPayloadContestedvar->SetVisibility(ESlateVisibility::Hidden);
			}
		}
		if (wPayloadPushing)
		{
			wPayloadPushingvar = CreateWidget<UUserWidget>(PC, wPayloadPushing);
			if (wPayloadPushingvar)
			{
				wPayloadPushingvar->AddToViewport();
				wPayloadPushingvar->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
}

void ACharacterPlayerState::Tick(float DeltaTime)
{
	TArray<AActor*> Payload;
	UGameplayStatics::GetAllActorsOfClass(this, PayloadCharacterClass, Payload);
	for (int i = 0; i < Payload.Num(); i++)
	{
		APayloadCharacter* PayloadChar = Cast<APayloadCharacter>(Payload[i]);
		if (PayloadChar)
		{
			if (PayloadChar->OnePlayerPushing == true)
			{
				if (wPayloadObjectivesvar)
				{
					wPayloadObjectivesvar->SetVisibility(ESlateVisibility::Hidden);
				}
				if (wPayloadContestedvar)
				{
					wPayloadContestedvar->SetVisibility(ESlateVisibility::Hidden);
				}
				if (wPayloadPushingvar)
				{
					wPayloadPushingvar->SetVisibility(ESlateVisibility::Visible);
				}
			}
			if (PayloadChar->ContestedPushing == true)
			{
				if (wPayloadObjectivesvar)
				{
					wPayloadObjectivesvar->SetVisibility(ESlateVisibility::Hidden);
				}
				if (wPayloadContestedvar)
				{
					wPayloadContestedvar->SetVisibility(ESlateVisibility::Visible);
				}
				if (wPayloadPushingvar)
				{
					wPayloadPushingvar->SetVisibility(ESlateVisibility::Hidden);
				}
			}
			if (PayloadChar->ContestedPushing == false && PayloadChar->OnePlayerPushing == false)
			{
				if (wPayloadObjectivesvar)
				{
					wPayloadObjectivesvar->SetVisibility(ESlateVisibility::Visible);
				}
				if (wPayloadContestedvar)
				{
					wPayloadContestedvar->SetVisibility(ESlateVisibility::Hidden);
				}
				if (wPayloadPushingvar)
				{
					wPayloadPushingvar->SetVisibility(ESlateVisibility::Hidden);
				}
			}
		}
	}

}
