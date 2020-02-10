// Fill out your copyright notice in the Description page of Project Settings.


#include "AimMapGameStateBase.h"

#include "UObject/ConstructorHelpers.h" 

#include "Blueprint/UserWidget.h" 

#include "UI/TimerWidget.h"

#include "Components/TextBlock.h" 

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetTextLibrary.h"

AAimMapGameStateBase::AAimMapGameStateBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//static ConstructorHelpers::FClassFinder<UUserWidget> BlueprintClass(TEXT("/Game/UI/Timer/WBP_Timer"));
	//if (BlueprintClass.Class)
	//{
	//	TimerWidgetClass = BlueprintClass.Class;
	//}
	

}

void AAimMapGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	RoundTime = 120.0f;
	UpdateTimerText(RoundTime);
	IsTimerActive = true;

	UWorld* World = this->GetWorld();
	if (!ensure(World != nullptr)) return;
	WidgetRef = CreateWidget<UTimerWidget>(World, TimerWidgetClass);
	WidgetRef->AddToViewport();
	

}

void AAimMapGameStateBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsTimerActive == true)
	{
		RoundTime = RoundTime - DeltaTime;
		UpdateTimerText(RoundTime);
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
	FText FinalText = FText::Join({}, MinuteText, SecondText);

	if (WidgetRef)
	{
		UTextBlock* TextBlock = WidgetRef->Timer;
		if (TextBlock)
		{
			TextBlock->SetText(FinalText);
		}
	}
}
