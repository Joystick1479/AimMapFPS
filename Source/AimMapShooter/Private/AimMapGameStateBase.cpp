// Fill out your copyright notice in the Description page of Project Settings.


#include "AimMapGameStateBase.h"

#include "AimMapGameModeBase.h"

#include "PayloadCharacter.h"

#include "TimerManager.h"

#include "UObject/ConstructorHelpers.h" 

#include "Blueprint/UserWidget.h" 

#include "UI/TimerWidget.h"

#include "Components/TextBlock.h" 

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetTextLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "EngineUtils.h" 

AAimMapGameStateBase::AAimMapGameStateBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//static ConstructorHelpers::FClassFinder<UUserWidget> BlueprintClass(TEXT("/Game/UI/Timer/WBP_Timer"));
	//if (BlueprintClass.Class)
	//{
	//	TimerWidgetClass = BlueprintClass.Class;
	//}
	IsTimeOver = false;
	

}

void AAimMapGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	//RoundTime = 120.0f;
	//UpdateTimerText(RoundTime);
	IsTimerActive = true;

	UWorld* World = this->GetWorld();
	if (!ensure(World != nullptr)) return;
	//Timer UI turned off for now
	//WidgetRef = CreateWidget<UTimerWidget>(World, TimerWidgetClass);
	//WidgetRef->AddToViewport();

	PayloadStatusHud();
}

void AAimMapGameStateBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	///TIMER

	/*if (IsTimerActive == true)
	{
		if (RoundTime > 0)
		{
			RoundTime = RoundTime - DeltaTime;
			UpdateTimerText(RoundTime);
		}
	}*/

	///GameMode hud logic

	/*TArray<AActor*> Payload;
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
	}*/
}

void AAimMapGameStateBase::UpdateTimerText(float Seconds)
{
	FTimespan Timespan;
	Timespan = UKismetMathLibrary::FromSeconds(Seconds);
	int32 Minute = Timespan.GetMinutes();
	int32 Second = Timespan.GetSeconds();

	FText SecondText = UKismetTextLibrary::Conv_IntToText(Second, false, false, 2, 2);
	FText MinuteText = UKismetTextLibrary::Conv_IntToText(Minute);
	FText BreakText = FText::FromString(" : ");
	FText FinalText = FText::Join({}, MinuteText, BreakText, SecondText);

	if (WidgetRef)
	{
		UTextBlock* TextBlock = WidgetRef->Timer;
		if (TextBlock)
		{
			TextBlock->SetText(FinalText);
		}
		if (Minute == 0 && Second == 0)
		{
			if (TextBlock)
			{
				FText GameOverText = FText::FromString("Game over");
				TextBlock->SetText(GameOverText);
				IsTimeOver = true;
				GetWorldTimerManager().SetTimer(TimerHandle, this, &AAimMapGameStateBase::GameRestart, 3.0f);
			}
		}
	}
}

void AAimMapGameStateBase::GameRestart()
{
	if (WidgetRef)
	{
		WidgetRef->RemoveFromParent();
	}


	AAimMapGameModeBase* GameMode = Cast<AAimMapGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->RestartGame();
	}

	GetWorldTimerManager().ClearTimer(TimerHandle);
}

void AAimMapGameStateBase::PayloadStatusHud()
{
	UWorld* World = this->GetWorld();
	if (!ensure(World != nullptr)) return;

	//APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
//	if (PC)
//	{
		if (wPayloadObjective)
		{
			wPayloadObjectivesvar = CreateWidget<UUserWidget>(World, wPayloadObjective);
			if (wPayloadObjectivesvar)
			{
				wPayloadObjectivesvar->AddToViewport();
			}
		}
		if (wPayloadContested)
		{
			wPayloadContestedvar = CreateWidget<UUserWidget>(World, wPayloadContested);
			if (wPayloadContestedvar)
			{
				wPayloadContestedvar->AddToViewport();
				wPayloadContestedvar->SetVisibility(ESlateVisibility::Hidden);
			}
		}
		if (wPayloadPushing)
		{
			wPayloadPushingvar = CreateWidget<UUserWidget>(World, wPayloadPushing);
			if (wPayloadPushingvar)
			{
				wPayloadPushingvar->AddToViewport();
				wPayloadPushingvar->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	//}
}
