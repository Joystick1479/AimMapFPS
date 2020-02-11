// Fill out your copyright notice in the Description page of Project Settings.


#include "AimMapGameStateBase.h"

#include "AimMapGameModeBase.h"

#include "TimerManager.h"

#include "UObject/ConstructorHelpers.h" 

#include "Blueprint/UserWidget.h" 

#include "UI/TimerWidget.h"

#include "Components/TextBlock.h" 

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetTextLibrary.h"

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
	WidgetRef = CreateWidget<UTimerWidget>(World, TimerWidgetClass);
	WidgetRef->AddToViewport();
//	WidgetRef->SetVisibility(ESlateVisibility::Hidden);


}

void AAimMapGameStateBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsTimerActive == true)
	{
		if (RoundTime > 0)
		{
		//	WidgetRef->SetVisibility(ESlateVisibility::Visible);
			RoundTime = RoundTime - DeltaTime;
			UpdateTimerText(RoundTime);
		}
	}
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
